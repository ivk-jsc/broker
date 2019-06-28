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

import java.util.Collections;
import java.util.Enumeration;

import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.MessageFormatException;

import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.protocol.Protocol;
import com.google.protobuf.ByteString;

public class UPMQMapMessage extends UPMQMessage implements MapMessage {

	public UPMQMapMessage(UPMQMessageConsumer consumer) throws JMSException {
		super(consumer);

		_headerBuilder.setBodyType(Protocol.Body.MAP_BODY_FIELD_NUMBER);
	}

	@Override
	public boolean getBoolean(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsBooleanType(name))
					return _bodyBuilder.getMapBody().getValueMap().get(name).getValueBool();
				else
					return convertMapValueToBooleanType(name);
			else
				return false;
		else
			return false;
	}

	@Override
	public byte getByte(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsByteType(name))
					return (byte) _bodyBuilder.getMapBody().getValueMap().get(name).getValueByte();
				else
					return convertMapValueToByteType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public short getShort(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsShortType(name))
					return (short) _bodyBuilder.getMapBody().getValueMap().get(name).getValueShort();
				else
					return convertMapValueToShortType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public int getInt(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsIntType(name))
					return _bodyBuilder.getMapBody().getValueMap().get(name).getValueInt();
				else
					return convertMapValueToIntType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public long getLong(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsLongType(name))
					return _bodyBuilder.getMapBody().getValueMap().get(name).getValueLong();
				else
					return convertMapValueToLongType(name);
			else
				throw new NumberFormatException();
		else
			throw new NumberFormatException();
	}

	@Override
	public float getFloat(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsFloatType(name))
					return _bodyBuilder.getMapBody().getValueMap().get(name).getValueFloat();
				else
					return convertMapValueToFloatType(name);
			else
				throw new NullPointerException();
		else
			throw new NullPointerException();
	}

	@Override
	public double getDouble(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsDoubleType(name))
					return _bodyBuilder.getMapBody().getValueMap().get(name).getValueDouble();
				else
					return convertMapValueToDoubleType(name);
			else
				throw new NullPointerException();
		else
			throw new NullPointerException();
	}

	@Override
	public String getString(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsStringType(name))
					return _bodyBuilder.getMapBody().getValueMap().get(name).getValueString();
				else
					return convertMapValueToStringType(name);
			else
				return null;
		else
			return null;
	}

	@Override
	public Object getObject(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsObjectType(name))
					return UPMQMessage.deserializeObjectFromString(_bodyBuilder.getMapBody().getValueMap().get(name).getValueObject());
				else
					return convertMapValueToObjectType(name);
			else
				return null;
		else
			return null;
	}

	@Override
	public char getChar(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsCharType(name))
					return (char) _bodyBuilder.getMapBody().getValueMap().get(name).getValueChar();
				else
					return convertMapValueToCharType(name);
			else
				throw new NullPointerException();
		else
			throw new NullPointerException();
	}

	@Override
	public byte[] getBytes(String name) throws JMSException {
		checkMapName(name);
		if (itemExists(name))
			if (mapValueNotNull(name))
				if (mapValueIsBytesType(name))
					return _bodyBuilder.getMapBody().getValueMap().get(name).getValueBytes().toByteArray();
				else
					return convertMapValueToBytesType(name);
			else
				return null;
		else
			return null;
	}

	@Override
	@SuppressWarnings("rawtypes")
	public Enumeration getMapNames() throws JMSException {
		return Collections.enumeration(_bodyBuilder.getMapBody().getValueMap().keySet());
	}

	@Override
	public void setBoolean(String name, boolean value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueBool(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setByte(String name, byte value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueByte(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setShort(String name, short value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueShort(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setInt(String name, int value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueInt(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setLong(String name, long value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueLong(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setFloat(String name, float value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueFloat(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setDouble(String name, double value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueDouble(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setString(String name, String value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(value == null ? true : false);
			propertyb.setValueString(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setObject(String name, Object value) throws JMSException {
		checkWrite();
		checkMapName(name);

		if (value instanceof Boolean | value instanceof Byte | value instanceof Short | value instanceof Integer | value instanceof Long | value instanceof Float | value instanceof Double
				| value instanceof String | value instanceof Character | value instanceof byte[]) {

		} else if (value == null) {

		} else {
			throw new MessageFormatException("invalid object type (should be Boolean, Byte, Short, Integer, Long, Float, Double, String, Character, byte[])");
		}

		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(value == null ? true : false);
			propertyb.setValueObject(UPMQMessage.serializeObjectToString(value));
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setChar(String name, char value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueChar(value);
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setBytes(String name, byte[] value) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueBytes(ByteString.copyFrom(value));
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void setBytes(String name, byte[] value, int offset, int length) throws JMSException {
		checkWrite();
		checkMapName(name);
		try {
			Protocol.Property.Builder propertyb = Protocol.Property.newBuilder();
			propertyb.setIsNull(false);
			propertyb.setValueBytes(ByteString.copyFrom(value, offset, length));
			_bodyBuilder.getMapBodyBuilder().putValue(name, propertyb.build());
		} catch (Exception e) {
			JMSException ex = new JMSException("set error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public boolean itemExists(String name) throws JMSException {
		return _bodyBuilder.getMapBody().getValueMap().containsKey(name);
	}

	public void checkMapName(String name) throws JMSException {
		if (name == null || name == "") {
			throw new IllegalArgumentException("invalid map name (null or empty)");
		}
		for (int i = 0; i < name.length(); i++) {
			if (i == 0 && !Character.isJavaIdentifierStart(name.charAt(i))) {
				throw new IllegalArgumentException("invalid map name");
			}
			if (i != 0 && !Character.isJavaIdentifierPart(name.charAt(i))) {
				throw new IllegalArgumentException("invalid map name");
			}
		}
		/*
		for (String reservedKeyword : MessageImpl.reservedKeywords) {
			if (name.equalsIgnoreCase(reservedKeyword)) {
				throw new JMSException("invalid property name (reserved keyword)");
			}
		}
		for (String reservedProperty : MessageImpl.reservedProperties) {
			if (name.equalsIgnoreCase(reservedProperty)) {
				throw new JMSException("invalid property name (reserved keyword)");
			}
		}
		*/
	}

	public boolean mapValueNotNull(String name) throws JMSException {
		return !_bodyBuilder.getMapBody().getValueMap().get(name).getIsNull();
	}

	public Object convertMapValueToObjectType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_BOOL:
			return Boolean.valueOf(getBoolean(name));
		case VALUE_BYTE:
			return Byte.valueOf(getByte(name));
		case VALUE_DOUBLE:
			return Double.valueOf(getDouble(name));
		case VALUE_FLOAT:
			return Float.valueOf(getFloat(name));
		case VALUE_INT:
			return Integer.valueOf(getInt(name));
		case VALUE_LONG:
			return Long.valueOf(getLong(name));
		case VALUE_SHORT:
			return Short.valueOf(getShort(name));
		case VALUE_STRING:
			return String.valueOf(getString(name));
		case VALUE_OBJECT:
			return UPMQMessage.deserializeObjectFromString(_bodyBuilder.getMapBody().getValueMap().get(name).getValueObject());
		case VALUE_BYTES:
			return getBytes(name);
		case VALUE_CHAR:
			return getChar(name);
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsObjectType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_OBJECT)
			return true;
		else
			return false;
	}

	public boolean convertMapValueToBooleanType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_STRING:
			return Boolean.valueOf(getString(name));
		case VALUE_OBJECT:
			return (Boolean) getObject(name);
		case VALUE_BYTE:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_INT:
		case VALUE_LONG:
		case VALUE_SHORT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsBooleanType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_BOOL)
			return true;
		else
			return false;
	}

	public int convertMapValueToIntType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_BYTE:
			return Integer.valueOf(getByte(name));
		case VALUE_SHORT:
			return Integer.valueOf(getShort(name));
		case VALUE_STRING:
			return Integer.valueOf(getString(name));
		case VALUE_OBJECT:
			return (Integer) getObject(name);
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_LONG:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsIntType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_INT)
			return true;
		else
			return false;
	}

	public long convertMapValueToLongType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_BYTE:
			return Long.valueOf(getByte(name));
		case VALUE_SHORT:
			return Long.valueOf(getShort(name));
		case VALUE_INT:
			return Long.valueOf(getInt(name));
		case VALUE_STRING:
			return Long.valueOf(getString(name));
		case VALUE_OBJECT:
			return (Long) getObject(name);
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsLongType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_LONG)
			return true;
		else
			return false;
	}

	public byte convertMapValueToByteType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_STRING:
			return Byte.valueOf(getString(name));
		case VALUE_OBJECT:
			return (Byte) getObject(name);
		case VALUE_INT:
		case VALUE_LONG:
		case VALUE_SHORT:
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsByteType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_BYTE)
			return true;
		else
			return false;
	}

	public short convertMapValueToShortType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_BYTE:
			return Short.valueOf(getByte(name));
		case VALUE_STRING:
			return Short.valueOf(getString(name));
		case VALUE_OBJECT:
			return (Short) getObject(name);
		case VALUE_LONG:
		case VALUE_INT:
		case VALUE_BOOL:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_BYTES:
		case VALUE_CHAR:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsShortType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_SHORT)
			return true;
		else
			return false;
	}

	public float convertMapValueToFloatType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_STRING:
			String valueString = getString(name);
			if ("NaN".equals(valueString) || "+Infinity".equals(valueString) || "-Infinity".equals(valueString) || "Infinity".equals(valueString)) {
				throw new NumberFormatException("JMS CTS requires conversion from java.lang.String" + "=( NaN, -Infinity, +Infinity, Infinity) to float should fail");
			}
			return Float.valueOf(valueString);
		case VALUE_OBJECT:
			return Float.parseFloat(String.valueOf(UPMQMessage.deserializeObjectFromString(_bodyBuilder.getMapBody().getValueMap().get(name).getValueObject())));
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

	public boolean mapValueIsFloatType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_FLOAT)
			return true;
		else
			return false;
	}

	public double convertMapValueToDoubleType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_FLOAT:
			return Double.valueOf(getFloat(name));
		case VALUE_STRING:
			String valueString = getString(name);
			if ("NaN".equals(valueString) || "+Infinity".equals(valueString) || "-Infinity".equals(valueString) || "Infinity".equals(valueString)) {
				throw new NumberFormatException("JMS CTS requires conversion from java.lang.String" + "=( NaN, -Infinity, +Infinity, Infinity) to float should fail");
			}
			return Double.valueOf(valueString);
		case VALUE_OBJECT:
			return Double.parseDouble(String.valueOf(UPMQMessage.deserializeObjectFromString(_bodyBuilder.getMapBody().getValueMap().get(name).getValueObject())));
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

	public boolean mapValueIsDoubleType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_DOUBLE)
			return true;
		else
			return false;
	}

	public String convertMapValueToStringType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_BOOL:
			return String.valueOf(getBoolean(name));
		case VALUE_BYTE:
			return String.valueOf(getByte(name));
		case VALUE_DOUBLE:
			return String.valueOf(getDouble(name));
		case VALUE_FLOAT:
			return String.valueOf(getFloat(name));
		case VALUE_INT:
			return String.valueOf(getInt(name));
		case VALUE_LONG:
			return String.valueOf(getLong(name));
		case VALUE_SHORT:
			return String.valueOf(getShort(name));
		case VALUE_CHAR:
			return String.valueOf(getChar(name));
		case VALUE_OBJECT:
			return String.valueOf(getObject(name));
		case VALUE_BYTES:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsStringType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_STRING)
			return true;
		else
			return false;
	}

	public char convertMapValueToCharType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_OBJECT:
			return (Character) getObject(name);
		case VALUE_BOOL:
		case VALUE_BYTE:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_INT:
		case VALUE_LONG:
		case VALUE_SHORT:
		case VALUE_CHAR:
		case VALUE_BYTES:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsCharType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_CHAR)
			return true;
		else
			return false;
	}

	public byte[] convertMapValueToBytesType(String name) throws JMSException {

		switch (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase()) {
		case VALUE_OBJECT:
			return (byte[]) UPMQMessage.deserializeObjectFromString(_bodyBuilder.getMapBody().getValueMap().get(name).getValueObject());
		case VALUE_BOOL:
		case VALUE_BYTE:
		case VALUE_DOUBLE:
		case VALUE_FLOAT:
		case VALUE_INT:
		case VALUE_LONG:
		case VALUE_SHORT:
		case VALUE_CHAR:
		case VALUE_BYTES:
		case PROPERTYVALUE_NOT_SET:
		default:
			throw new MessageFormatException("no possible conversion");
		}
	}

	public boolean mapValueIsBytesType(String name) {
		if (_bodyBuilder.getMapBody().getValueMap().get(name).getPropertyValueCase() == Protocol.Property.PropertyValueCase.VALUE_BYTES)
			return true;
		else
			return false;
	}

	@Override
	public String toString() {
		return "UPMQMapMessage {" + getMessageInfo() + "}";
	}
}
