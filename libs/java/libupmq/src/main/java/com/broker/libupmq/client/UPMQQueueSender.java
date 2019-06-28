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
import javax.jms.Message;
import javax.jms.Queue;
import javax.jms.QueueSender;

import com.broker.libupmq.session.UPMQSession;

public class UPMQQueueSender extends UPMQMessageProducer implements QueueSender {

	public UPMQQueueSender(UPMQSession sessionImpl, Destination destination) throws JMSException {
		super(sessionImpl, destination);
	}

	@Override
	public Queue getQueue() throws JMSException {
		return super.getQueue();
	}

	@Override
	public void send(Message message) throws JMSException {
		super.send(message);
	}

	@Override
	public void send(Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		super.send(message, deliveryMode, priority, timeToLive);
	}

	@Override
	public void send(Queue queue, Message message) throws JMSException {
		super.send(queue, message);
	}

	@Override
	public void send(Queue queue, Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		super.send(queue, message, deliveryMode, priority, timeToLive);
	}
}
