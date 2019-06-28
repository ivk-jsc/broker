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
 
package com.broker.libupmq.transport;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import com.broker.protocol.Protocol;
import com.broker.protocol.Protocol.ProtoMessage.ProtoMessageTypeCase;

/**
 * 
 * 
 */
public final class UPMQWireFormat implements WireFormat {

	private boolean sizePrefixDisabled = false;
	private long maxFrameSize = Long.MAX_VALUE;
	private int MAX_BUFFER_SIZE = 1024 * 1024;
	private ByteBuffer sizeBuffer = ByteBuffer.allocate(4 + 8);

	public UPMQWireFormat() {

	}

	//	@Override
	//	public void marshal(Object o, DataOutput dataOut) throws IOException {
	//
	//		UPMQCommand command = (UPMQCommand) o;
	//		command.setMarshall();
	//
	//		ByteBuffer byteBuffer = ByteBuffer.allocate(command._bodyArraySize > 0 ? command._headerArraySize + command._bodyArraySize : command._headerArraySize);
	//		byteBuffer.clear();
	//		byteBuffer.put(command._headerArray);
	//		if (command._bodyArraySize > 0)
	//			byteBuffer.put(command._bodyArray);
	//		byteBuffer.rewind();
	//
	//		ByteBuffer sizeBuffer = ByteBuffer.allocate(4 * 2);
	//		sizeBuffer.clear();
	//		sizeBuffer.order(ByteOrder.LITTLE_ENDIAN);
	//		sizeBuffer.putInt(command._headerArraySize);
	//		if (command._bodyArraySize > 0)
	//			sizeBuffer.putInt(command._bodyArraySize);
	//		else
	//			sizeBuffer.putInt(0);
	//		sizeBuffer.rewind();
	//
	//		dataOut.write(sizeBuffer.array());
	//		dataOut.write(byteBuffer.array());
	//	}

	@Override
	public void marshal(Object o, DataOutput dataOut) throws IOException {

		UPMQCommand command = (UPMQCommand) o;
		command.setMarshall();

		int bodySize = 0;

		if (command._bodyBuilder != null)
			bodySize = command._bodyBuilder.build().getSerializedSize();

		sizeBuffer.clear();
		sizeBuffer.order(ByteOrder.LITTLE_ENDIAN);
		sizeBuffer.putInt(command._headerBuilder.build().getSerializedSize());
		if (bodySize > 0)
			sizeBuffer.putInt(bodySize);
		else if (bodySize == 0 && command._uri != null) {
			File file = new File(command._uri.getPath());
			sizeBuffer.putLong(file.length());
		} else
			sizeBuffer.putLong(0);
		sizeBuffer.rewind();

		dataOut.write(sizeBuffer.array());
		dataOut.write(command._headerBuilder.build().toByteArray());
		if (bodySize > 0)
			dataOut.write(command._bodyBuilder.build().toByteArray());
		else if (bodySize == 0 && command._uri != null) {
			InputStream stream = command._uri.toURL().openStream();
			int n = 0;
			byte[] buffer = new byte[MAX_BUFFER_SIZE];
			while ((n = stream.read(buffer)) > -1)
				dataOut.write(buffer, 0, n);
			stream.close();
		}
	}

	@Override
	public Object unmarshal(DataInput in) throws IOException {

		UPMQCommand command = new UPMQCommand();

		ByteBuffer sizeBuffer = ByteBuffer.allocate(12);
		sizeBuffer.order(ByteOrder.LITTLE_ENDIAN);

		in.readFully(sizeBuffer.array(), 0, 4 + 8); //int + long

		command._headerArraySize = sizeBuffer.getInt();
		command._bodyArraySize = sizeBuffer.getLong();

		if (command._headerArraySize == 0) {
			throw new IOException("Frame header size is 0");
		}
		if (command._headerArraySize + command._bodyArraySize > maxFrameSize) {
			//TODO ?
			//throw new IOException("Frame size of " + ((command._headerArraySize + command._bodyArraySize) / (1024 * 1024)) + " MB larger than max allowed " + (maxFrameSize / (1024 * 1024)) + " MB");
		}

		command._headerArray = new byte[command._headerArraySize];
		in.readFully(command._headerArray, 0, command._headerArraySize);

		command.Unmarshall();

		if (command._headerMessage.getProtoMessageTypeCase() == ProtoMessageTypeCase.MESSAGE && !command._headerMessage.getMessage().hasBodyType() && command._bodyArraySize > 0) {
			File file = File.createTempFile("upmq-large-message-" + command._headerMessage.getMessage().getMessageId().substring(3) + "-", ".tmp");
			OutputStream stream = new FileOutputStream(file);

			byte[] buffer = new byte[MAX_BUFFER_SIZE];
			for (long i = 0; i < command._bodyArraySize / MAX_BUFFER_SIZE; i++) {
				in.readFully(buffer);
				stream.write(buffer);
			}
			int restlen = (int) (command._bodyArraySize % MAX_BUFFER_SIZE);
			if (restlen > 0) {
				in.readFully(buffer, 0, restlen);
				stream.write(buffer, 0, restlen);
			}
			stream.close();
			command._uri = file.toURI();
		} else {
			if (command._bodyArraySize != 0) {
				command._bodyArray = new byte[(int) command._bodyArraySize];
				in.readFully(command._bodyArray, 0, (int) command._bodyArraySize);
				command._bodyMessage = Protocol.Body.parseFrom(command._bodyArray);
			}
		}

		return command;
	}

	@Override
	public void setVersion(int version) {
		// TODO Auto-generated method stub

	}

	@Override
	public int getVersion() {
		// TODO Auto-generated method stub
		return 0;
	}
}
