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
 
package com.broker.itest.jms.topic;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.TemporaryTopic;
import javax.jms.TextMessage;
import javax.jms.TopicSubscriber;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.TopicConnectionTestCase;

/**
 * Test the <code>javax.jms.TemporaryTopic</code> features.
 */
public class TemporaryTopicTest extends TopicConnectionTestCase {

	private TemporaryTopic tempTopic;
	private TopicSubscriber tempSubscriber;

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test a TemporaryTopic
	 */
	@Test
	public void testTemporaryTopic() {
		try {
			// we stop both publisher and subscriber connections
			//      publisherConnection.stop();
			//      subscriberConnection.stop();

			// we create a temporary topic to receive messages
			tempTopic = subscriberSession.createTemporaryTopic();
			// we recreate the sender because it has been
			// already created with another Destination as parameter
			publisher = publisherSession.createPublisher(tempTopic);
			// we create a temporary subscriber on the temporary topic
			tempSubscriber = subscriberSession.createSubscriber(tempTopic);
			subscriberConnection.start();
			publisherConnection.start();

			TextMessage message = publisherSession.createTextMessage();
			message.setText("testTemporaryTopic");
			publisher.publish(message);

			Message m = tempSubscriber.receive(ProviderLoader.TIMEOUT);
			assertTrue(m instanceof TextMessage);
			TextMessage msg = (TextMessage) m;
			assertEquals("testTemporaryTopic", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}
}
