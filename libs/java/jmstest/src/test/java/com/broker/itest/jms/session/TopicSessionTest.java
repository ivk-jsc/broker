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
 
package com.broker.itest.jms.session;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import javax.jms.InvalidDestinationException;
import javax.jms.InvalidSelectorException;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.Session;
import javax.jms.TextMessage;
import javax.jms.Topic;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.TopicConnectionTestCase;

/**
 * Test topic sessions <br />
 * See JMS specifications, 4.4 Session
 */
public class TopicSessionTest extends TopicConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that if we rollback a transaction which has consumed a message, the message is effectively redelivered.
	 */
	@Test
	public void testRollbackReceivedMessage() {
		try {
			publisherConnection.stop();
			// publisherSession has been declared has non transacted
			// we recreate it as a transacted session
			publisherSession = publisherConnection.createTopicSession(true, Session.SESSION_TRANSACTED);
			assertEquals(true, publisherSession.getTransacted());
			// we also recreate the publisher
			publisher = publisherSession.createPublisher(publisherTopic);
			publisherConnection.start();

			subscriberConnection.stop();
			// subscriberSession has been declared has non transacted
			// we recreate it as a transacted session
			subscriberSession = subscriberConnection.createTopicSession(true, Session.SESSION_TRANSACTED);
			assertEquals(true, subscriberSession.getTransacted());
			// we also recreate the subscriber
			subscriber = subscriberSession.createSubscriber(subscriberTopic);
			subscriberConnection.start();

			// we create a message...
			TextMessage message = publisherSession.createTextMessage();
			message.setText("testRollbackReceivedMessage");
			// ... publish it ...
			publisher.publish(message);
			// ... and commit the transaction
			publisherSession.commit();

			// we receive it
			Message msg1 = subscriber.receive(ProviderLoader.TIMEOUT);
			assertTrue("no message received", msg1 != null);
			assertTrue(msg1 instanceof TextMessage);
			assertEquals("testRollbackReceivedMessage", ((TextMessage) msg1).getText());

			// we rollback the transaction of subscriberSession
			subscriberSession.rollback();

			// we expect to receive a second time the message
			Message msg2 = subscriber.receive(ProviderLoader.TIMEOUT);
			assertTrue("no message received after rollbacking subscriber session.", msg2 != null);
			assertTrue(msg2 instanceof TextMessage);
			assertEquals("testRollbackReceivedMessage", ((TextMessage) msg2).getText());

			// finally we commit the subscriberSession transaction
			subscriberSession.commit();
		} catch (Exception e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a durable subscriber effectively receives the messages sent to its topic while it was inactive.
	 */
	@Test
	public void testDurableSubscriber() {
		try {
			subscriber = subscriberSession.createDurableSubscriber(subscriberTopic, subscriberTopic.getTopicName());
			subscriberConnection.close();
			subscriberConnection = null;

			TextMessage message = publisherSession.createTextMessage();
			message.setText("test");
			publisher.publish(message);

			subscriberConnection = subscriberTCF.createTopicConnection();
			if (subscriberConnection.getClientID() == null) {
				subscriberConnection.setClientID("subscriberConnection");
			}
			subscriberSession = subscriberConnection.createTopicSession(false, Session.AUTO_ACKNOWLEDGE);
			subscriber = subscriberSession.createDurableSubscriber(subscriberTopic, subscriberTopic.getTopicName());
			subscriberConnection.start();

			TextMessage m = (TextMessage) subscriber.receive(ProviderLoader.TIMEOUT);
			assertTrue(m != null);
			assertEquals("test", m.getText());

			subscriber.close();
			subscriberSession.unsubscribe(subscriberTopic.getTopicName());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Test
	public void testDurableConsumer() {
		try {
			subscriber = subscriberSession.createDurableSubscriber(subscriberTopic, subscriberTopic.getTopicName());
			subscriberConnection.close();
			subscriberConnection = null;

			TextMessage message = publisherSession.createTextMessage();
			message.setText("test");
			publisher.publish(message);

			subscriberConnection = subscriberTCF.createTopicConnection();
			if (subscriberConnection.getClientID() == null) {
				subscriberConnection.setClientID("subscriberConnection");
			}
			subscriberSession = subscriberConnection.createTopicSession(false, Session.AUTO_ACKNOWLEDGE);
			subscriber = subscriberSession.createDurableSubscriber(subscriberTopic, subscriberTopic.getTopicName());
			subscriberConnection.start();

			TextMessage m = (TextMessage) subscriber.receive(ProviderLoader.TIMEOUT);
			assertTrue(m != null);
			assertEquals("test", m.getText());

			subscriber.close();
			subscriberSession.unsubscribe(subscriberTopic.getTopicName());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that creating a duplicate subscriber in the same session throws JMSException
	 */
	@Test
	public void testDuplicateDurableSubscriber() {

		try {
			subscriber = subscriberSession.createDurableSubscriber(subscriberTopic, subscriberTopic.getTopicName());
			try {
				subscriberSession.createDurableSubscriber(subscriberTopic, subscriberTopic.getTopicName());
				fail("Duplicate durable subscriber in the same session, Should throw a javax.jms.JMSException.\n");
			} catch (JMSException expected) {
				subscriber.close();
				subscriberSession.unsubscribe(subscriberTopic.getTopicName());
			}
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the unsubscription of a durable subscriber.
	 */
	@Test
	public void testUnsubscribe() {
		try {
			subscriber = subscriberSession.createDurableSubscriber(subscriberTopic, subscriberTopic.getTopicName());
			subscriber.close();
			// nothing should happen when unsubscribing the durable subscriber
			subscriberSession.unsubscribe(subscriberTopic.getTopicName());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a call to the <code>createDurableSubscriber()</code> method with an invalid message selector throws a
	 * <code>javax.jms.InvalidSelectorException</code>.
	 */
	@Test
	public void testCreateDurableSubscriber_2() {
		try {
			subscriberSession.createDurableSubscriber(subscriberTopic, "topic", "definitely not a message selector!", true);
			fail("Should throw a javax.jms.InvalidSelectorException.\n");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidSelectorException, not a " + e);
		}
	}

	/**
	 * Test that a call to the <code>createDurableSubscriber()</code> method with an invalid <code>Topic</code> throws a
	 * <code>javax.jms.InvalidDestinationException</code>.
	 */
	@Test
	public void testCreateDurableSubscriber_1() {
		try {
			subscriberSession.createDurableSubscriber((Topic) null, subscriberTopic.getTopicName());
			fail("Should throw a javax.jms.InvalidDestinationException.\n");
		} catch (InvalidDestinationException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidDestinationException, not a " + e);
		}
	}

	/**
	 * Test that a call to the <code>createSubscriber()</code> method with an invalid message selector throws a
	 * <code>javax.jms.InvalidSelectorException</code>.
	 */
	@Test
	public void testCreateSubscriber_2() {
		try {
			subscriberSession.createSubscriber(subscriberTopic, "definitely not a message selector!", true);
			fail("Should throw a javax.jms.InvalidSelectorException.\n");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidSelectorException, not a " + e);
		}
	}

	/**
	 * Test that a call to the <code>createSubscriber()</code> method with an invalid <code>Topic</code> throws a
	 * <code>javax.jms.InvalidDestinationException</code>.
	 */
	@Test
	public void testCreateSubscriber_1() {
		try {
			subscriberSession.createSubscriber((Topic) null);
			fail("Should throw a javax.jms.InvalidDestinationException.\n");
		} catch (InvalidDestinationException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidDestinationException, not a " + e);
		}
	}
}
