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

import java.io.IOException;
import java.net.URI;

import javax.jms.InvalidSelectorException;
import javax.jms.JMSException;

import com.broker.protocol.Protocol;
import com.broker.protocol.Protocol.Connected;
import com.broker.protocol.Protocol.ProtoMessage.ProtoMessageTypeCase;
import com.broker.protocol.Protocol.Receipt;
import com.google.protobuf.InvalidProtocolBufferException;

/**
 * 
 */
public class UPMQCommand extends Response {

	public byte[] _headerArray = null;
	public byte[] _bodyArray = null;

	public int _headerArraySize = 0;
	public long _bodyArraySize = 0;

	public Protocol.ProtoMessage.Builder _headerBuilder = null;
	public Protocol.Body.Builder _bodyBuilder = null;
	public URI _uri;

	public Protocol.ProtoMessage _headerMessage = null;
	public Protocol.Body _bodyMessage = null;

	public UPMQCommand() {

	}

	/*
	public UPMQCommand(byte[] header, byte[] data) {
	
		_header = header;
		_headerSize = _header.length;
	
		if (data != null) {
			_data = data;
			_dataSize = _data.length;
		}
	}
	*/

	public UPMQCommand(Protocol.ProtoMessage.Builder headerb, Protocol.Body.Builder bodyb, URI uri) {
		_headerBuilder = headerb;
		_bodyBuilder = bodyb;
		_uri = uri;
	}

	public Protocol.ProtoMessage processCommand() throws JMSException {

		try {
			Protocol.ProtoMessage protoMessage = Protocol.ProtoMessage.parseFrom(this._headerArray);
			if (protoMessage.getProtoMessageTypeCase() == ProtoMessageTypeCase.ERROR) {
				String errorMessage = "ERROR CODE: " + String.valueOf(protoMessage.getError().getErrorCode()) + System.getProperty("line.separator") + "ERROR MESSAGE: "
						+ protoMessage.getError().getErrorMessage();
				switch (protoMessage.getError().getErrorCode()) {
				case ERROR_INVALID_SELECTOR:
					throw new InvalidSelectorException(errorMessage);
				default:
					throw new JMSException(errorMessage);
				}
			}
			return protoMessage;
		} catch (InvalidProtocolBufferException e) {
			JMSException ex = new JMSException("Command parse error");
			ex.setLinkedException(e);
			throw ex;
		}
	}

	public Connected processConnected() throws JMSException {

		Protocol.ProtoMessage protoMessage = processCommand();
		if (protoMessage.getProtoMessageTypeCase() != ProtoMessageTypeCase.CONNECTED)
			throw new JMSException("Connected command type error");
		return protoMessage.getConnected();
	}

	public Receipt processReceipt() throws JMSException {

		Protocol.ProtoMessage protoMessage = processCommand();
		if (protoMessage.getProtoMessageTypeCase() != ProtoMessageTypeCase.RECEIPT)
			throw new JMSException("Connected command type error");
		return protoMessage.getReceipt();
	}

	public long processBrowserInfo() throws JMSException {

		Protocol.ProtoMessage protoMessage = processCommand();
		if (protoMessage.getProtoMessageTypeCase() != ProtoMessageTypeCase.BROWSER_INFO)
			throw new JMSException("Connected command type error");
		return protoMessage.getBrowserInfo().getMessageCount();
	}

	public String getObjectId() {
		return _headerMessage.getObjectId();
	}

	public void setMarshall() throws IOException {

		_headerBuilder.setRequestReplyId(this.getCommandId());
	}

	public void Unmarshall() throws IOException {

		_headerMessage = Protocol.ProtoMessage.parseFrom(_headerArray);
		if (_headerMessage.getRequestReplyId() == 0) {
			this.setResponse(false);
		} else {
			this.setResponse(true);
			this.setCorrelationId(_headerMessage.getRequestReplyId());
		}

		//4bas
//				if (_headerMessage.getProtoMessageTypeCase() == ProtoMessageTypeCase.MESSAGE) {
//					System.out.println("      tcp income: " + _headerMessage.getMessage().getMessageId());
//				}
	}
}
