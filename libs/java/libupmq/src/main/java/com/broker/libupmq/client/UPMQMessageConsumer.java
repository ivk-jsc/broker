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
 
package com.broker.libupmq.client;

import java.util.Enumeration;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.jms.ConnectionConsumer;
import javax.jms.Destination;
import javax.jms.InvalidDestinationException;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageListener;
import javax.jms.Queue;
import javax.jms.ServerSession;
import javax.jms.ServerSessionPool;
import javax.jms.Session;
import javax.jms.Topic;

import org.apache.log4j.Logger;

import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.message.UPMQBytesMessage;
import com.broker.libupmq.message.UPMQMapMessage;
import com.broker.libupmq.message.UPMQMessage;
import com.broker.libupmq.message.UPMQObjectMessage;
import com.broker.libupmq.message.UPMQStreamMessage;
import com.broker.libupmq.message.UPMQTextMessage;
import com.broker.libupmq.session.UPMQSession;
import com.broker.libupmq.transport.Command;
import com.broker.libupmq.transport.MessageDispatchChannel;
import com.broker.libupmq.transport.SimplePriorityMessageDispatchChannel;
import com.broker.libupmq.transport.UPMQCommand;
import com.broker.libupmq.transport.UPMQDispatcher;
import com.broker.protocol.Protocol;

public class UPMQMessageConsumer implements MessageConsumer, ConnectionConsumer, Enumeration<Object>, UPMQDispatcher, Runnable, MessageListener {
	private static final Logger log = Logger.getLogger(UPMQMessageConsumer.class);

	public String _objectId = UUID.randomUUID().toString();
	public UPMQSession _session = null;

	private final AtomicBoolean _isStarted = new AtomicBoolean(false);
	private final AtomicBoolean _isStoped = new AtomicBoolean(true);
	private final AtomicBoolean _isClosed = new AtomicBoolean(false);

	private Destination _defaultDestination = null;
	private UPMQDestination _defaultDestinationImpl = null;

	private String _messageSelector = null;
	private boolean _noLocal = false;
	public String _subscription = null;
	public UPMQConsumer _type;
	private boolean _deleteDurableConsumer = false;
	private ServerSessionPool _serverSessionPool = null;

	public static enum UPMQConsumer {
		CONSUMER, DURABLE_CONSUMER, BROWSER
	};

	public static int TIMEOUT = 1000;

	private Thread _OnMessageThread = null;
	private final AtomicBoolean _isSubscriptioned = new AtomicBoolean(false);
	private final AtomicBoolean _isSubscribed = new AtomicBoolean(false);
	Lock activeOnMessageLock = new ReentrantLock();

	private AtomicReference<MessageListener> _messageListener = new AtomicReference<MessageListener>();
	public MessageDispatchChannel _messageQueue = new SimplePriorityMessageDispatchChannel();

	public long _browserCount = 0;

	public UPMQMessageConsumer(UPMQSession sessionImpl, Destination destination, String messageSelector, String subscription, boolean noLocal, UPMQConsumer type) throws JMSException {
		_session = sessionImpl;
		_defaultDestination = destination;
		_defaultDestinationImpl = (UPMQDestination) destination;
		_messageSelector = messageSelector;
		_subscription = subscription;
		_noLocal = noLocal;
		_type = type;

		//		if (_session == null)
		//			throw new JMSException("Invalid parameter (is null)");

		if (destination == null)
			throw new InvalidDestinationException("Invalid parameter (is null)");

		subscription();

		//No need, look for UPMQSession: addConsumer(consumer); consumer.start();
		//		if (_session != null && _session.isStarted()) {
		//			start();
		//		}
	}

	@Override
	public void close() throws JMSException {
		if (isClosed())
			return;

		activeOnMessageLock.lock();
		activeOnMessageLock.unlock();

		if (isStarted() || (isStoped() && !isClosed()))
			unsubscription();

		_messageQueue.clear();
		_messageQueue.close();

		_isStarted.set(false);
		_isStoped.set(true);
		_isClosed.set(true);

		if (_OnMessageThread != null) {
			try {
				_OnMessageThread.join();
				_OnMessageThread = null;
			} catch (InterruptedException e) {
				JMSException ex = new JMSException("message thread join error");
				ex.setLinkedException(e);
				ex.printStackTrace();
			}
		}

		_session.removeConsumer(this);
	}

	public void deleteDurableConsumer() throws JMSException {
		_deleteDurableConsumer = true;
	}

