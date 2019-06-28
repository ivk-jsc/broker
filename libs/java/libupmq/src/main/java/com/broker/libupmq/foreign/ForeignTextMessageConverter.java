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
import javax.jms.TextMessage;

import com.broker.libupmq.message.UPMQTextMessage;

class ForeignTextMessageConverter extends AbstractMessageConverter {

	@Override
	protected Message create() throws JMSException {
		return new UPMQTextMessage(null);
	}

	@Override
	protected void populate(Message source, Message target) throws JMSException {
		TextMessage from = (TextMessage) source;
		TextMessage to = (TextMessage) target;

		// populate header
		super.populate(from, to);

		// populate body
		to.setText(from.getText());
	}
}
