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

import java.util.Enumeration;

import javax.jms.JMSException;
import javax.jms.Message;

abstract class AbstractMessageConverter implements ForeignConverter {

	@Override
	public Message convert(Message message) throws JMSException {
		Message result = create();
		populate(message, result);
		return result;
	}

	protected abstract Message create() throws JMSException;

	protected void populate(Message source, Message target) throws JMSException {

		target.setJMSMessageID(source.getJMSMessageID());
		target.setJMSDestination(source.getJMSDestination());
		target.setJMSTimestamp(source.getJMSTimestamp());
		target.setJMSPriority(source.getJMSPriority());
		target.setJMSExpiration(source.getJMSExpiration());
		target.setJMSDeliveryMode(source.getJMSDeliveryMode());
		target.setJMSCorrelationID(source.getJMSCorrelationID());
		target.setJMSType(source.getJMSType());
		target.setJMSReplyTo(source.getJMSReplyTo());

		// populate properties
		Enumeration<?> iterator = source.getPropertyNames();
		while (iterator.hasMoreElements()) {
			String name = (String) iterator.nextElement();
			if (!name.startsWith("JMSX") || name.equals("JMSXGroupID") || name.equals("JMSXGroupSeq")) {
				// only populate user and supported JMSX properties

				Object value = source.getObjectProperty(name);
				target.setObjectProperty(name, value);
			}
		}
	}

}
