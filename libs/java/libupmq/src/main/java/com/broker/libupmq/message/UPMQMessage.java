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
 
package com.broker.libupmq.message;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Map;

import javax.jms.DeliveryMode;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageFormatException;
import javax.jms.MessageNotReadableException;
import javax.jms.MessageNotWriteableException;
import javax.xml.bind.DatatypeConverter;

import com.broker.jms.FileMessage;
import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.destination.UPMQQueue;
import com.broker.libupmq.destination.UPMQTemporaryQueue;
import com.broker.libupmq.destination.UPMQTemporaryTopic;
import com.broker.libupmq.destination.UPMQTopic;
import com.broker.libupmq.transport.UPMQCommand;
import com.broker.protocol.Protocol;
import com.google.protobuf.Descriptors.FieldDescriptor;

public class UPMQMessage implements Message, FileMessage {

	public static final String JMSX_GROUP_ID = "JMSXGroupID";
	public static final String JMSX_GROUP_SEQ = "JMSXGroupSeq";
	public static final String JMSX_DELIVERY_COUNT = "JMSXDeliveryCount";
	public final static String[] reservedKeywords = { "NULL", "TRUE", "FALSE", "NOT", "AND", "OR", "BETWEEN", "LIKE", "IN", "IS", "ESCAPE" };
	public final static String[] reservedProperties = { "JMSXUserID", "JMSXAppID", "JMSXProducerTXID", "JMSXConsumerTXID", "JMSXRcvTimestamp", "JMSXState", "JMSXDeliveryCount" };

	UPMQMessageConsumer _consumer = null;

	public Protocol.Message.Builder _headerBuilder = null;
	public Protocol.Body.Builder _bodyBuilder = null;
	public URI _uri;

	public Destination _destination = null;
	public Destination _replyto = null;

	protected boolean _propertiesReadOnly = false;
	protected boolean _bodyReadOnly = false;

	public UPMQMessage(UPMQMessageConsumer consumer) throws JMSException {

		_consumer = consumer;

		_headerBuilder = Protocol.Message.newBuilder();
		_bodyBuilder = Protocol.Body.newBuilder();

		_headerBuilder.setBodyType(0);
	}

	//TODO one ack send check
	@Override
	public void acknowledge() throws JMSException {

		if (_consumer != null) {
			_consumer.checkClosed();

			Protocol.ProtoMessage.Builder messageb = Protocol.ProtoMessage.newBuilder();
			String msgID = this._headerBuilder.getMessageId();
			messageb.setObjectId(_consumer._objectId);

			messageb.getAckBuilder().setMessageId(msgID);
			messageb.getAckBuilder().setSessionId(_consumer._session._objectId);
			messageb.getAckBuilder().setDestinationUri(this._headerBuilder.getDestinationUri());
			messageb.getAckBuilder().setReceiptId(msgID);
			messageb.getAckBuilder().setSubscriptionName(_consumer._subscription);

			//TODO add sync mode
			UPMQCommand response = (UPMQCommand) _consumer._session._connection.syncSendPacket(new UPMQCommand(messageb, null, null));
			response.processReceipt();
		}
	}

	@Override
	public Destination getJMSReplyTo() throws JMSException {

		if (_replyto != null)
			return _replyto;

		if (_headerBuilder.getReplyTo().startsWith(UPMQDestination.QUEUE_PREFIX))
			return new UPMQQueue(_headerBuilder.getReplyTo().substring(UPMQDestination.QUEUE_PREFIX.length()));
		else if (_headerBuilder.getReplyTo().startsWith(UPMQDestination.TEMP_QUEUE_PREFIX))
			return new UPMQTemporaryQueue(_headerBuilder.getReplyTo().substring(UPMQDestination.TEMP_QUEUE_PREFIX.length()));
		if (_headerBuilder.getReplyTo().startsWith(UPMQDestination.TOPIC_PREFIX))
			return new UPMQTopic(_headerBuilder.getReplyTo().substring(UPMQDestination.TOPIC_PREFIX.length()));
		else if (_headerBuilder.getReplyTo().startsWith(UPMQDestination.TEMP_TOPIC_PREFIX))
			return new UPMQTemporaryTopic(_headerBuilder.getReplyTo().substring(UPMQDestination.TEMP_TOPIC_PREFIX.length()));
		else
			return null;
	}

