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
import java.io.EOFException;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

import javax.jms.JMSException;
import javax.jms.MessageEOFException;
import javax.jms.MessageFormatException;
import javax.jms.MessageNotReadableException;
import javax.jms.MessageNotWriteableException;
import javax.jms.StreamMessage;

import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.libupmq.foreign.StreamMessageConverter;
import com.broker.protocol.Protocol;
import com.google.protobuf.ByteString;

public class UPMQStreamMessage extends UPMQMessage implements StreamMessage {

	private ObjectInputStream objectIn = null;
	private ObjectOutputStream objectOut = null;

	private ByteArrayOutputStream bytesOut = null;

	private Object resetValue = null;
	private boolean resetValueIsSet = false;
	private int byteArrayBytesRead = -1;

	public UPMQStreamMessage(UPMQMessageConsumer consumer) throws JMSException {
		super(consumer);

		_headerBuilder.setBodyType(Protocol.Body.STREAM_BODY_FIELD_NUMBER);

		try {
			ByteArrayOutputStream magicBytesStream = new ByteArrayOutputStream();
			ObjectOutputStream magicObjectOut = new ObjectOutputStream(magicBytesStream);
			byte[] magicBytes = magicBytesStream.toByteArray();
			magicObjectOut.close();
			magicBytesStream.close();
			_bodyBuilder.getStreamBodyBuilder().setValue(ByteString.copyFrom(magicBytes));
		} catch (IOException e) {
			JMSException ex = new JMSException("init stream message error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public boolean readBoolean() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getBoolean(value);
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public byte readByte() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getByte(value);
		} catch (NumberFormatException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public short readShort() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getShort(value);
		} catch (NumberFormatException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public char readChar() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getChar(value);
		} catch (NullPointerException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public int readInt() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getInt(value);
		} catch (NumberFormatException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public long readLong() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getLong(value);
		} catch (NumberFormatException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public float readFloat() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getFloat(value);
		} catch (NumberFormatException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (NullPointerException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public double readDouble() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getDouble(value);
		} catch (NumberFormatException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (NullPointerException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public String readString() throws JMSException {
		checkRead();
		final Object value = readInnerObject(false);
		try {
			return StreamMessageConverter.getString(value);
		} catch (JMSException e) {
			resetValue = value;
			resetValueIsSet = true;
			throw e;
		}
	}

	@Override
	public int readBytes(byte[] value) throws JMSException {
		checkRead();
		final Object objectValue = readInnerObject(true);
		final byte[] valueRead;
		try {
			valueRead = StreamMessageConverter.getBytes(objectValue);
		} catch (JMSException e) {
			resetValue = objectValue;
			resetValueIsSet = true;
			throw e;
		}

		final int read;
		int endByte = 0;
		if (valueRead == null) {
			read = -1;
		} else {
			final int startByte = byteArrayBytesRead == -1 ? 0 : byteArrayBytesRead;
			final int available = valueRead.length - startByte;
			if (available == 0) {
				read = -1;
			} else {
				read = Math.min(available, value.length);
				endByte = startByte + read;
				System.arraycopy(valueRead, startByte, value, 0, endByte - startByte);
			}
		}

		if (read < value.length) {
			byteArrayBytesRead = -1;
			resetValue = null;
			resetValueIsSet = false;
		} else {
			byteArrayBytesRead = endByte;
			resetValue = objectValue;
			resetValueIsSet = true;
		}

		return read;
	}

	@Override
	public Object readObject() throws JMSException {
		checkRead();
		return readInnerObject(false);
	}

	@Override
	public void writeBoolean(boolean value) throws JMSException {
		checkWrite();
		writeObject(value ? Boolean.TRUE : Boolean.FALSE);
	}

	@Override
	public void writeByte(byte value) throws JMSException {
		checkWrite();
		writeObject(new Byte(value));
	}

	@Override
	public void writeShort(short value) throws JMSException {
		checkWrite();
		writeObject(new Short(value));
	}

	@Override
	public void writeChar(char value) throws JMSException {
		checkWrite();
		writeObject(new Character(value));
	}

	@Override
	public void writeInt(int value) throws JMSException {
		checkWrite();
		writeObject(new Integer(value));
	}

	@Override
	public void writeLong(long value) throws JMSException {
		checkWrite();
		writeObject(new Long(value));
	}

	@Override
	public void writeFloat(float value) throws JMSException {
		checkWrite();
		writeObject(new Float(value));
	}

	@Override
	public void writeDouble(double value) throws JMSException {
		checkWrite();
		writeObject(new Double(value));
	}

	@Override
	public void writeString(String value) throws JMSException {
		checkWrite();
		writeObject(value);
	}

	@Override
	public void writeBytes(byte[] value) throws JMSException {
		checkWrite();
		writeObject(value);
	}

	@Override
	public void writeBytes(byte[] value, int offset, int length) throws JMSException {
		checkWrite();
		try {
			objectOut.writeInt(length);
			objectOut.write(value, offset, length);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeObject(Object value) throws JMSException {
		checkWrite();
		try {
			if (value instanceof byte[]) {
				objectOut.writeInt(((byte[]) value).length);
				objectOut.write((byte[]) StreamMessageConverter.checkObject(value, false));
			} else {
				objectOut.writeInt(-1);
				objectOut.writeObject(StreamMessageConverter.checkObject(value, false));
			}
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void reset() throws JMSException {

		if (!_bodyReadOnly)
			_bodyReadOnly = true;

		try {
			if (objectOut != null) {
				objectOut.close();
				objectOut = null;
				bytesOut.close();
				_bodyBuilder.getStreamBodyBuilder().setValue(ByteString.copyFrom(bytesOut.toByteArray()));
				bytesOut = null;
			}
			if (objectIn != null) {
				objectIn.close();
				objectIn = null;
			}
		} catch (IOException e) {
			JMSException ex = new JMSException("reset error");
			ex.setLinkedException(e);
			throw ex;
		}

		resetValue = null;
		resetValueIsSet = false;
		byteArrayBytesRead = -1;

		try {
			objectIn = new ObjectInputStream(new ByteArrayInputStream(_bodyBuilder.getStreamBody().getValue().toByteArray()));
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			JMSException ex = new JMSException("reset error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	public void close() throws JMSException {

		if (objectOut != null)
			_bodyBuilder.getStreamBodyBuilder().setValue(ByteString.copyFrom(bytesOut.toByteArray()));
	}

	@Override
	public void clearBody() throws JMSException {
		super.clearBody();

		try {
			if (objectOut != null) {
				objectOut.close();
				objectOut = null;

				bytesOut.close();
				_bodyBuilder.getStreamBodyBuilder().setValue(ByteString.copyFrom(bytesOut.toByteArray()));
				bytesOut = null;
			}
			if (objectIn != null) {
				objectIn.close();
				objectIn = null;
			}
		} catch (IOException e) {
			throw new MessageEOFException(e.getMessage());
		}

		checkWrite();
	}

	private Object readInnerObject(final boolean partialOkay) throws JMSException {

		if (objectIn == null) {
			throw new MessageNotReadableException("Message is currently in write-only mode");
		}

		try {
			final Object value;
			if (resetValueIsSet) {
				if (!partialOkay && byteArrayBytesRead != -1) {
					throw new MessageFormatException("Attempting to read another data type after an incomplete read of a byte array");
				}

				value = resetValue;
				resetValue = null;
				resetValueIsSet = false;
			} else {
				final int length = objectIn.readInt();
				if (length == -1) {
					value = objectIn.readObject();
				} else {
					final byte[] bytesValue = new byte[length];
					objectIn.readFully(bytesValue);
					value = bytesValue;
				}
			}
			return value;
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		} catch (ClassNotFoundException e) {
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	protected void checkWrite() throws MessageNotWriteableException {
		super.checkWrite();

		if (objectOut == null) {
			bytesOut = new ByteArrayOutputStream();
			try {
				objectOut = new ObjectOutputStream(bytesOut);
			} catch (IOException e) {

			}
		}
	}

	@Override
	protected void checkRead() throws MessageNotReadableException {
		super.checkRead();
	}

	@Override
	public String toString() {
		return "UPMQStreamMessage {" + getMessageInfo() + "}";
	}
}
