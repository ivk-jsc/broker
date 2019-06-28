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
 
package com.broker.itest.jms.message.header;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

import javax.jms.DeliveryMode;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.Queue;
import javax.jms.Session;
import javax.jms.TemporaryQueue;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.Provider;
import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the headers of a message
 */
public class MessageHeaderTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test priority of the message.
	 */
	@Test
	public void testJMSPriority_3() {
		try {
			Message message = senderSession.createMessage();
			for (int i = 0; i < 10; i++) {
				sender.send(message, Session.AUTO_ACKNOWLEDGE, i, 0);
			}

			Thread.sleep(3000);

			for (int j = 9; j >= 0; j--) {
				Message m = receiver.receive(ProviderLoader.TIMEOUT);
				assertEquals("3.4.10 Message priority failed.\n", j, m.getJMSPriority());
			}
		} catch (JMSException | InterruptedException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>MessageProducer.setPriority()</code> changes effectively priority of the message.
	 */
	@Test
	public void testJMSPriority_2() {
		try {
			Message message = senderSession.createMessage();
			sender.send(message);
			sender.setPriority(9);
			sender.send(message);
			assertEquals("3.4.9 After completion of the send it holds the value specified by the " + "method sending the message.\n", 9, message.getJMSPriority());

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(msg != null);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the priority set by <code>Message.setJMSPriority()</code> is ignored when a message is sent and that it
	 * holds the value specified when sending the message (i.e. <code>Message.DEFAULT_PRIORITY</code> in this test).
	 */
	@Test
	public void testJMSPriority_1() {
		try {
			Message message = senderSession.createMessage();
			message.setJMSPriority(0);
			sender.send(message);
			assertTrue("3.4.9 When a message is sent this value is ignored.\n", message.getJMSPriority() != 0);
			assertEquals("3.4.9 After completion of the send it holds the value specified by the " + "method sending the message.\n", Message.DEFAULT_PRIORITY, message.getJMSPriority());

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(msg != null);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the value of the <code>JMSExpiration<code> header field is the same for the sent message and the
	 * received one.
	 */
	@Test
	public void testJMSExpiration() {
		try {
			Message message = senderSession.createMessage();
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertEquals("3.4.9 When a message is received its JMSExpiration header field contains this same " + "value [i.e. set on return of the send method].\n", message.getJMSExpiration(),
					msg.getJMSExpiration());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>JMSMessageID</code> is set by the provider when the <code>send</code> method returns and that
	 * it starts with <code>"ID:"</code>.
	 */
	@Test
	public void testJMSMessageID_2() {
		try {
			Message message = senderSession.createMessage();
			sender.send(message);
			assertTrue("3.4.3 When the send method returns it contains a provider-assigned value.\n", message.getJMSMessageID() != null);
			assertTrue("3.4.3 All JMSMessageID values must start with the prefix 'ID:'.\n", message.getJMSMessageID().startsWith("ID:"));

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("3.4.3 All JMSMessageID values must start with the prefix 'ID:'.\n", msg.getJMSMessageID().startsWith("ID:"));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>JMSMessageID</code> header field value is ignored when the message is sent.
	 */
	@Test
	public void testJMSMessageID_1() {
		try {
			Message message = senderSession.createMessage();
			message.setJMSMessageID("ID:foo");
			sender.send(message);
			assertTrue("3.4.3 When a message is sent this value is ignored.\n", message.getJMSMessageID() != "ID:foo");
			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(msg != null);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>JMSDeliveryMode</code> header field value is ignored when the message is sent and that it
	 * holds the value specified by the sending method (i.e. <code>Message.DEFAULT_DELIVERY_MODE</code> in this test
	 * when the message is received.
	 */
	@Test
	public void testJMSDeliveryMode() {
		try {
			// sender has been created with the DEFAULT_DELIVERY_MODE which is
			// PERSISTENT
			assertEquals(DeliveryMode.PERSISTENT, sender.getDeliveryMode());
			Message message = senderSession.createMessage();
			// send a message specfiying NON_PERSISTENT for the JMSDeliveryMode header
			// field
			message.setJMSDeliveryMode(DeliveryMode.NON_PERSISTENT);
			sender.send(message);
			assertTrue("3.4.2 When a message is sent this value is ignored", message.getJMSDeliveryMode() != DeliveryMode.NON_PERSISTENT);
			assertEquals("3.4.2 After completion of the send it holds the delivery mode specified " + "by the sending method (persistent by default).\n", Message.DEFAULT_DELIVERY_MODE,
					message.getJMSDeliveryMode());

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(msg != null);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>JMSDestination</code> header field value is ignored when the message is sent and that after
	 * completion of the sending method, it holds the <code>Destination</code> specified by the sending method. Also
	 * test that the value of the header on the received message is the same that on the sent message.
	 */
	@Test
	public void testJMSDestination() {
		try {
			Provider admin = getProvider();
			admin.createQueue("anotherQueue");
			Queue anotherQueue = admin.createQueue("anotherQueue");
			assertTrue(anotherQueue != senderQueue);

			// set the JMSDestination header field to the anotherQueue Destination
			Message message = senderSession.createMessage();
			message.setJMSDestination(anotherQueue);
			sender.send(message);
			assertTrue("3.4.1 When a message is sent this value is ignored.\n", message.getJMSDestination() != anotherQueue);
			assertEquals("3.4.1 After completion of the send it holds the destination object specified " + "by the sending method.\n", senderQueue, message.getJMSDestination());

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertEquals("3.4.1 When a message is received, its destination value must be equivalent  " + " to the value assigned when it was sent.\n",
					((Queue) message.getJMSDestination()).getQueueName(), ((Queue) msg.getJMSDestination()).getQueueName());

			admin.deleteQueue(anotherQueue);
		} catch (Exception e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a <code>Destination</code> set by the <code>setJMSReplyTo()</code> method on a sended message
	 * corresponds to the <code>Destination</code> get by the </code>getJMSReplyTo()</code> method.
	 */
	@Test
	public void testJMSReplyTo_1() {
		try {
			Message message = senderSession.createMessage();
			message.setJMSReplyTo(senderQueue);
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			Destination dest = msg.getJMSDestination();//getJMSReplyTo();
			assertTrue("JMS ReplyTo header field should be a Queue", dest instanceof Queue);
			Queue replyTo = (Queue) dest;
			assertEquals("JMS ReplyTo header field should be equals to the sender queue", replyTo.getQueueName(), senderQueue.getQueueName());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that if the JMS ReplyTo header field has been set as a <code>TemporaryQueue</code>, it will be rightly get
	 * also as a <code>TemporaryQueue</code> (and not only as a <code>Queue</code>).
	 */
	@Test
	public void testJMSReplyTo_2() {
		try {
			TemporaryQueue tempQueue = senderSession.createTemporaryQueue();
			Message message = senderSession.createMessage();
			message.setJMSReplyTo(tempQueue);
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			Destination dest = msg.getJMSReplyTo();
			assertTrue("JMS ReplyTo header field should be a TemporaryQueue", dest instanceof TemporaryQueue);
			Queue replyTo = (Queue) dest;
			assertEquals("JMS ReplyTo header field should be equals to the temporary queue", replyTo.getQueueName(), ((Queue) tempQueue).getQueueName());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Test
	public void testJMSExpiration_online() {
		try {

			long expirationInterval = 2 * 1000;
			long sleepInterval = 4 * 1000;
			long messageCount = 3;

			HashSet expiringIds = new HashSet();
			HashSet nonExpiringIds = new HashSet();

			Message message = senderSession.createMessage();

			// send messages that expire
			for (int i = 0; i < messageCount; ++i) {
				sender.send(message, Message.DEFAULT_DELIVERY_MODE, Message.DEFAULT_PRIORITY, expirationInterval);
				expiringIds.add(message.getJMSMessageID());
			}

			// send messages that don't expire
			for (int i = 0; i < messageCount; ++i) {
				sender.send(message, Message.DEFAULT_DELIVERY_MODE, Message.DEFAULT_PRIORITY, Message.DEFAULT_TIME_TO_LIVE);
				nonExpiringIds.add(message.getJMSMessageID());
			}

			assertEquals("Duplicate JMSMessageID allocated", messageCount, expiringIds.size());
			assertEquals("Duplicate JMSMessageID allocated", messageCount, nonExpiringIds.size());

			// now wait to allow the messages to expire
			try {
				Thread.sleep(sleepInterval);
			} catch (InterruptedException ignore) {
			}

			List messages = new ArrayList();
			message = null;

			while ((message = receiver.receive(ProviderLoader.TIMEOUT)) != null) {
				messages.add(message);
			}

			Iterator iterator = messages.iterator();
			while (iterator.hasNext()) {
				Message received = (Message) iterator.next();
				String id = received.getJMSMessageID();
				if (!nonExpiringIds.contains(id)) {
					if (expiringIds.contains(id)) {
						fail("Received a message which should have " + "expired: " + id);
					} else {
						fail("Received a message which wasn't sent by this" + " test: " + id);
					}
				}
			}

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Test
	public void testJMSExpiration_offline() {
		try {

			receiverConnection.stop();

			long expirationInterval = 2 * 1000;
			long sleepInterval = 4 * 1000;
			long messageCount = 3;

			HashSet expiringIds = new HashSet();
			HashSet nonExpiringIds = new HashSet();

			Message message = senderSession.createMessage();

			// send messages that expire
			for (int i = 0; i < messageCount; ++i) {
				sender.send(message, Message.DEFAULT_DELIVERY_MODE, Message.DEFAULT_PRIORITY, expirationInterval);
				expiringIds.add(message.getJMSMessageID());
			}

			// send messages that don't expire
			for (int i = 0; i < messageCount; ++i) {
				sender.send(message, Message.DEFAULT_DELIVERY_MODE, Message.DEFAULT_PRIORITY, Message.DEFAULT_TIME_TO_LIVE);
				nonExpiringIds.add(message.getJMSMessageID());
			}

			assertEquals("Duplicate JMSMessageID allocated", messageCount, expiringIds.size());
			assertEquals("Duplicate JMSMessageID allocated", messageCount, nonExpiringIds.size());

			// now wait to allow the messages to expire
			try {
				Thread.sleep(sleepInterval);
			} catch (InterruptedException ignore) {
			}

			receiverConnection.start();

			List messages = new ArrayList();
			message = null;

			while ((message = receiver.receive(ProviderLoader.TIMEOUT)) != null) {
				messages.add(message);
			}

			Iterator iterator = messages.iterator();
			while (iterator.hasNext()) {
				Message received = (Message) iterator.next();
				String id = received.getJMSMessageID();
				if (!nonExpiringIds.contains(id)) {
					if (expiringIds.contains(id)) {
						fail("Received a message which should have " + "expired: " + id);
					} else {
						fail("Received a message which wasn't sent by this" + " test: " + id);
					}
				}
			}

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}
}
