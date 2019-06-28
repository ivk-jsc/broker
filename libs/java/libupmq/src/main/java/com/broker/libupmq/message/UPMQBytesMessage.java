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
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.IOException;

import javax.jms.BytesMessage;
import javax.jms.JMSException;
import javax.jms.MessageEOFException;
import javax.jms.MessageFormatException;
import javax.jms.MessageNotReadableException;
import javax.jms.MessageNotWriteableException;

import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.protocol.Protocol;
import com.google.protobuf.ByteString;

public class UPMQBytesMessage extends UPMQMessage implements BytesMessage {

	private DataInputStream dataIn = null;
	private DataOutputStream dataOut = null;

	private ByteArrayOutputStream bytesOut = null;

	public UPMQBytesMessage(UPMQMessageConsumer consumer) throws JMSException {
		super(consumer);

		_headerBuilder.setBodyType(Protocol.Body.BYTES_BODY_FIELD_NUMBER);

		if (dataOut == null) {
			bytesOut = new ByteArrayOutputStream();
			dataOut = new DataOutputStream(bytesOut);
		}
	}

	@Override
	public void reset() throws JMSException {

		if (!_bodyReadOnly)
			_bodyReadOnly = true;

		try {
			if (dataOut != null) {
				dataOut.close();
				dataOut = null;
				bytesOut.close();
				if (!_bodyBuilder.getBytesBody().hasValue())
					_bodyBuilder.getBytesBodyBuilder().setValue(ByteString.copyFrom(bytesOut.toByteArray()));
				bytesOut = null;
			}
			if (dataIn != null) {
				dataIn.close();
				dataIn = null;
			}
		} catch (IOException e) {
			throw new MessageEOFException(e.getMessage());
		}

		dataIn = new DataInputStream(new ByteArrayInputStream(_bodyBuilder.getBytesBody().getValue().toByteArray()));
	}

	public void close() throws JMSException {

		if (dataOut != null)
			_bodyBuilder.getBytesBodyBuilder().setValue(ByteString.copyFrom(bytesOut.toByteArray()));
	}

	private void resetRead() {
		try {
			dataIn.reset();
		} catch (IOException e) {

		}
	}

	@Override
	public void clearBody() throws JMSException {
		super.clearBody();

		try {
			if (dataOut != null) {
				dataOut.close();
				dataOut = null;

				bytesOut.close();
				//FIXME
				//_bodyBuilder.getBytesBodyBuilder().setValue(ByteString.copyFrom(bytesOut.toByteArray()));
				bytesOut = null;
			}
			if (dataIn != null) {
				dataIn.close();
				dataIn = null;
			}
		} catch (IOException e) {
			throw new MessageEOFException(e.getMessage());
		}

		checkWrite();
	}

	@Override
	public long getBodyLength() throws JMSException {
		return _bodyBuilder.getBytesBody().getValue().toByteArray().length;
	}

	@Override
	public boolean readBoolean() throws JMSException {
		checkRead();
		try {
			return dataIn.readBoolean();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public byte readByte() throws JMSException {
		checkRead();
		try {
			return dataIn.readByte();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public int readUnsignedByte() throws JMSException {
		checkRead();
		try {
			return dataIn.readUnsignedByte();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public short readShort() throws JMSException {
		checkRead();
		try {
			return dataIn.readShort();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public int readUnsignedShort() throws JMSException {
		checkRead();
		try {
			return dataIn.readUnsignedShort();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public char readChar() throws JMSException {
		checkRead();
		try {
			return dataIn.readChar();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public int readInt() throws JMSException {
		checkRead();
		try {
			return dataIn.readInt();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public long readLong() throws JMSException {
		checkRead();
		try {
			return dataIn.readLong();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public float readFloat() throws JMSException {
		checkRead();
		try {
			return dataIn.readFloat();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public double readDouble() throws JMSException {
		checkRead();
		try {
			return dataIn.readDouble();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public String readUTF() throws JMSException {
		checkRead();
		try {
			return dataIn.readUTF();
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			throw new MessageFormatException(e.getMessage());
		}
	}

	@Override
	public int readBytes(byte[] value) throws JMSException {
		checkRead();
		try {
			return dataIn.read(value);
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public int readBytes(byte[] value, int length) throws JMSException {
		checkRead();
		try {
			return dataIn.read(value, 0, length);
		} catch (EOFException e) {
			throw new MessageEOFException(e.getMessage());
		} catch (IOException e) {
			resetRead();
			JMSException ex = new JMSException("read error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeBoolean(boolean value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeBoolean(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeByte(byte value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeByte(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeShort(short value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeShort(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeChar(char value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeChar(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeInt(int value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeInt(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeLong(long value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeLong(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeFloat(float value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeFloat(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeDouble(double value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeDouble(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeUTF(String value) throws JMSException {
		checkWrite();
		try {
			dataOut.writeUTF(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeBytes(byte[] value) throws JMSException {
		checkWrite();
		try {
			dataOut.write(value);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeBytes(byte[] value, int offset, int length) throws JMSException {
		checkWrite();
		try {
			dataOut.write(value, offset, length);
		} catch (IOException e) {
			JMSException ex = new JMSException("write error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	@Override
	public void writeObject(Object value) throws JMSException {
		checkWrite();

		if (value instanceof Boolean) {
			writeBoolean(((Boolean) value).booleanValue());
		} else if (value instanceof Byte) {
			writeByte(((Byte) value).byteValue());
		} else if (value instanceof Short) {
			writeShort(((Short) value).shortValue());
		} else if (value instanceof Character) {
			writeChar(((Character) value).charValue());
		} else if (value instanceof Integer) {
			writeInt(((Integer) value).intValue());
		} else if (value instanceof Long) {
			writeLong(((Long) value).longValue());
		} else if (value instanceof Float) {
			writeFloat(((Float) value).floatValue());
		} else if (value instanceof Double) {
			writeDouble(((Double) value).doubleValue());
		} else if (value instanceof String) {
			writeUTF((String) value);
		} else if (value instanceof byte[]) {
			writeBytes((byte[]) value);
		} else if (value == null) {
			throw new NullPointerException("bytesMessage does not support null");
		} else {
			throw new MessageFormatException("Cannot write objects of type=" + value.getClass().getName());
		}
	}

	@Override
	protected void checkWrite() throws MessageNotWriteableException {
		super.checkWrite();

		if (dataOut == null) {
			bytesOut = new ByteArrayOutputStream();
			dataOut = new DataOutputStream(bytesOut);
		}
	}

	@Override
	protected void checkRead() throws MessageNotReadableException {
		super.checkRead();
	}

	@Override
	public String toString() {
		return "UPMQBytesMessage {" + getMessageInfo() + "}";
	}
}