	@Override
	public void setJMSReplyTo(Destination replyTo) throws JMSException {

		_replyto = replyTo;

		if (_replyto != null) {
			UPMQDestination destinationImpl = (UPMQDestination) replyTo;
			_headerBuilder.setReplyTo(destinationImpl.getUri());
		} else {
			_headerBuilder.clearReplyTo();
		}
	}

	@Override
	public Destination getJMSDestination() throws JMSException {

		if (_destination != null)
			return _destination;

		if (_headerBuilder.getDestinationUri().startsWith(UPMQDestination.QUEUE_PREFIX))
			return new UPMQQueue(_headerBuilder.getDestinationUri().substring(UPMQDestination.QUEUE_PREFIX.length()));
		else if (_headerBuilder.getDestinationUri().startsWith(UPMQDestination.TEMP_QUEUE_PREFIX))
			return new UPMQTemporaryQueue(_headerBuilder.getDestinationUri().substring(UPMQDestination.TEMP_QUEUE_PREFIX.length()));
		if (_headerBuilder.getDestinationUri().startsWith(UPMQDestination.TOPIC_PREFIX))
			return new UPMQTopic(_headerBuilder.getDestinationUri().substring(UPMQDestination.TOPIC_PREFIX.length()));
		else if (_headerBuilder.getDestinationUri().startsWith(UPMQDestination.TEMP_TOPIC_PREFIX))
			return new UPMQTemporaryTopic(_headerBuilder.getDestinationUri().substring(UPMQDestination.TEMP_TOPIC_PREFIX.length()));
		else
			return null;
	}

	@Override
	public void setJMSDestination(Destination destination) throws JMSException {

		_destination = destination;

		if (_destination != null) {
			UPMQDestination destinationImpl = (UPMQDestination) destination;
			_headerBuilder.setDestinationUri(destinationImpl.getUri());
		} else {
			_headerBuilder.clearDestinationUri();
		}
	}

	@Override
	public String getJMSMessageID() throws JMSException {
		if (_headerBuilder.hasMessageId() == false)
			return null;
		else
			return _headerBuilder.getMessageId();
	}

	@Override
	public void setJMSMessageID(String id) throws JMSException {
		if (id == null)
			_headerBuilder.clearMessageId();
		else
			_headerBuilder.setMessageId(id);
	}

	@Override
	public long getJMSTimestamp() throws JMSException {
		return _headerBuilder.getTimestamp();
	}

	@Override
	public void setJMSTimestamp(long timestamp) throws JMSException {
		_headerBuilder.setTimestamp(timestamp);
	}

	@Override
	public byte[] getJMSCorrelationIDAsBytes() throws JMSException {
		if (_headerBuilder.hasCorrelationId() == false)
			return null;
		else
			return _headerBuilder.getCorrelationId().getBytes();
	}

	@Override
	public void setJMSCorrelationIDAsBytes(byte[] correlationID) throws JMSException {
		if (correlationID == null)
			_headerBuilder.clearCorrelationId();
		else
			_headerBuilder.setCorrelationId(new String(correlationID));
	}

	@Override
	public void setJMSCorrelationID(String correlationID) throws JMSException {
		if (correlationID == null)
			_headerBuilder.clearCorrelationId();
		else
			_headerBuilder.setCorrelationId(correlationID);
	}

	@Override
	public String getJMSCorrelationID() throws JMSException {
		if (_headerBuilder.hasCorrelationId() == false)
			return null;
		else
			return _headerBuilder.getCorrelationId();
	}

