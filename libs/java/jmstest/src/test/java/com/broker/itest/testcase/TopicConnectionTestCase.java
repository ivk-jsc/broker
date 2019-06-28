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

import javax.jms.Session;
import javax.jms.Topic;
import javax.jms.TopicConnection;
import javax.jms.TopicConnectionFactory;
import javax.jms.TopicPublisher;
import javax.jms.TopicSession;
import javax.jms.TopicSubscriber;

import org.junit.After;
import org.junit.Before;

import com.broker.itest.provider.Provider;
import com.broker.itest.provider.ProviderLoader;

public abstract class TopicConnectionTestCase extends ProviderLoader {

	private Provider admin;
	private String topicName;

	protected Topic publisherTopic;
	protected TopicPublisher publisher;
	protected TopicConnectionFactory publisherTCF;
	protected TopicConnection publisherConnection;
	protected TopicSession publisherSession;

	protected Topic subscriberTopic;
	protected TopicSubscriber subscriber;
	protected TopicConnectionFactory subscriberTCF;
	protected TopicConnection subscriberConnection;
	protected TopicSession subscriberSession;

	protected abstract String getTestName();

	@Before
	public void setUp() {
		try {

			if ((topicName = getTestName()) == null) {
				throw new InvalidParameterException("test name is null");
			}
			if ((admin = getProvider()) == null) {
				throw new InvalidParameterException("jms provider is null");
			}

			if (publisherTCF == null)
				publisherTCF = admin.createTopicConnectionFactory(tcfName);
			if (publisherTopic == null)
				publisherTopic = admin.createTopic(topicName);

			publisherConnection = publisherTCF.createTopicConnection();
			if (publisherConnection.getClientID() == null) {
				publisherConnection.setClientID("publisherConnection");
			}

			publisherSession = publisherConnection.createTopicSession(false, Session.AUTO_ACKNOWLEDGE);
			publisher = publisherSession.createPublisher(publisherTopic);

			if (subscriberTCF == null)
				subscriberTCF = admin.createTopicConnectionFactory(tcfName);
			if (subscriberTopic == null)
				subscriberTopic = admin.createTopic(topicName);

			subscriberConnection = subscriberTCF.createTopicConnection();
			if (subscriberConnection.getClientID() == null) {
				subscriberConnection.setClientID("subscriberConnection");
			}

			subscriberSession = subscriberConnection.createTopicSession(false, Session.AUTO_ACKNOWLEDGE);
			subscriber = subscriberSession.createSubscriber(subscriberTopic);

			publisherConnection.start();
			subscriberConnection.start();

		} catch (Exception e) {
			// XXX
			e.printStackTrace();
		}
	}

	@After
	public void tearDown() {
		try {
			if (publisherConnection != null)
				publisherConnection.close();
			if (subscriberConnection != null)
				subscriberConnection.close();

			if (admin != null) {
				admin.deleteTopicConnectionFactory(tcfName);
				admin.deleteTopic(subscriberTopic);
				admin.disconnect();
				admin = null;
			}
		} catch (Exception e) {
			// XXX 
			e.printStackTrace();
		} finally {
			publisherTopic = null;
			publisher = null;
			publisherTCF = null;
			publisherSession = null;
			publisherConnection = null;

			subscriberTopic = null;
			subscriber = null;
			subscriberTCF = null;
			subscriberSession = null;
			subscriberConnection = null;
		}
	}

}
