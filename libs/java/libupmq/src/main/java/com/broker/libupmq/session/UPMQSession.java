/*
 * Copyright 2014-present IVK JSC. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
package com.broker.libupmq.session;

import java.io.Serializable;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.jms.BytesMessage;
import javax.jms.Destination;
import javax.jms.IllegalStateException;
import javax.jms.InvalidDestinationException;
import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageListener;
import javax.jms.MessageProducer;
import javax.jms.ObjectMessage;
import javax.jms.Queue;
import javax.jms.QueueBrowser;
import javax.jms.QueueReceiver;
import javax.jms.QueueSender;
import javax.jms.QueueSession;
import javax.jms.Session;
import javax.jms.StreamMessage;
import javax.jms.TemporaryQueue;
import javax.jms.TemporaryTopic;
import javax.jms.TextMessage;
import javax.jms.Topic;
import javax.jms.TopicPublisher;
import javax.jms.TopicSession;
import javax.jms.TopicSubscriber;
import javax.jms.XAQueueSession;
import javax.jms.XATopicSession;

import org.apache.log4j.Logger;

import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.libupmq.client.UPMQMessageConsumer.UPMQConsumer;
import com.broker.libupmq.client.UPMQMessageProducer;
import com.broker.libupmq.client.UPMQQueueBrowser;
import com.broker.libupmq.client.UPMQQueueReceiver;
import com.broker.libupmq.client.UPMQQueueSender;
import com.broker.libupmq.client.UPMQTopicPublisher;
import com.broker.libupmq.client.UPMQTopicSubscriber;
import com.broker.libupmq.connection.UPMQConnection;
import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.destination.UPMQQueue;
import com.broker.libupmq.destination.UPMQTemporaryQueue;
import com.broker.libupmq.destination.UPMQTemporaryTopic;
import com.broker.libupmq.destination.UPMQTopic;
import com.broker.libupmq.message.UPMQBytesMessage;
import com.broker.libupmq.message.UPMQMapMessage;
import com.broker.libupmq.message.UPMQMessage;
import com.broker.libupmq.message.UPMQObjectMessage;
import com.broker.libupmq.message.UPMQStreamMessage;
import com.broker.libupmq.message.UPMQTextMessage;
import com.broker.libupmq.transport.UPMQCommand;
import com.broker.protocol.Protocol;

public class UPMQSession implements Session {
	private static final Logger log = Logger.getLogger(UPMQSession.class);
	private final Object lock = new Object();

	public String _objectId = UUID.randomUUID().toString();
	public UPMQConnection _connection;

	private final AtomicBoolean _isStarted = new AtomicBoolean(false);
	private final AtomicBoolean _isStoped = new AtomicBoolean(true);
	private final AtomicBoolean _isClosed = new AtomicBoolean(false);

	private int _acknowledgeMode;

	private MessageListener _messageListener = null;

	private volatile Set<UPMQMessageProducer> _producers = new HashSet<UPMQMessageProducer>();
	private volatile Set<UPMQMessageConsumer> _consumers = new HashSet<UPMQMessageConsumer>();

	public List<Message> _messageList = new LinkedList<Message>();

	public UPMQSession(UPMQConnection connectionImpl, boolean transacted, int acknowledgeMode) throws JMSException {
		_connection = connectionImpl;
		_acknowledgeMode = (transacted) ? SESSION_TRANSACTED : acknowledgeMode;

		if (connectionImpl == null)
			throw new JMSException("Invalid parameter (is null)");

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getSessionBuilder().setConnectionId(connectionImpl._objectId);
		messageb.getSessionBuilder().setSessionId(_objectId);
		messageb.getSessionBuilder().setReceiptId(_objectId);
		messageb.getSessionBuilder().setAcknowledgeType(Protocol.Acknowledge.forNumber(_acknowledgeMode));

		UPMQCommand response = (UPMQCommand) _connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		if (_connection.isStarted()) {
			start();
		}

		log.debug("+sess: " + _objectId + " (ack: " + Protocol.Acknowledge.forNumber(_acknowledgeMode) + ")");
	}

	@Override
	public Message createMessage() throws JMSException {
		checkClosed();
		return new UPMQMessage(null);
	}

	@Override
	public TextMessage createTextMessage() throws JMSException {
		checkClosed();
		TextMessage message = new UPMQTextMessage(null);
		return message;
	}

	@Override
	public MapMessage createMapMessage() throws JMSException {
		checkClosed();
		MapMessage message = new UPMQMapMessage(null);
		return message;
	}

	@Override
	public ObjectMessage createObjectMessage() throws JMSException {
		checkClosed();
		ObjectMessage message = new UPMQObjectMessage(null);
		return message;
	}

	@Override
	public BytesMessage createBytesMessage() throws JMSException {
		checkClosed();
		BytesMessage message = new UPMQBytesMessage(null);
		return message;
	}

	@Override
	public StreamMessage createStreamMessage() throws JMSException {
		checkClosed();
		StreamMessage message = new UPMQStreamMessage(null);
		return message;
	}

	@Override
	public ObjectMessage createObjectMessage(Serializable object) throws JMSException {
		checkClosed();
		ObjectMessage message = new UPMQObjectMessage(null);
		message.setObject(object);
		return message;
	}

	@Override
	public TextMessage createTextMessage(String text) throws JMSException {
		checkClosed();
		TextMessage message = new UPMQTextMessage(null);
		message.setText(text);
		return message;
	}

	@Override
	public boolean getTransacted() throws JMSException {
		checkClosed();
		return (_acknowledgeMode == SESSION_TRANSACTED);
	}

	@Override
	public int getAcknowledgeMode() throws JMSException {
		checkClosed();
		return _acknowledgeMode;
	}

	@Override
	public void commit() throws JMSException {
		checkClosed();
		checkTransactional();

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getCommitBuilder().setReceiptId(_objectId);
		messageb.getCommitBuilder().setSessionId(_objectId);

		UPMQCommand response = (UPMQCommand) _connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		log.debug(">commit: " + _objectId);
	}

	@Override
	public void rollback() throws JMSException {
		checkClosed();
		checkTransactional();

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getAbortBuilder().setReceiptId(_objectId);
		messageb.getAbortBuilder().setSessionId(_objectId);

		UPMQCommand response = (UPMQCommand) _connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		log.debug(">rollback: " + _objectId);
	}

	@Override
	public void recover() throws JMSException {
		checkClosed();
		checkNotTransactional();

		stop();

		for (Iterator<UPMQMessageConsumer> iterator = _consumers.iterator(); iterator.hasNext();) {
			final UPMQMessageConsumer messageConsumer = iterator.next();
			messageConsumer._messageQueue.stop();
			messageConsumer._messageQueue.clear();
			messageConsumer._messageQueue.start();
		}

		start();
	}

	public void start() throws JMSException {
		checkClosed();

		_isStarted.set(true);
		_isStoped.set(false);
		_isClosed.set(false);

		for (Iterator<UPMQMessageConsumer> iterator = _consumers.iterator(); iterator.hasNext();) {
			final UPMQMessageConsumer messageConsumer = iterator.next();
			messageConsumer.start();
		}
	}

	public void stop() throws JMSException {
		checkClosed();

		for (Iterator<UPMQMessageConsumer> iterator = _consumers.iterator(); iterator.hasNext();) {
			final UPMQMessageConsumer messageConsumer = iterator.next();
			messageConsumer.stop();
		}

		_isStarted.set(false);
		_isStoped.set(true);
	}

	@Override
	public void close() throws JMSException {
		if (isClosed())
			return;

		for (Iterator<UPMQMessageConsumer> iterator = _consumers.iterator(); iterator.hasNext();) {
			final UPMQMessageConsumer messageConsumer = iterator.next();
			iterator.remove();
			messageConsumer.close();
		}

		for (Iterator<UPMQMessageProducer> iterator = _producers.iterator(); iterator.hasNext();) {
			final UPMQMessageProducer messageProducer = iterator.next();
			iterator.remove();
			messageProducer.close();
		}

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getUnsessionBuilder().setReceiptId(_objectId);
		messageb.getUnsessionBuilder().setSessionId(_objectId);

		UPMQCommand response = (UPMQCommand) _connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		_isStarted.set(false);
		_isStoped.set(true);
		_isClosed.set(true);

		_connection.removeSession(this);

		log.debug("-sess: " + _objectId + " (ack: " + Protocol.Acknowledge.forNumber(_acknowledgeMode) + ")");
	}

	@Override
	public MessageListener getMessageListener() throws JMSException {
		checkClosed();
		return _messageListener;
	}

	@Override
	public void setMessageListener(MessageListener listener) throws JMSException {
		checkClosed();
		_messageListener = listener;
	}

	public TopicPublisher createPublisher(Topic topic) throws JMSException {
		checkClosed();

		UPMQTopicPublisher producer = new UPMQTopicPublisher(this, topic);
		addProducer(producer);
		return producer;
	}

	public QueueSender createSender(Queue queue) throws JMSException {
		checkClosed();

		UPMQQueueSender producer = new UPMQQueueSender(this, queue);
		addProducer(producer);
		return producer;
	}

	@Override
	public MessageProducer createProducer(Destination destination) throws JMSException {
		return _createProducer(destination);
	}

	@Override
	public MessageConsumer createConsumer(Destination destination) throws JMSException {
		return _createConsumer(destination, null, null, false);
	}

	public TopicSubscriber createSubscriber(Topic topic) throws JMSException {
		checkClosed();

		UPMQTopicSubscriber consumer = new UPMQTopicSubscriber(this, topic, null, null, false, UPMQConsumer.CONSUMER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	public QueueReceiver createReceiver(Queue queue) throws JMSException {
		checkClosed();

		UPMQQueueReceiver consumer = new UPMQQueueReceiver(this, queue, null, null, false, UPMQConsumer.CONSUMER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	@Override
	public QueueBrowser createBrowser(Queue queue) throws JMSException {
		checkClosed();
		checkQueueSession();

		UPMQQueueBrowser consumer = new UPMQQueueBrowser(this, queue, null, null, false, UPMQConsumer.BROWSER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	@Override
	public MessageConsumer createConsumer(Destination destination, String messageSelector) throws JMSException {
		return _createConsumer(destination, messageSelector, null, false);
	}

	public QueueReceiver createReceiver(Queue queue, String messageSelector) throws JMSException {
		checkClosed();

		UPMQQueueReceiver consumer = new UPMQQueueReceiver(this, queue, messageSelector, null, false, UPMQConsumer.CONSUMER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	@Override
	public QueueBrowser createBrowser(Queue queue, String messageSelector) throws JMSException {
		checkClosed();

		UPMQQueueBrowser consumer = new UPMQQueueBrowser(this, queue, messageSelector, null, false, UPMQConsumer.BROWSER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	@Override
	public MessageConsumer createConsumer(Destination destination, String messageSelector, boolean noLocal) throws JMSException {
		return _createConsumer(destination, messageSelector, null, noLocal);
	}

	public TopicSubscriber createSubscriber(Topic topic, String messageSelector, boolean noLocal) throws JMSException {
		checkClosed();

		UPMQTopicSubscriber consumer = new UPMQTopicSubscriber(this, topic, messageSelector, null, noLocal, UPMQConsumer.CONSUMER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	@Override
	public TopicSubscriber createDurableSubscriber(Topic topic, String name) throws JMSException {
		checkClosed();
		checkTopicSession();

		UPMQTopicSubscriber consumer = new UPMQTopicSubscriber(this, topic, null, name, false, UPMQConsumer.DURABLE_CONSUMER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	@Override
	public TopicSubscriber createDurableSubscriber(Topic topic, String name, String messageSelector, boolean noLocal) throws JMSException {
		checkClosed();
		checkTopicSession();

		UPMQTopicSubscriber consumer = new UPMQTopicSubscriber(this, topic, messageSelector, name, noLocal, UPMQConsumer.DURABLE_CONSUMER);
		addConsumer(consumer);
		if (isStarted())
			consumer.start();
		return consumer;
	}

	public UPMQMessageProducer _createProducer(Destination destination) throws JMSException {
		checkClosed();

		UPMQMessageProducer producer = new UPMQMessageProducer(this, destination);
		addProducer(producer);
		return producer;
	}

	public UPMQMessageConsumer _createConsumer(Destination destination, String messageSelector, String subscription, boolean NoLocal) throws JMSException {
		checkClosed();

		if (destination == null)
			throw new InvalidDestinationException("Don't understand null destinations");

		UPMQMessageConsumer consumer = new UPMQMessageConsumer(this, destination, messageSelector, subscription, NoLocal, UPMQMessageConsumer.UPMQConsumer.CONSUMER);
		addConsumer(consumer);
		if (isStarted()) {
			consumer.start();
		}
		return consumer;
	}

	@Override
	public Queue createQueue(String queueName) throws JMSException {
		checkClosed();
		checkQueueSession();

		if (queueName.startsWith(UPMQDestination.QUEUE_PREFIX))
			return new UPMQQueue(queueName.substring(UPMQDestination.QUEUE_PREFIX.length()));
		else
			return new UPMQQueue(queueName);
	}

	@Override
	public Topic createTopic(String topicName) throws JMSException {
		checkClosed();
		checkTopicSession();

		if (topicName.startsWith(UPMQDestination.TOPIC_PREFIX))
			return new UPMQTopic(topicName.substring(UPMQDestination.TOPIC_PREFIX.length()));
		else
			return new UPMQTopic(topicName);
	}

	@Override
	public TemporaryQueue createTemporaryQueue() throws JMSException {
		checkClosed();
		checkQueueSession();

		return new UPMQTemporaryQueue();
	}

	@Override
	public TemporaryTopic createTemporaryTopic() throws JMSException {
		checkClosed();
		checkTopicSession();

		return new UPMQTemporaryTopic();
	}

	@Override
	public void unsubscribe(String name) throws JMSException {
		checkClosed();
		checkTopicSession();

		for (Iterator<UPMQMessageConsumer> iterator = _consumers.iterator(); iterator.hasNext();) {
			final UPMQMessageConsumer messageConsumer = iterator.next();
			if (messageConsumer._type == UPMQConsumer.DURABLE_CONSUMER && messageConsumer._subscription != null && messageConsumer._subscription.equals(name)) {
				messageConsumer.deleteDurableConsumer();
				messageConsumer.unsubscription();
			}
		}
	}

	@Override
	public void run() {
		if (_messageListener != null) {
			while (_messageList.size() != 0) {
				Message message = _messageList.remove(0);
				try {
					if (getAcknowledgeMode() != Session.CLIENT_ACKNOWLEDGE)
						message.acknowledge();
				} catch (JMSException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				_messageListener.onMessage(message);
			}
		}
	}

	public void checkClosed() throws JMSException {
		if (_isClosed.get())
			throw new javax.jms.IllegalStateException("Cannot perform operation - session has been closed");
	}

	private void checkTopicSession() throws IllegalStateException {
		if (this instanceof QueueSession || this instanceof XAQueueSession)
			throw new javax.jms.IllegalStateException("Cannot perform operation");
	}

	private void checkQueueSession() throws IllegalStateException {
		if (this instanceof TopicSession || this instanceof XATopicSession)
			throw new javax.jms.IllegalStateException("Cannot perform operation");
	}

	public boolean isStarted() {
		return _isStarted.get();
	}

	protected boolean isStoped() {
		return _isStoped.get();
	}

	protected boolean isClosed() {
		return _isClosed.get();
	}

	private void checkTransactional() throws JMSException {
		if (_acknowledgeMode != SESSION_TRANSACTED) {
			throw new javax.jms.IllegalStateException("Cannot perform operation - session is not transactional");
		}
	}

	private void checkNotTransactional() throws JMSException {
		if (_acknowledgeMode == SESSION_TRANSACTED) {
			throw new javax.jms.IllegalStateException("Cannot perform operation - session is transactional");
		}
	}

	protected void addConsumer(final UPMQMessageConsumer consumer) throws JMSException {
		synchronized (lock) {
			_consumers.add(consumer);
			_connection.addDispatcher(consumer._objectId, consumer);
		}
	}

	public void removeConsumer(final UPMQMessageConsumer consumer) {
		synchronized (lock) {
			_consumers.remove(consumer);
			_connection.removeDispatcher(consumer._objectId);
		}
	}

	protected void addProducer(final UPMQMessageProducer producer) throws JMSException {
		synchronized (lock) {
			_producers.add(producer);
		}
	}

	public void removeProducer(final UPMQMessageProducer producer) {
		synchronized (lock) {
			_consumers.remove(producer);
		}
	}

	public synchronized void SyncOnMessage(MessageListener messageListener, Message msg) {
		messageListener.onMessage(msg);
	}
}
