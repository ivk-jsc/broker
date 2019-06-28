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

import java.io.Serializable;

import javax.jms.JMSException;
import javax.jms.ObjectMessage;

import com.broker.libupmq.client.UPMQMessageConsumer;
import com.broker.protocol.Protocol;

public class UPMQObjectMessage extends UPMQMessage implements ObjectMessage {

	public UPMQObjectMessage(UPMQMessageConsumer consumer) throws JMSException {
		super(consumer);

		_headerBuilder.setBodyType(Protocol.Body.OBJECT_BODY_FIELD_NUMBER);
	}

	@Override
	public void setObject(Serializable object) throws JMSException {
		checkWrite();

		if (object == null)
			_bodyBuilder.clearObjectBody();
		else
			_bodyBuilder.getObjectBodyBuilder().setValue(UPMQMessage.serializeObjectToString(object));
	}

	@Override
	public Serializable getObject() throws JMSException {
		if (_bodyBuilder.getBodyTypeCase() != Protocol.Body.BodyTypeCase.OBJECT_BODY)
			return null;
		else
			return (Serializable) UPMQMessage.deserializeObjectFromString(_bodyBuilder.getObjectBody().getValue());
	}

	@Override
	public String toString() {
		return "UPMQObjectMessage {" + getMessageInfo() + "}";
	}
}