	@Override
	public int getJMSDeliveryMode() throws JMSException {
		if (_headerBuilder.getPersistent() == true)
			return DeliveryMode.PERSISTENT;
		else
			return DeliveryMode.NON_PERSISTENT;
	}

	@Override
	public void setJMSDeliveryMode(int deliveryMode) throws JMSException {
		if (deliveryMode == DeliveryMode.PERSISTENT)
			_headerBuilder.setPersistent(true);
		else
			_headerBuilder.setPersistent(false);
	}

	@Override
	public boolean getJMSRedelivered() throws JMSException {
		return _headerBuilder.getRedelivered();
	}

	@Override
	public void setJMSRedelivered(boolean redelivered) throws JMSException {
		_headerBuilder.setRedelivered(redelivered);
	}

	@Override
	public String getJMSType() throws JMSException {
		if (_headerBuilder.hasType() == false)
			return null;
		else
			return _headerBuilder.getType();
	}

	@Override
	public void setJMSType(String type) throws JMSException {
		if (type == null)
			_headerBuilder.clearType();
		else
			_headerBuilder.setType(type);
	}

	@Override
	public long getJMSExpiration() throws JMSException {
		return _headerBuilder.getExpiration();
	}

	@Override
	public void setJMSExpiration(long expiration) throws JMSException {
		_headerBuilder.setExpiration(expiration);
	}

	@Override
	public int getJMSPriority() throws JMSException {
		return _headerBuilder.getPriority();
	}

	@Override
	public void setJMSPriority(int priority) throws JMSException {
		_headerBuilder.setPriority(priority);
	}

	@Override
	public void clearProperties() throws JMSException {
		try {
			_headerBuilder.clearProperty();
			_propertiesReadOnly = false;
		} catch (Exception e) {
			JMSException ex = new JMSException("clear property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public boolean propertyExists(String name) throws JMSException {
		return _headerBuilder.getPropertyMap().containsKey(name);
	}

	public boolean propertyValueNotNull(String name) throws JMSException {
		return !_headerBuilder.getPropertyMap().get(name).getIsNull();
	}

	@Override
	public boolean getBooleanProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsBooleanType(name))
					return _headerBuilder.getPropertyMap().get(name).getValueBool();
				else
					return convertPropertyValueToBooleanType(name);
			else
				return false;
		else
			return false;
	}