	public Message areceive() throws JMSException {
		checkClosed();

		Command command = null;
		UPMQMessage message = null;

		do {
			command = dequeue(-1);
			if (command == null)
				return null;

			message = CommandToUPMQMessage(command);

		} while (message.isExpired());

		return message;
	}

	@Override
	public Message receive() throws JMSException {
		checkClosed();

		Command command = null;
		UPMQMessage message = null;

		do {
			command = dequeue(-1);
			if (command == null)
				return null;

			message = CommandToUPMQMessage(command);
		} while (message.isExpired());

		if (_session.getAcknowledgeMode() != Session.CLIENT_ACKNOWLEDGE)
			message.acknowledge();

		return message;
	}

	@Override
	public Message receive(long timeout) throws JMSException {
		checkClosed();

		Command command = null;
		UPMQMessage message = null;

		do {
			if (timeout == 0)
				command = dequeue(-1);
			else
				command = dequeue(timeout);

			if (command == null)
				return null;

			message = CommandToUPMQMessage(command);
		} while (message.isExpired());

		if (_session.getAcknowledgeMode() != Session.CLIENT_ACKNOWLEDGE)
			message.acknowledge();

		return message;
	}

	@Override
	public Message receiveNoWait() throws JMSException {
		checkClosed();

		Command command = null;
		UPMQMessage message = null;

		do {
			command = dequeue(0);
			if (command == null)
				return null;

			message = CommandToUPMQMessage(command);
		} while (message.isExpired());

		if (_session.getAcknowledgeMode() != Session.CLIENT_ACKNOWLEDGE)
			message.acknowledge();

		return message;
	}

	@Override
	public void setMessageListener(MessageListener listener) throws JMSException {
		checkClosed();

		activeOnMessageLock.lock();
		_messageListener.set(listener);
		activeOnMessageLock.unlock();

		if (getMessageListener() != null) {
			if (_session._connection.isStarted())
				start();
		} else {
			stop();
		}
	}

	@Override
	public void run() {
		log.debug("OnMessage thread start (" + _objectId + ")");

		Message msg = null;
		while (isStarted() && !isStoped() && _messageListener.get() != null) {

			try {
				msg = areceive();
				if (msg != null) {
					activeOnMessageLock.lock();
					if (isStarted() && !isStoped() && _messageListener.get() != null) {
						if (_session.getAcknowledgeMode() != Session.CLIENT_ACKNOWLEDGE)
							msg.acknowledge();
						_session.SyncOnMessage(_messageListener.get(), msg);
					}
					activeOnMessageLock.unlock();
				}
			} catch (IllegalStateException e) {
				continue;
			} catch (JMSException e) {
				JMSException ex = new JMSException("Async receive thread error");
				ex.setLinkedException(e);
				ex.printStackTrace();
			}
		}

		log.debug("OnMessage thread end   (" + _session._connection._objectId + ")");
	}

	@Override
	public MessageListener getMessageListener() throws JMSException {
		checkClosed();

		return _messageListener.get();
	}

	@Override
	public String getMessageSelector() throws JMSException {
		return _messageSelector;
	}

	public void start() throws JMSException {

		_isStarted.set(true);
		_isStoped.set(false);
		_isClosed.set(false);

		_messageQueue.start();

		if (_OnMessageThread == null && getMessageListener() != null) {
			_OnMessageThread = new Thread(this, "UPMQConsumer: " + _objectId);
			_OnMessageThread.start();
		}

		subscribe();
	}

	public void stop() throws JMSException {

		unsubscribe();

		_isStarted.set(false);
		_isStoped.set(true);

		_messageQueue.clear();
		_messageQueue.stop();

		try {
			if (_OnMessageThread != null) {
				_messageQueue.close();//need areceive return before join
				_OnMessageThread.join();
				_OnMessageThread = null;
			}
		} catch (InterruptedException e) {
			JMSException ex = new JMSException("message thread join error");
			ex.setLinkedException(e);
			ex.printStackTrace();
		}
	}

	public Topic getTopic() throws JMSException {
		return (Topic) _defaultDestination;
	}

	public boolean getNoLocal() throws JMSException {
		return _noLocal;
	}

	// QueueBrowser method start
	// ----------------------------------------------------
	public Enumeration<Object> getEnumeration() throws JMSException {

		//		if (_session.isStarted() && _type == UPMQConsumer.BROWSER) {
		//			start();
		//		}

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		//TODO set revision number
		messageb.getBrowserBuilder().setDestinationUri(_defaultDestinationImpl.getUri());
		messageb.getBrowserBuilder().setSubscriptionName(_subscription);

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		_browserCount = response.processBrowserInfo();

		return this;
	}

