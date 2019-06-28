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
import javax.jms.TemporaryTopic;
import javax.jms.Topic;
import javax.jms.TopicPublisher;
import javax.jms.TopicSession;
import javax.jms.TopicSubscriber;

import com.broker.libupmq.connection.UPMQConnection;

public class UPMQTopicSession extends UPMQSession implements TopicSession {

	public UPMQTopicSession(UPMQConnection connectionImpl, boolean transacted, int acknowledgeMode) throws JMSException {
		super(connectionImpl, transacted, acknowledgeMode);
	}

	@Override
	public Topic createTopic(String topicName) throws JMSException {
		return super.createTopic(topicName);
	}

	@Override
	public TopicSubscriber createSubscriber(Topic topic) throws JMSException {
		return super.createSubscriber(topic);
	}

	@Override
	public TopicSubscriber createSubscriber(Topic topic, String messageSelector, boolean noLocal) throws JMSException {
		return super.createSubscriber(topic, messageSelector, noLocal);
	}

	@Override
	public TopicSubscriber createDurableSubscriber(Topic topic, String name) throws JMSException {
		return super.createDurableSubscriber(topic, name);
	}

	@Override
	public TopicSubscriber createDurableSubscriber(Topic topic, String name, String messageSelector, boolean noLocal) throws JMSException {
		return super.createDurableSubscriber(topic, name, messageSelector, noLocal);
	}

	@Override
	public TopicPublisher createPublisher(Topic topic) throws JMSException {
		return super.createPublisher(topic);
	}

	@Override
	public TemporaryTopic createTemporaryTopic() throws JMSException {
		return super.createTemporaryTopic();
	}

	@Override
	public void unsubscribe(String name) throws JMSException {
		super.unsubscribe(name);
	}
}
