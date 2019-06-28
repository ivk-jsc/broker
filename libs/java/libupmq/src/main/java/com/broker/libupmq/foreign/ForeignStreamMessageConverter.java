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
import javax.jms.Message;
import javax.jms.MessageEOFException;
import javax.jms.StreamMessage;

import com.broker.libupmq.message.UPMQStreamMessage;

class ForeignStreamMessageConverter extends AbstractMessageConverter {

	@Override
	protected Message create() throws JMSException {
		return new UPMQStreamMessage(null);
	}

	@Override
	protected void populate(Message source, Message target) throws JMSException {
		StreamMessage from = (StreamMessage) source;
		StreamMessage to = (StreamMessage) target;

		// populate header
		super.populate(from, to);

		// populate body
		from.reset(); // make sure the message can be read
		try {
			while (true) {
				Object object = from.readObject();
				to.writeObject(object);
			}
		} catch (MessageEOFException ignore) {
			// all done
		}
	}
}
