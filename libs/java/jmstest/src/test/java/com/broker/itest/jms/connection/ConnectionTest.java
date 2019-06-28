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
 
package com.broker.itest.jms.connection;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageListener;
import javax.jms.Session;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test connections.
 * 
 * See JMS specifications, 4.3.5 Closing a Connection
 * 
 */
public class ConnectionTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that invoking the <code>acknowledge()</code> method of a received message from a closed connection's session
	 * must throw an <code>IllegalStateException</code>.
	 */
	@Test
	public void testAcknowledge() {
		try {

			receiverConnection.stop();
			//need close previous session
			receiverSession.close();
			receiverSession = receiverConnection.createQueueSession(false, Session.CLIENT_ACKNOWLEDGE);
			receiver = receiverSession.createReceiver(receiverQueue);
			receiverConnection.start();

			Message message = senderSession.createMessage();
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			receiverConnection.close();
			m.acknowledge();
			fail("4.3.5 Invoking the acknowledge method of a received message from a closed " + "connection's session must throw a [javax.jms.]IllegalStateException.\n");
		} catch (javax.jms.IllegalStateException e) {
		} catch (JMSException e) {
			fail("4.3.5 Invoking the acknowledge method of a received message from a closed " + "connection's session must throw a [javax.jms.]IllegalStateException, not a " + e);
		} catch (java.lang.IllegalStateException e) {
			fail("4.3.5 Invoking the acknowledge method of a received message from a closed " + "connection's session must throw an [javax.jms.]IllegalStateException "
					+ "[not a java.lang.IllegalStateException]");
		}
	}

	/**
	 * Test that an attempt to use a <code>Connection</code> which has been closed throws a
	 * <code>javax.jms.IllegalStateException</code>.
	 */
	@Test
	public void testUseClosedConnection() {
		try {
			senderConnection.close();
			senderConnection.createQueueSession(false, Session.AUTO_ACKNOWLEDGE);
			fail("Should raise a javax.jms.IllegalStateException");
		} catch (javax.jms.IllegalStateException e) {
		} catch (JMSException e) {
			fail("Should raise a javax.jms.IllegalStateException, not a " + e);
		} catch (java.lang.IllegalStateException e) {
			fail("Should raise a javax.jms.IllegalStateException, not a java.lang.IllegalStateException");
		}
	}

	/**
	 * Test session creation without using parameters
	 */
	@Test
	public void testSessionCreation() {
		try {
			senderConnection.start();
			javax.jms.Session session = senderConnection.createSession(false, Session.AUTO_ACKNOWLEDGE);
			assertTrue("Sesssion is created", (session != null));
		} catch (JMSException e) {
			fail("Unable to create session without parameters " + e);
		}
	}

	/**
	 * Test session creation without using parameters
	 */
	@Test
	public void testSessionCreationSessionMode() {
		try {
			senderConnection.start();
			javax.jms.Session session = senderConnection.createSession(true, Session.SESSION_TRANSACTED);
			assertTrue("Sesssion is created", (session != null));
			assertTrue("Sesssion transacted is true", (session.getTransacted() == true));
		} catch (JMSException e) {
			fail("Unable to create session with session mode as parameter " + e);
		}
	}

	/**
	 * Test that a <code>MessageProducer</code> can send messages while a <code>Connection</code> is stopped.
	 */
	@Test
	public void testMessageSentWhenConnectionClosed() {
		try {
			senderConnection.stop();
			Message message = senderSession.createTextMessage();
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(m != null);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that closing a closed connectiondoes not throw an exception.
	 */
	@Test
	public void testCloseClosedConnection() {
		try {
			// senderConnection is already started
			// we close it once
			senderConnection.close();
			// we close it a second time
			senderConnection.close();
		} catch (Exception e) {
			fail("4.3.5 Closing a closed connection must not throw an exception.\n");
		}
	}

	/**
	 * Test that starting a started connection is ignored
	 */
	@Test
	public void testStartStartedConnection() {
		try {
			// senderConnection is already started
			// start it again should be ignored
			senderConnection.start();
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that stopping a stopped connection is ignored
	 */
	@Test
	public void testStopStoppedConnection() {
		try {
			// senderConnection is started
			// we stop it once
			senderConnection.stop();
			// stopping it a second time is ignored
			senderConnection.stop();
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that delivery of message is stopped if the message consumer connection is stopped
	 */
	@Test
	public void testStopConsumerConnection() {
		try {
			receiverConnection.stop();

			receiver.setMessageListener(new MessageListener() {
				@Override
				public void onMessage(Message m) {
					try {
						fail("The message must not be received, the consumer connection is stopped");
						assertEquals("test", ((TextMessage) m).getText());
					} catch (JMSException e) {
						fail(e.getMessage());
					}
				}
			});

			TextMessage message = senderSession.createTextMessage();
			message.setText("test");
			sender.send(message);
			synchronized (this) {
				try {
					wait(1000);
				} catch (Exception e) {
					fail(e.getMessage());
				}
			}
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}
}