	@Override
	public byte getByteProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsByteType(name))
					return (byte) _headerBuilder.getPropertyMap().get(name).getValueByte();
				else
					return convertPropertyValueToByteType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public short getShortProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsShortType(name))
					return (short) _headerBuilder.getPropertyMap().get(name).getValueShort();
				else
					return convertPropertyValueToShortType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public int getIntProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (UPMQMessage.JMSX_DELIVERY_COUNT.equals(name)) {
			return _headerBuilder.getDeliveryCount();
		} else if (UPMQMessage.JMSX_GROUP_SEQ.equals(name)) {
			return _headerBuilder.getGroupSeq();
		}

		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsIntType(name))
					return _headerBuilder.getPropertyMap().get(name).getValueInt();
				else
					return convertPropertyValueToIntType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public long getLongProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsLongType(name))
					return _headerBuilder.getPropertyMap().get(name).getValueLong();
				else
					return convertPropertyValueToLongType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public float getFloatProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsFloatType(name))
					return _headerBuilder.getPropertyMap().get(name).getValueFloat();
				else
					return convertPropertyValueToFloatType(name);
			else
				throw new NullPointerException();
		else
			throw new NullPointerException();
	}

	@Override
	public double getDoubleProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsDoubleType(name))
					return _headerBuilder.getPropertyMap().get(name).getValueDouble();
				else
					return convertPropertyValueToDoubleType(name);
			else
				throw new NullPointerException();
		else
			throw new NullPointerException();
	}

	@Override
	public String getStringProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (UPMQMessage.JMSX_GROUP_ID.equals(name)) {
			return _headerBuilder.getGroupId();
		}

		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsStringType(name))
					return _headerBuilder.getPropertyMap().get(name).getValueString();
				else
					return convertPropertyValueToStringType(name);
			else
				return null;
		else
			return null;
	}

	@Override
	public Object getObjectProperty(String name) throws JMSException {
		checkGetPropertyName(name, null);
		if (propertyExists(name))
			if (propertyValueNotNull(name))
				if (propertyValueIsObjectType(name))
					return UPMQMessage.deserializeObjectFromString(_headerBuilder.getPropertyMap().get(name).getValueObject());
				else
					return convertPropertyValueToObjectType(name);
			else
				return null;
		else
			return null;
	}

	@Override
	@SuppressWarnings("rawtypes")
	public Enumeration getPropertyNames() throws JMSException {
		return Collections.enumeration(_headerBuilder.getPropertyMap().keySet());
	}

	@Override
	public void setBooleanProperty(String name, boolean value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueBool(value);
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setByteProperty(String name, byte value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueByte(value);
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setShortProperty(String name, short value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueShort(value);
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setIntProperty(String name, int value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);

		if (UPMQMessage.JMSX_DELIVERY_COUNT.equals(name)) {
			_headerBuilder.setDeliveryCount(value);
			return;
		} else if (UPMQMessage.JMSX_GROUP_SEQ.equals(name)) {
			_headerBuilder.setGroupSeq(value);
			return;
		}

		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueInt(value);
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setLongProperty(String name, long value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueLong(value);
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setFloatProperty(String name, float value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueFloat(value);
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setDoubleProperty(String name, double value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueDouble(value);
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setStringProperty(String name, String value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);

		if (UPMQMessage.JMSX_GROUP_ID.equals(name)) {
			_headerBuilder.setGroupId(value);
			return;
		}

		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			if (value == null) {
				propertyb.setIsNull(true);
				propertyb.setValueString("null");
			} else {
				propertyb.setIsNull(false);
				propertyb.setValueString(value);
			}
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setObjectProperty(String name, Object value) throws JMSException {
		checkPropertyWrite();
		checkSetPropertyName(name, value);

		if (value instanceof Boolean | value instanceof Byte | value instanceof Short | value instanceof Integer | value instanceof Long | value instanceof Float | value instanceof Double
				| value instanceof String) {

		} else if (value == null) {

		} else {
			throw new MessageFormatException("invalid object type (should be Boolean, Byte, Short, Integer, Long, Float, Double, String)");
		}

		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(value == null ? true : false);
			propertyb.setValueObject(UPMQMessage.serializeObjectToString(value));
			_headerBuilder.putProperty(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set property error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void clearBody() throws JMSException {
		try {
			_bodyBuilder.clear();
			_bodyReadOnly = false;
		} catch (Exception e) {
			JMSException ex = new JMSException("clear body error");
			ex.setLinkedException(e);
			throw ex;
		}

	}

	public Message setMessage(Protocol.Message.Builder newBuilderForType) {
		_propertiesReadOnly = true;
		_bodyReadOnly = true;

		_headerBuilder = newBuilderForType;
		return this;
	}

	public Message setBody(Protocol.Body.Builder newBuilderForType) {
		_propertiesReadOnly = true;
		_bodyReadOnly = true;

		_bodyBuilder = newBuilderForType;
		return this;
	}

	public final void checkPropertyWrite() throws MessageNotWriteableException {
		if (_propertiesReadOnly)
			throw new MessageNotWriteableException("message properties in read-only mode");
	}

	public final void checkPropertyRead() throws MessageNotReadableException {
		if (!_propertiesReadOnly)
			throw new MessageNotReadableException("message properties in write-only mode");
	}

	protected void checkWrite() throws MessageNotWriteableException {
		if (_bodyReadOnly)
			throw new MessageNotWriteableException("message body in read-only mode");
	}

	protected void checkRead() throws MessageNotReadableException {
		if (!_bodyReadOnly)
			throw new MessageNotReadableException("message body in write-only mode");
	}

	public void checkSetPropertyName(String name, Object value) throws JMSException {
		if (name == null || name == "") {
			throw new IllegalArgumentException("invalid property name (null or empty)");
		}
		for (int i = 0; i < name.length(); i++) {
			if (i == 0 && !Character.isJavaIdentifierStart(name.charAt(i))) {
				throw new IllegalArgumentException("invalid property name");
			}
			if (i != 0 && !Character.isJavaIdentifierPart(name.charAt(i))) {
				throw new IllegalArgumentException("invalid property name");
			}
		}
		for (String reservedKeyword : reservedKeywords) {
			if (name.equalsIgnoreCase(reservedKeyword)) {
				throw new JMSException("invalid property name (reserved keyword)");
			}
		}
		for (String reservedProperty : reservedProperties) {
			if (name.equalsIgnoreCase(reservedProperty)) {
				throw new JMSException("invalid property name (reserved keyword)");
			}
		}

		if (UPMQMessage.JMSX_GROUP_ID.equals(name) && value != null && !(value instanceof String)) {
			throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_ID + " must be of type String");
		}
		if (UPMQMessage.JMSX_GROUP_SEQ.equals(name)) {
			if (value != null && !(value instanceof Integer)) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be of type int or Integer");
			}
			if (value != null && ((Number) value).intValue() < 1) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be a positive value");
			}
		}
		if (UPMQMessage.JMSX_GROUP_SEQ.equals(name)) {
			if (value != null && !(value instanceof Integer)) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be of type int or Integer");
			}
			if (value != null && ((Number) value).intValue() < 1) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be a positive value");
			}
		}
	}

	public void checkGetPropertyName(String name, Object value) throws JMSException {
		if (name == null || name == "") {
			throw new IllegalArgumentException("invalid property name (null or empty)");
		}
		for (int i = 0; i < name.length(); i++) {
			if (i == 0 && !Character.isJavaIdentifierStart(name.charAt(i))) {
				throw new IllegalArgumentException("invalid property name");
			}
			if (i != 0 && !Character.isJavaIdentifierPart(name.charAt(i))) {
				throw new IllegalArgumentException("invalid property name");
			}
		}
		for (String reservedKeyword : reservedKeywords) {
			if (name.equalsIgnoreCase(reservedKeyword)) {
				throw new JMSException("invalid property name (reserved keyword)");
			}
		}

		if (UPMQMessage.JMSX_GROUP_ID.equals(name) && value != null && !(value instanceof String)) {
			throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_ID + " must be of type String");
		}
		if (UPMQMessage.JMSX_GROUP_SEQ.equals(name)) {
			if (value != null && !(value instanceof Integer)) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be of type int or Integer");
			}
			if (value != null && ((Number) value).intValue() < 1) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be a positive value");
			}
		}
		if (UPMQMessage.JMSX_GROUP_SEQ.equals(name)) {
			if (value != null && !(value instanceof Integer)) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be of type int or Integer");
			}
			if (value != null && ((Number) value).intValue() < 1) {
				throw new MessageFormatException("property " + UPMQMessage.JMSX_GROUP_SEQ + " must be a positive value");
			}
		}
	}

	public Object convertPropertyValueToObjectType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_BOOL:
			return Boolean.valueOf(getBooleanProperty(name));
		case VALUE_BYTE:
			return Byte.valueOf(getByteProperty(name));
		case VALUE_DOUBLE:
			return Double.valueOf(getDoubleProperty(name));
		case VALUE_FLOAT:
			return Float.valueOf(getFloatProperty(name));
		case VALUE_INT:
			return Integer.valueOf(getIntProperty(name));
		case VALUE_LONG:
			return Long.valueOf(getLongProperty(name));
		case VALUE_SHORT:
			return Short.valueOf(getShortProperty(name));
		case VALUE_STRING:
			return String.valueOf(getStringProperty(name));
		case VALUE_OBJECT:
			return UPMQMessage.deserializeObjectFromString(_headerBuilder.getPropertyMap().get(name).getValueObject());
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsObjectType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_OBJECT)
			return true;
		else
			return false;
	}

	public boolean convertPropertyValueToBooleanType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_STRING:
			return Boolean.valueOf(getStringProperty(name));
		case VALUE_BYTE:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_INT:
		case VALUE_LONG:
		case VALUE_SHORT:
		case VALUE_OBJECT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsBooleanType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_BOOL)
			return true;
		else
			return false;
	}

	public int convertPropertyValueToIntType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_BYTE:
			return Integer.valueOf(getByteProperty(name));
		case VALUE_SHORT:
			return Integer.valueOf(getShortProperty(name));
		case VALUE_STRING:
			return Integer.valueOf(getStringProperty(name));
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_LONG:
		case VALUE_OBJECT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsIntType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_INT)
			return true;
		else
			return false;
	}

	public long convertPropertyValueToLongType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_BYTE:
			return Long.valueOf(getByteProperty(name));
		case VALUE_SHORT:
			return Long.valueOf(getShortProperty(name));
		case VALUE_INT:
			return Long.valueOf(getIntProperty(name));
		case VALUE_STRING:
			return Long.valueOf(getStringProperty(name));
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_OBJECT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsLongType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_LONG)
			return true;
		else
			return false;
	}

	public byte convertPropertyValueToByteType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_STRING:
			return Byte.valueOf(getStringProperty(name));
		case VALUE_INT:
		case VALUE_LONG:
		case VALUE_SHORT:
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_OBJECT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsByteType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_BYTE)
			return true;
		else
			return false;
	}

	public short convertPropertyValueToShortType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_BYTE:
			return Short.valueOf(getByteProperty(name));
		case VALUE_STRING:
			return Short.valueOf(getStringProperty(name));
		case VALUE_LONG:
		case VALUE_INT:
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_OBJECT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsShortType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_SHORT)
			return true;
		else
			return false;
	}

	public float convertPropertyValueToFloatType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_STRING:
			String valueString = getStringProperty(name);
			if ("NaN".equals(valueString) || "+Infinity".equals(valueString) || "-Infinity".equals(valueString) || "Infinity".equals(valueString)) {
				throw new NumberFormatException("JMS CTS requires conversion from java.lang.String" + "=( NaN, -Infinity, +Infinity, Infinity) to float should fail");
			}
			return Float.valueOf(valueString);
		case VALUE_OBJECT:
			Object object = UPMQMessage.deserializeObjectFromString(_headerBuilder.getPropertyMap().get(name).getValueObject());
			if (object instanceof Double)
				return (float) Double.parseDouble(object.toString());
			else
				return Float.valueOf(object.toString());
		case VALUE_SHORT:
		case VALUE_BYTE:
		case VALUE_LONG:
		case VALUE_INT:
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsFloatType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_FLOAT)
			return true;
		else
			return false;
	}

	public double convertPropertyValueToDoubleType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_FLOAT:
			return Double.valueOf(getFloatProperty(name));
		case VALUE_STRING:
			String valueString = getStringProperty(name);
			if ("NaN".equals(valueString) || "+Infinity".equals(valueString) || "-Infinity".equals(valueString) || "Infinity".equals(valueString)) {
				throw new NumberFormatException("JMS CTS requires conversion from java.lang.String" + "=( NaN, -Infinity, +Infinity, Infinity) to float should fail");
			}
			return Double.valueOf(valueString);
		case VALUE_OBJECT:
			Object object = UPMQMessage.deserializeObjectFromString(_headerBuilder.getPropertyMap().get(name).getValueObject());
			if (object instanceof Float)
				return Double.valueOf(Float.valueOf(object.toString()));
			else
				return Double.valueOf(object.toString());
		case VALUE_SHORT:
		case VALUE_BYTE:
		case VALUE_LONG:
		case VALUE_INT:
		case VALUE_BOOL:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsDoubleType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_DOUBLE)
			return true;
		else
			return false;
	}

	public String convertPropertyValueToStringType(String name) throws JMSException {
		switch (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase()) {
		case VALUE_BOOL:
			return String.valueOf(getBooleanProperty(name));
		case VALUE_BYTE:
			return String.valueOf(getByteProperty(name));
		case VALUE_DOUBLE:
			return String.valueOf(getDoubleProperty(name));
		case VALUE_FLOAT:
			return String.valueOf(getFloatProperty(name));
		case VALUE_INT:
			return String.valueOf(getIntProperty(name));
		case VALUE_LONG:
			return String.valueOf(getLongProperty(name));
		case VALUE_SHORT:
			return String.valueOf(getShortProperty(name));
		case VALUE_OBJECT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean propertyValueIsStringType(String name) {
		if (_headerBuilder.getPropertyMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_STRING)
			return true;
		else
			return false;
	}

	public static String serializeObjectToString(Object object) throws JMSException {
		try {
			ByteArrayOutputStream arrayOutputStream = new ByteArrayOutputStream();
			//GZIPOutputStream gzipOutputStream = new GZIPOutputStream(arrayOutputStream);
			ObjectOutputStream objectOutputStream = new ObjectOutputStream(arrayOutputStream);
			objectOutputStream.writeObject(object);
			objectOutputStream.flush();
			//return new String(Base64.getEncoder().encodeToString(arrayOutputStream.toByteArray()));
			return DatatypeConverter.printBase64Binary(arrayOutputStream.toByteArray());
		} catch (IOException e) {
			JMSException ex = new JMSException("object to string serialize error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	public static Object deserializeObjectFromString(String objectString) throws JMSException {
		try {
			//ByteArrayInputStream arrayInputStream = new ByteArrayInputStream(Base64.getDecoder().decode(objectString));
			ByteArrayInputStream arrayInputStream = new ByteArrayInputStream(DatatypeConverter.parseBase64Binary(objectString));
			//GZIPInputStream gzipInputStream = new GZIPInputStream(arrayInputStream);
			ObjectInputStream objectInputStream = new ObjectInputStream(arrayInputStream);
			return objectInputStream.readObject();
		} catch (ClassNotFoundException e) {
			JMSException ex = new JMSException("object from string serialize error");
			ex.setLinkedException(e);
			throw ex;
		} catch (IOException e) {
			JMSException ex = new JMSException("object from string serialize error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	public boolean isExpired() throws JMSException {
		long expireTime = getJMSExpiration();
		boolean expired = expireTime > 0 && System.currentTimeMillis() > expireTime;
		if (expired)
			this.acknowledge();
		return expired;
	}

	public String getMessageInfo() {

		String strAll = "";
		for (Map.Entry<FieldDescriptor, Object> entry : _headerBuilder.getAllFields().entrySet()) {
			String strPart = entry.getKey().getName() + " = " + entry.getValue() + " ";
			strAll += strPart;
		}
		return strAll;
	}

	@Override
	public String toString() {
		return "UPMQMessage {" + getMessageInfo() + "}";
	}

	@Override
	public URI getURI() {
		return _uri;
	}

	@Override
	public void setURI(URI uri) throws JMSException {
		if (!uri.getScheme().equals("file"))
			throw new UnsupportedOperationException("not supported yet");

		if (!Files.isRegularFile(Paths.get(uri)))
			new JMSException("not a file");

		_uri = uri;

		return;
	}

	@Override
	public void setInputStream(InputStream input) {
		// TODO Auto-generated method stub

	}

	@Override
	public OutputStream getOutputStream() {
		// TODO Auto-generated method stub
		return null;
	}
}
