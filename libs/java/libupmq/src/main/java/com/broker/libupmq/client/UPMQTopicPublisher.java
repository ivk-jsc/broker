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
import javax.jms.Topic;
import javax.jms.TopicPublisher;

import com.broker.libupmq.session.UPMQSession;

public class UPMQTopicPublisher extends UPMQMessageProducer implements TopicPublisher {

	public UPMQTopicPublisher(UPMQSession sessionImpl, Destination destination) throws JMSException {
		super(sessionImpl, destination);
	}

	@Override
	public Topic getTopic() throws JMSException {
		return super.getTopic();
	}

	@Override
	public void publish(Message message) throws JMSException {
		super.publish(message);
	}

	@Override
	public void publish(Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		super.publish(message, deliveryMode, priority, timeToLive);
	}

	@Override
	public void publish(Topic topic, Message message) throws JMSException {
		super.publish(topic, message);
	}

	@Override
	public void publish(Topic topic, Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		super.publish(topic, message, deliveryMode, priority, timeToLive);
	}

}
