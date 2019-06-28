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

import java.util.UUID;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.jms.BytesMessage;
import javax.jms.DeliveryMode;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageProducer;
import javax.jms.Queue;
import javax.jms.StreamMessage;
import javax.jms.Topic;

import org.apache.log4j.Logger;

import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.foreign.ForeignConverter;
import com.broker.libupmq.foreign.ForeignConverterFactory;
import com.broker.libupmq.message.UPMQBytesMessage;
import com.broker.libupmq.message.UPMQMessage;
import com.broker.libupmq.message.UPMQStreamMessage;
import com.broker.libupmq.session.UPMQSession;
import com.broker.libupmq.transport.UPMQCommand;
import com.broker.protocol.Protocol;

public class UPMQMessageProducer implements MessageProducer {
	private static final Logger log = Logger.getLogger(UPMQMessageProducer.class);

	public String _objectId = UUID.randomUUID().toString();
	public UPMQSession _session = null;

	private final AtomicBoolean _isClosed = new AtomicBoolean(false);

	private Destination _defaultDestination = null;
	private Destination _lastUsedDestination = null;
	private boolean _isDisableMessageId = false;
	private boolean _isDisableMessageTimestamp = false;

	private int _defaultPriority = Message.DEFAULT_PRIORITY;
	private long _defaultTimeToLive = Message.DEFAULT_TIME_TO_LIVE;
	private long _defaultDeliveryMode = DeliveryMode.PERSISTENT;

	public UPMQMessageProducer(UPMQSession sessionImpl, Destination destination) throws JMSException {
		_session = sessionImpl;
		_defaultDestination = destination;

		if (sessionImpl == null)
			throw new JMSException("Invalid parameter (is null)");
		if (destination != null) {
			sender((UPMQDestination) destination);
			_lastUsedDestination = destination;
		}

		log.debug("+prod: " + _objectId);
	}

	private void sender(UPMQDestination destination) throws JMSException {
		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getSenderBuilder().setReceiptId(_objectId);
		messageb.getSenderBuilder().setDestinationUri(destination.getUri());
		messageb.getSenderBuilder().setSessionId(_session._objectId);
		messageb.getSenderBuilder().setSenderId(_objectId);

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();
	}

	private void unsender() throws JMSException {

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setObjectId(_objectId);

		messageb.getUnsenderBuilder().setReceiptId(_objectId);
		messageb.getUnsenderBuilder().setDestinationUri((_defaultDestination != null) ? ((UPMQDestination) _defaultDestination).getUri() : "");
		messageb.getUnsenderBuilder().setSessionId(_session._objectId);
		messageb.getUnsenderBuilder().setSenderId(_objectId);

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
		response.processReceipt();
	}

	@Override
	public void close() throws JMSException {
		if (_isClosed.get() == true)
			return;

		unsender();

		_isClosed.set(true);
		_session.removeProducer(this);

		log.debug("-prod: " + _objectId);
	}

	@Override
	public void setDisableMessageID(boolean value) throws JMSException {
		_isDisableMessageId = value;
	}

	@Override
	public boolean getDisableMessageID() throws JMSException {
		return _isDisableMessageId;
	}

	@Override
	public void setDisableMessageTimestamp(boolean value) throws JMSException {
		_isDisableMessageTimestamp = value;
	}

	@Override
	public boolean getDisableMessageTimestamp() throws JMSException {
		return _isDisableMessageTimestamp;
	}

	@Override
	public void setDeliveryMode(int deliveryMode) throws JMSException {
		_defaultDeliveryMode = deliveryMode;
	}

	@Override
	public int getDeliveryMode() throws JMSException {
		return (int) _defaultDeliveryMode;
	}

	@Override
	public void setPriority(int defaultPriority) throws JMSException {
		_defaultPriority = defaultPriority;
	}

	@Override
	public int getPriority() throws JMSException {
		return _defaultPriority;
	}

	@Override
	public void setTimeToLive(long timeToLive) throws JMSException {
		_defaultTimeToLive = timeToLive;
	}

	@Override
	public long getTimeToLive() throws JMSException {
		return _defaultTimeToLive;
	}

	@Override
	public Destination getDestination() throws JMSException {
		return _defaultDestination;
	}

