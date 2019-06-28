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
 
package com.broker.libupmq.foreign;

import javax.jms.JMSException;
import javax.jms.MessageFormatException;

public class StreamMessageConverter {

	public static boolean getBoolean(final Object objectValue) throws JMSException {
		final boolean value;
		if (objectValue == null) {
			value = Boolean.valueOf(null).booleanValue();
		} else if (objectValue instanceof Boolean) {
			value = ((Boolean) objectValue).booleanValue();
		} else if (objectValue instanceof String) {
			value = Boolean.valueOf((String) objectValue).booleanValue();
		} else {
			throw new MessageFormatException("Only values set as a boolean or String may be retrieved as a boolean");
		}

		return value;
	}

	public static byte getByte(final Object objectValue) throws JMSException {
		final byte value;
		if (objectValue == null) {
			value = Byte.valueOf(null).byteValue();
		} else if (objectValue instanceof Byte) {
			value = ((Byte) objectValue).byteValue();
		} else if (objectValue instanceof String) {
			value = Byte.valueOf((String) objectValue).byteValue();
		} else {
			throw new MessageFormatException("Only values set as a byte or String may be retrieved as a byte");
		}

		return value;
	}

	public static short getShort(final Object objectValue) throws JMSException {
		final short value;
		if (objectValue == null) {
			value = Short.valueOf(null).shortValue();
		} else if (objectValue instanceof Byte) {
			value = ((Byte) objectValue).byteValue();
		} else if (objectValue instanceof Short) {
			value = ((Short) objectValue).shortValue();
		} else if (objectValue instanceof String) {
			value = Short.valueOf((String) objectValue).shortValue();
		} else {
			throw new MessageFormatException("Only values set as a byte, short or String may be retrieved as a short");
		}

		return value;
	}

	public static int getInt(final Object objectValue) throws JMSException {
		final int value;
		if (objectValue == null) {
			value = Integer.valueOf(null).intValue();
		} else if (objectValue instanceof Byte) {
			value = ((Byte) objectValue).byteValue();
		} else if (objectValue instanceof Short) {
			value = ((Short) objectValue).shortValue();
		} else if (objectValue instanceof Integer) {
			value = ((Integer) objectValue).intValue();
		} else if (objectValue instanceof String) {
			value = Integer.valueOf((String) objectValue).intValue();
		} else {
			throw new MessageFormatException("Only values set as a byte, short, int or String may be retrieved as a int");
		}

		return value;
	}

	public static long getLong(final Object objectValue) throws JMSException {
		final long value;
		if (objectValue == null) {
			value = Long.valueOf(null).longValue();
		} else if (objectValue instanceof Byte) {
			value = ((Byte) objectValue).byteValue();
		} else if (objectValue instanceof Short) {
			value = ((Short) objectValue).shortValue();
		} else if (objectValue instanceof Integer) {
			value = ((Integer) objectValue).intValue();
		} else if (objectValue instanceof Long) {
			value = ((Long) objectValue).longValue();
		} else if (objectValue instanceof String) {
			value = Long.valueOf((String) objectValue).longValue();
		} else {
			throw new MessageFormatException("Only values set as a byte, short, int, long or String may be retrieved as a long");
		}

		return value;
	}

	public static float getFloat(final Object objectValue) throws JMSException {
		final float value;
		if (objectValue == null) {
			value = Float.valueOf(null).floatValue();
		} else if (objectValue instanceof Float) {
			value = ((Float) objectValue).floatValue();
		} else if (objectValue instanceof String) {
			// NB:  I don't think this is true to the spec, but is required by JMS CTS
			if ("NaN".equals(objectValue) || "-Infinity".equals(objectValue) || "Infinity".equals(objectValue) || "+Infinity".equals(objectValue)) {
				throw new NumberFormatException("JMS CTS requires conversion from java.lang.String" + "=( NaN, -Infinity, +Infinity or Infinity ) to float should fail");
			}
			value = Float.valueOf((String) objectValue).floatValue();
		} else {
			throw new MessageFormatException("Only values set as a float or String may be retrieved as a float");
		}

		return value;
	}

	public static double getDouble(final Object objectValue) throws JMSException {
		final double value;
		if (objectValue == null) {
			value = Double.valueOf(null).doubleValue();
		} else if (objectValue instanceof Float) {
			value = ((Float) objectValue).floatValue();
		} else if (objectValue instanceof Double) {
			value = ((Double) objectValue).doubleValue();
		} else if (objectValue instanceof String) {
			// NB:  I don't think this is true to the spec, but is required by JMS CTS
			if ("NaN".equals(objectValue) || "-Infinity".equals(objectValue) || "Infinity".equals(objectValue) || "+Infinity".equals(objectValue)) {
				throw new NumberFormatException("JMS CTS requires conversion from java.lang.String" + "=( NaN, -Infinity, +Infinity or Infinity ) to float should fail");
			}
			value = Double.valueOf((String) objectValue).doubleValue();
		} else {
			throw new MessageFormatException("Only values set as a float, double or String may be retrieved as a double");
		}

		return value;
	}

	public static char getChar(final Object objectValue) throws JMSException {
		final char value;
		if (objectValue == null) {
			value = String.valueOf(null).charAt(0);
		} else if (objectValue instanceof Character) {
			value = ((Character) objectValue).charValue();
		} else {
			throw new MessageFormatException("Only values set as a char or String may be retrieved as a char");
		}

		return value;
	}

	public static String getString(final Object objectValue) throws JMSException {
		final String value;
		if (objectValue == null || objectValue instanceof String) {
			value = (String) objectValue;
		} else if (objectValue instanceof byte[]) {
			throw new MessageFormatException("Values set as byte[] cannot be retrieved as String");
		} else {
			value = String.valueOf(objectValue);
		}

		return value;
	}

	public static byte[] getBytes(final Object objectValue) throws JMSException {
		final byte[] value;
		if (objectValue == null) {
			value = null;
		} else if (objectValue instanceof byte[]) {
			value = (byte[]) objectValue;
		} else {
			throw new MessageFormatException("Only values set as a byte or String may be retrieved as a byte");
		}

		return value;
	}

	public static Object getObject(final Object value) {
		Object mutableValue = value;
		if (mutableValue instanceof byte[]) {
			// byte[]s are mutable so we must clone to prevent client from modifying the maps internal data
			mutableValue = ((byte[]) mutableValue).clone();
		}

		return mutableValue;
	}

	public static Object checkObject(final Object value, final boolean propertyMode) throws JMSException {

		final boolean validPropertyType = value == null || value instanceof Boolean || value instanceof Byte || value instanceof Short || value instanceof Integer || value instanceof Long
				|| value instanceof Float || value instanceof Double || value instanceof String;
		final boolean validMapMessageType = validPropertyType || value instanceof Character || value instanceof byte[];
		if (propertyMode && !validPropertyType || !propertyMode && !validMapMessageType) {
			throw new MessageFormatException("Attempting to call setObject() with an object of type other than" + " Boolean, Byte, Short, Integer, Long, Float, Double, Character, String, or byte[]");
		}

		Object immutableValue = value;
		if (value instanceof byte[]) {
			// byte[]s are mutable so we must clone to prevent client from modifying the maps internal data
			immutableValue = ((byte[]) value).clone();
		}

		return immutableValue;
	}

}
