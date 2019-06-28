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
 
package com.broker.itest.testcase;

import java.security.InvalidParameterException;

import javax.jms.Queue;
import javax.jms.QueueConnection;
import javax.jms.QueueConnectionFactory;
import javax.jms.QueueSession;
import javax.jms.Session;
import javax.jms.Topic;
import javax.jms.TopicConnection;
import javax.jms.TopicConnectionFactory;
import javax.jms.TopicSession;

import org.junit.After;
import org.junit.Before;

import com.broker.itest.provider.Provider;
import com.broker.itest.provider.ProviderLoader;

public abstract class UnifiedConnectionTestCase extends ProviderLoader {

	private Provider admin;

	protected QueueConnectionFactory queueFactory;
	protected QueueConnection queueConnection;
	protected QueueSession queueSession;
	protected Queue queue;
	protected String queueName;

	protected TopicConnectionFactory topicFactory;
	protected TopicConnection topicConnection;
	protected TopicSession topicSession;
	protected Topic topic;
	protected String topicName;

	protected abstract String getTestName();

	@Before
	public void setUp() {

		try {

			if ((queueName = getTestName()) == null) {
				throw new InvalidParameterException("test name is null");
			}
			if ((topicName = getTestName()) == null) {
				throw new InvalidParameterException("test name is null");
			}
			if ((admin = getProvider()) == null) {
				throw new InvalidParameterException("jms provider is null");
			}

			queueName += "_queue";
			topicName += "_topic";

			if (queueFactory == null)
				queueFactory = admin.createQueueConnectionFactory(qcfName);
			if (topicFactory == null)
				topicFactory = admin.createTopicConnectionFactory(tcfName);

			if (queue == null)
				queue = admin.createQueue(queueName);
			if (topic == null)
				topic = admin.createTopic(topicName);

			queueConnection = queueFactory.createQueueConnection();
			queueSession = queueConnection.createQueueSession(false, Session.AUTO_ACKNOWLEDGE);

			topicConnection = topicFactory.createTopicConnection();
			topicSession = topicConnection.createTopicSession(false, Session.AUTO_ACKNOWLEDGE);

			queueConnection.start();
			topicConnection.start();

		} catch (Exception e) {
			// XXX
			e.printStackTrace();
		}
	}

	@After
	public void tearDown() {
		try {
			if (queueConnection != null)
				queueConnection.close();
			if (topicConnection != null)
				topicConnection.close();

			if (admin != null) {
				admin.deleteTopicConnectionFactory(tcfName);
				admin.deleteQueueConnectionFactory(qcfName);
				admin.deleteTopic(topic);
				admin.deleteQueue(queue);
				admin.disconnect();
				admin = null;
			}

		} catch (Exception e) {
			// XXX
			e.printStackTrace();
		} finally {
			queueFactory = null;
			queueConnection = null;
			queueSession = null;
			queue = null;

			topicFactory = null;
			topicConnection = null;
			topicSession = null;
			topic = null;
		}
	}

}
