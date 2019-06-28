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
 
package com.broker.libupmq.client;

import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Queue;
import javax.jms.QueueReceiver;

import com.broker.libupmq.session.UPMQSession;

public class UPMQQueueReceiver extends UPMQMessageConsumer implements QueueReceiver {

	public UPMQQueueReceiver(UPMQSession sessionImpl, Destination destination, String messageSelector, String subscription, boolean noLocal, UPMQConsumer type) throws JMSException {
		super(sessionImpl, destination, messageSelector, subscription, noLocal, type);
	}

	@Override
	public Queue getQueue() throws JMSException {
		return super.getQueue();
	}
}
