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
import javax.jms.Queue;
import javax.jms.QueueBrowser;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test queue sessions <br />
 * See JMS specifications, 4.4 Session
 */
public class QueueSessionTest extends QueueConnectionTestCase {

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
	public void testRollbackRececeivedMessage() {
		try {
			senderConnection.stop();
			// senderSession has been created as non transacted
			// we create it again but as a transacted session
			senderSession.close();
			senderSession = senderConnection.createQueueSession(true, 0);
			assertEquals(true, senderSession.getTransacted());
			// we create again the sender
			sender = senderSession.createSender(senderQueue);
			senderConnection.start();

			receiverConnection.stop();
			// receiverSession has been created as non transacted
			// we create it again but as a transacted session
			//need close previous session
			receiverSession.close();
			receiverSession = receiverConnection.createQueueSession(true, 0);
			assertEquals(true, receiverSession.getTransacted());
			// we create again the receiver
			receiver = receiverSession.createReceiver(receiverQueue);
			receiverConnection.start();

			// we send a message...
			TextMessage message = senderSession.createTextMessage();
			message.setText("testRollbackRececeivedMessage");
			sender.send(message);
			// ... and commit the *producer* transaction
			senderSession.commit();

			// we receive a message...
			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(m != null);
			assertTrue(m instanceof TextMessage);
			TextMessage msg = (TextMessage) m;
			// ... which is the one which was sent...
			assertEquals("testRollbackRececeivedMessage", msg.getText());
			// ...and has not been redelivered
			assertEquals(false, msg.getJMSRedelivered());

			// we rollback the *consumer* transaction
			receiverSession.rollback();

			// we receive again a message
			m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(m != null);
			assertTrue(m instanceof TextMessage);
			msg = (TextMessage) m;
			// ... which is still the one which was sent...
			assertEquals("testRollbackRececeivedMessage", msg.getText());
			// .. but this time, it has been redelivered
			assertEquals(true, msg.getJMSRedelivered());

		} catch (Exception e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a call to the <code>createBrowser()</code> method with an invalid messaeg session throws a
	 * <code>javax.jms.InvalidSelectorException</code>.
	 */
	@Test
	public void testCreateBrowser_2() {
		try {
			@SuppressWarnings("unused")
			QueueBrowser browser = senderSession.createBrowser(senderQueue, "definitely not a message selector!");
			fail("Should throw a javax.jms.InvalidSelectorException.\n");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidSelectorException, not a " + e);
		}
	}

	/**
	 * Test that a call to the <code>createBrowser()</code> method with an invalid <code>Queue</code> throws a
	 * <code>javax.jms.InvalidDestinationException</code>.
	 */
	@Test
	public void testCreateBrowser_1() {
		try {
			@SuppressWarnings("unused")
			QueueBrowser browser = senderSession.createBrowser((Queue) null);
			fail("Should throw a javax.jms.InvalidDestinationException.\n");
		} catch (InvalidDestinationException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidDestinationException, not a " + e);
		}
	}

	/**
	 * Test that a call to the <code>createReceiver()</code> method with an invalid message selector throws a
	 * <code>javax.jms.InvalidSelectorException</code>.
	 */
	@Test
	public void testCreateReceiver_2() {
		try {
			receiver = senderSession.createReceiver(senderQueue, "definitely not a message selector!");
			fail("Should throw a javax.jms.InvalidSelectorException.\n");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidSelectorException, not a " + e);
		}
	}

	/**
	 * Test that a call to the <code>createReceiver()</code> method with an invalid <code>Queue</code> throws a
	 * <code>javax.jms.InvalidDestinationException</code>>
	 */
	@Test
	public void testCreateReceiver_1() {
		try {
			receiver = senderSession.createReceiver((Queue) null);
			fail("Should throw a javax.jms.InvalidDestinationException.\n");
		} catch (InvalidDestinationException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.InvalidDestinationException, not a " + e);
		}
	}
}