	@Override
	public boolean hasMoreElements() {

		if (_browserCount == 0)
			return false;
		else
			return true;
	}

	@Override
	public Object nextElement() {

		try {
			Message message = receive();
			if (message != null)
				_browserCount--;
			return message;
		} catch (JMSException e) {
			// TODO: log error
			e.printStackTrace();
			return null;
		}
	}

	public Queue getQueue() throws JMSException {
		return (Queue) _defaultDestination;
	}
	// QueueBrowser method end
	// ------------------------------------------------------

	// ConnectionConsumer method start
	// ----------------------------------------------------
	@Override
	public ServerSessionPool getServerSessionPool() throws JMSException {
		checkClosed();
		return _serverSessionPool;
	}

	public void setServerSessionPool(ServerSessionPool serverSessionPool) {
		_serverSessionPool = serverSessionPool;
	}
	// ConnectionConsumer method end
	// ------------------------------------------------------

	public void checkClosed() throws JMSException {
		if (_isClosed.get()) {
			throw new javax.jms.IllegalStateException("Cannot perform operation - consumer has been closed");
		}
	}

	public String getDestinationId() {
		return _defaultDestinationImpl.getUri();
	}

	public String getDestinationName() {
		return _defaultDestinationImpl.getUri();
	}

	protected boolean isStarted() {
		return _isStarted.get();
	}

	protected boolean isStoped() {
		return _isStoped.get();
	}

	protected boolean isClosed() {
		return _isClosed.get();
	}

	@Override
	public void dispatch(Command command) {
		_messageQueue.enqueue(command);
	}

	private Command dequeue(long timeout) throws JMSException {
		try {
			return _messageQueue.dequeue(timeout);
		} catch (InterruptedException e) {
			return null;
			// TODO:
			// Thread.currentThread().interrupt();
			// throw JMSException
		}
	}

	UPMQMessage CommandToUPMQMessage(Command md) throws JMSException {

		final UPMQCommand command = (UPMQCommand) md;

		UPMQMessage resultMessage = null;
		switch (command._headerMessage.getMessage().getBodyType()) {
		case 0:
			resultMessage = new UPMQMessage(this);
			if (command._uri != null)
				resultMessage._uri = command._uri;
			break;
		case Protocol.Body.TEXT_BODY_FIELD_NUMBER:
			resultMessage = new UPMQTextMessage(this);
			break;
		case Protocol.Body.MAP_BODY_FIELD_NUMBER:
			resultMessage = new UPMQMapMessage(this);
			break;
		case Protocol.Body.OBJECT_BODY_FIELD_NUMBER:
			resultMessage = new UPMQObjectMessage(this);
			break;
		case Protocol.Body.BYTES_BODY_FIELD_NUMBER:
			resultMessage = new UPMQBytesMessage(this);
			break;
		case Protocol.Body.STREAM_BODY_FIELD_NUMBER:
			resultMessage = new UPMQStreamMessage(this);
			break;
		default:
			throw new JMSException("Error message type on receive");
		}

		resultMessage.setMessage(command._headerMessage.getMessage().toBuilder());
		if (command._bodyMessage != null)
			resultMessage.setBody(command._bodyMessage.toBuilder());

		switch (command._headerMessage.getMessage().getBodyType()) {
		case Protocol.Body.BYTES_BODY_FIELD_NUMBER:
			((UPMQBytesMessage) resultMessage).reset();
			break;
		case Protocol.Body.STREAM_BODY_FIELD_NUMBER:
			((UPMQStreamMessage) resultMessage).reset();
			break;
		}

		return resultMessage;
	}