	@Override
	public void send(Message message) throws JMSException {
		if (getDestination() == null)
			throw new JMSException("destination not set");
		send(getDestination(), message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	@Override
	public void send(Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		if (getDestination() == null)
			throw new JMSException("destination not set");
		send(getDestination(), message, deliveryMode, priority, timeToLive);
	}

	@Override
	public void send(Destination destination, Message message) throws JMSException {
		if (destination == null)
			throw new JMSException("destination not set");

		send(destination, message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	@Override
	public void send(Destination destination, Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {

		checkClosed();
		if (_lastUsedDestination == null || ((UPMQDestination) _lastUsedDestination).getUri().compareTo(((UPMQDestination) destination).getUri()) != 0) {
			sender((UPMQDestination) destination);
			_lastUsedDestination = destination;
		}

		UPMQMessage messageImpl = null;
		if (message instanceof UPMQMessage) {
			messageImpl = (UPMQMessage) message;
		} else {
			ForeignConverter converter = ForeignConverterFactory.create(message);
			messageImpl = (UPMQMessage) converter.convert(message);
		}

		//SessionId
		messageImpl._headerBuilder.setSessionId(_session._objectId);

		// DeliveryModeValue
		messageImpl._headerBuilder.setPersistent(deliveryMode == DeliveryMode.PERSISTENT ? true : false);

		// Priority
		messageImpl._headerBuilder.setPriority(priority);

		// MessageId (set by provider after every send call)
		String newMsgId = assignNewId();
		messageImpl._headerBuilder.setMessageId(newMsgId);

// FIXME : for art : set by default for temporary compatibility
		// ReceiptId ()
		messageImpl._headerBuilder.setReceiptId(newMsgId);

		// SenderId
		messageImpl._headerBuilder.setSenderId(_objectId);

		long currTimeMillis = 0;
		if (timeToLive > 0 || _isDisableMessageTimestamp == false) {
			currTimeMillis = System.currentTimeMillis();
		}

		// TimeToLive + Expiration
		if (timeToLive > 0) {
			messageImpl._headerBuilder.setTimetolive(timeToLive);
			messageImpl._headerBuilder.setExpiration(timeToLive + currTimeMillis);
		} else {
			messageImpl._headerBuilder.setTimetolive(0);
			messageImpl._headerBuilder.setExpiration(0);
		}

		// TimeStamp
		if (_isDisableMessageTimestamp == false) {
			messageImpl._headerBuilder.setTimestamp(currTimeMillis);
		}

		//Expiration
		//		if (messageImpl._headerBuilder.getTimetolive() != 0 && messageImpl._headerBuilder.getExpiration() == 0)
		//			messageImpl._headerBuilder.setExpiration(currTimeMillis + messageImpl._headerBuilder.getTimetolive());
		//		else if (messageImpl._headerBuilder.getTimetolive() == 0 && messageImpl._headerBuilder.getExpiration() != 0)
		//			messageImpl._headerBuilder.setTimetolive(messageImpl._headerBuilder.getExpiration() - currTimeMillis);
		//		else if (messageImpl._headerBuilder.getTimetolive() != 0 && messageImpl._headerBuilder.getExpiration() != 0)
		//			messageImpl._headerBuilder.setTimetolive(messageImpl._headerBuilder.getExpiration() - currTimeMillis);

		// Destination
		UPMQDestination destinationImpl = (UPMQDestination) destination;
		messageImpl._headerBuilder.setDestinationUri(destinationImpl.getUri());
		messageImpl._destination = destination;

		Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
		messageb.setMessage(messageImpl._headerBuilder);
		messageb.setObjectId(_objectId);

		if (messageImpl instanceof BytesMessage)
			((UPMQBytesMessage) messageImpl).close();
		else if (messageImpl instanceof StreamMessage)
			((UPMQStreamMessage) messageImpl).close();

		if (messageImpl._uri != null && messageImpl._bodyBuilder.build().getSerializedSize() != 0) {
			throw new JMSException("not supported yet (uri + body)");
		}

		UPMQCommand response = (UPMQCommand) _session._connection.syncSendPacket(new UPMQCommand(messageb, messageImpl._bodyBuilder, messageImpl._uri));
		response.processReceipt();
	}

	public Topic getTopic() throws JMSException {
		return (Topic) _defaultDestination;
	}

	public Queue getQueue() throws JMSException {
		return (Queue) _defaultDestination;
	}

	public void publish(Message message) throws JMSException {
		send(getDestination(), message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	public void publish(Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		send(getDestination(), message, deliveryMode, priority, timeToLive);
	}

	public void publish(Topic topic, Message message) throws JMSException {
		if (topic == null)
			throw new JMSException("Destination not set");
		send(topic, message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	public void publish(Topic topic, Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		if (topic == null)
			throw new JMSException("Destination not set");
		send(topic, message, deliveryMode, priority, timeToLive);
	}

	public void send(Queue queue, Message message) throws JMSException {
		if (queue == null)
			throw new JMSException("Destination not set");
		send((Destination) queue, message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	public void send(Queue queue, Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		if (queue == null)
			throw new JMSException("Destination not set");
		send((Destination) queue, message, deliveryMode, priority, timeToLive);
	}

	public String assignNewId() {
		return "ID:" + UUID.randomUUID().toString();
	}

	public void checkClosed() throws JMSException {
		if (_isClosed.get()) {
			throw new javax.jms.IllegalStateException("Cannot perform operation - producer has been closed");
		}
	}
}
