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

import javax.jms.JMSException;
import javax.jms.TextMessage;

import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.protocol.Protocol;

public class UPMQTextMessage extends UPMQMessage implements TextMessage {

	public UPMQTextMessage(UPMQMessageConsumer consumer) throws JMSException {
		super(consumer);

		_headerBuilder.setBodyType(Protocol.Body.TEXT_BODY_FIELD_NUMBER);
	}

	@Override
	public void setText(String string) throws JMSException {
		checkWrite();

		if (string == null)
			_bodyBuilder.clearTextBody();
		else
			_bodyBuilder.getTextBodyBuilder().setValue(string);
	}

	@Override
	public String getText() throws JMSException {
		if (_bodyBuilder.getBodyTypeCase() != Protocol.Body.BodyTypeCase.TEXT_BODY)
			return null;
		else
			return super._bodyBuilder.getTextBody().getValue();
	}

	@Override
	public String toString() {
		return "UPMQTextMessage {" + getMessageInfo() + "}";
	}
}