	public void subscription() throws JMSException {

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getSubscriptionBuilder().setReceiptId(_objectId);
		messageb.getSubscriptionBuilder().setDestinationUri(_defaultDestinationImpl.getUri());
		messageb.getSubscriptionBuilder().setSessionId(_session._objectId);
		messageb.getSubscriptionBuilder().setNoLocal(_noLocal);

		if (_subscription == null)
			if ((_defaultDestinationImpl.getUri().startsWith(UPMQDestination.QUEUE_PREFIX) || _defaultDestinationImpl.getUri().startsWith(UPMQDestination.TEMP_QUEUE_PREFIX))
					&& _type != UPMQConsumer.BROWSER)
				_subscription = _defaultDestinationImpl.getUri();
			else
				_subscription = _objectId;

		messageb.getSubscriptionBuilder().setSubscriptionName(_subscription);

		if (_messageSelector != null)
			messageb.getSubscriptionBuilder().setSelector(_messageSelector);

		if (_type == UPMQConsumer.DURABLE_CONSUMER)
			messageb.getSubscriptionBuilder().setDurable(true);
		else
			messageb.getSubscriptionBuilder().setDurable(false);

		if (_type == UPMQConsumer.BROWSER)
			messageb.getSubscriptionBuilder().setBrowse(true);
		else
			messageb.getSubscriptionBuilder().setBrowse(false);

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		_isSubscriptioned.set(true);

		log.debug("+cons: " + _objectId + " (" + _type.toString() + ", " + _defaultDestinationImpl.getUri() + ")");
	}

	public void unsubscription() throws JMSException {
		if (isSubscribed())
			unsubscribe();

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getUnsubscriptionBuilder().setReceiptId(_objectId);
		messageb.getUnsubscriptionBuilder().setDestinationUri(_defaultDestinationImpl.getUri());
		messageb.getUnsubscriptionBuilder().setSessionId(_session._objectId);
		messageb.getUnsubscriptionBuilder().setSubscriptionName(_subscription);

		if (_type == UPMQConsumer.DURABLE_CONSUMER)
			messageb.getUnsubscriptionBuilder().setDurable(true);
		else
			messageb.getUnsubscriptionBuilder().setDurable(false);

		if (_type == UPMQConsumer.BROWSER)
			messageb.getUnsubscriptionBuilder().setBrowse(true);
		else
			messageb.getUnsubscriptionBuilder().setBrowse(false);

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		_isSubscriptioned.set(false);
		log.debug("-cons: " + _objectId + " (" + _type.toString() + ", " + _defaultDestinationImpl.getUri() + ")");
	}

	public void subscribe() throws JMSException {
		if (isSubscribed())
			return;

		if (!isSubscriptioned()) {
			subscription();
		}

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getSubscribeBuilder().setReceiptId(_objectId);
		messageb.getSubscribeBuilder().setDestinationUri(_defaultDestinationImpl.getUri());
		messageb.getSubscribeBuilder().setSubscriptionName(_subscription);
		messageb.getSubscribeBuilder().setSessionId(_session._objectId);

		if (_type == UPMQConsumer.BROWSER)
			messageb.getSubscribeBuilder().setBrowse(true);
		else
			messageb.getSubscribeBuilder().setBrowse(false);

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		_isSubscribed.set(true);
		log.debug("+subs: " + _objectId + " (" + _type.toString() + ", " + _defaultDestinationImpl.getUri() + ")");
	}

	public void unsubscribe() throws JMSException {
		if (!isSubscribed())
			return;

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getUnsubscribeBuilder().setReceiptId(_objectId);
		messageb.getUnsubscribeBuilder().setDestinationUri(_defaultDestinationImpl.getUri());
		messageb.getUnsubscribeBuilder().setSubscriptionName(_subscription);
		messageb.getUnsubscribeBuilder().setSessionId(_session._objectId);
		messageb.getUnsubscribeBuilder().setDurable(_deleteDurableConsumer);

		if (_type == UPMQConsumer.BROWSER)
			messageb.getUnsubscribeBuilder().setBrowse(true);
		else
			messageb.getUnsubscribeBuilder().setBrowse(false);

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();

		_isSubscribed.set(false);
		log.debug("-subs: " + _objectId + " (" + _type.toString() + ", " + _defaultDestinationImpl.getUri() + ")");
	}

	private boolean isSubscribed() {
		return _isSubscribed.get();
	}

	private boolean isSubscriptioned() {
		return _isSubscriptioned.get();
	}

	@Override
	public void onMessage(Message message) {
		if (_serverSessionPool != null) {
			try {
				ServerSession serverSession = _serverSessionPool.getServerSession();
				Session s = serverSession.getSession();
				UPMQSession session = null;

				if (s instanceof UPMQSession) {
					session = (UPMQSession) s;
				} else {
					// FIXME new JMSException("Session pool provided an invalid session type: " + s.getClass()));
					return;
				}
				session._messageList.add(message);
				serverSession.start();
			} catch (JMSException e) {
				// FIXME
			}
		}
	}
}
