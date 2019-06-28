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
 
package com.broker.libupmq.session;

import javax.jms.JMSException;
import javax.jms.Queue;
import javax.jms.QueueBrowser;
import javax.jms.QueueReceiver;
import javax.jms.QueueSender;
import javax.jms.QueueSession;
import javax.jms.TemporaryQueue;

import com.broker.libupmq.connection.UPMQConnection;

public class UPMQQueueSession extends UPMQSession implements QueueSession {

	public UPMQQueueSession(UPMQConnection connectionImpl, boolean transacted, int acknowledgeMode) throws JMSException {
		super(connectionImpl, transacted, acknowledgeMode);
	}

	@Override
	public Queue createQueue(String queueName) throws JMSException {
		return super.createQueue(queueName);
	}

	@Override
	public QueueReceiver createReceiver(Queue queue) throws JMSException {
		return super.createReceiver(queue);
	}

	@Override
	public QueueReceiver createReceiver(Queue queue, String messageSelector) throws JMSException {
		return super.createReceiver(queue, messageSelector);
	}

	@Override
	public QueueSender createSender(Queue queue) throws JMSException {
		return super.createSender(queue);
	}

	@Override
	public QueueBrowser createBrowser(Queue queue) throws JMSException {
		return super.createBrowser(queue);
	}

	@Override
	public QueueBrowser createBrowser(Queue queue, String messageSelector) throws JMSException {
		return super.createBrowser(queue, messageSelector);
	}

	@Override
	public TemporaryQueue createTemporaryQueue() throws JMSException {
		return super.createTemporaryQueue();
	}
}
