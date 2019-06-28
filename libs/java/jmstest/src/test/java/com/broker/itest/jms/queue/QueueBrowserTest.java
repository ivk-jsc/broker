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
 
package com.broker.itest.jms.queue;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Enumeration;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.QueueBrowser;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the <code>javax.jms.QueueBrowser</code> features.
 */
public class QueueBrowserTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * The <code>QueueBrowser</code> of the receiver's session
	 */
	protected QueueBrowser receiverBrowser;

	/**
	 * The <code>QueueBrowser</code> of the sender's session
	 */
	protected QueueBrowser senderBrowser;

	/**
	 * Test the <code>QueueBrowser</code> of the sender.
	 */

	@Test
	public void testSenderBrowser() {
		try {
			TextMessage message_1 = senderSession.createTextMessage();
			message_1.setText("testBrowser:message_1");
			TextMessage message_2 = senderSession.createTextMessage();
			message_2.setText("testBrowser:message_2");

			// send two messages...
			sender.send(message_1);
			sender.send(message_2);

			// ask the browser to browse the sender's session
			Enumeration<?> enumeration = senderBrowser.getEnumeration();
			int count = 0;
			while (enumeration.hasMoreElements()) {
				// one more message in the queue
				count++;
				// check that the message in the queue is one of the two which where
				// sent
				Object obj = enumeration.nextElement();
				assertTrue("No message was received", obj != null);
				if (obj == null)
					return;
				assertTrue(obj instanceof TextMessage);
				TextMessage msg = (TextMessage) obj;
				assertTrue(msg.getText().startsWith("testBrowser:message_"));
			}
			// check that there is effectively 2 messages in the queue
			assertEquals(2, count);

			// receive the first message...
			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			// ... and check it is the first which was sent.
			assertTrue(m instanceof TextMessage);
			TextMessage msg = (TextMessage) m;
			assertEquals("testBrowser:message_1", msg.getText());

			// receive the second message...
			m = receiver.receive(ProviderLoader.TIMEOUT);
			// ... and check it is the second which was sent.
			assertTrue(m instanceof TextMessage);
			msg = (TextMessage) m;
			assertEquals("testBrowser:message_2", msg.getText());

			// ask the browser to browse the sender's session
			enumeration = receiverBrowser.getEnumeration();
			// check that there is no messages in the queue
			// (the two messages have been acknowledged and so removed
			// from the queue)
			assertTrue(!enumeration.hasMoreElements());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the <code>QueueBrowser</code> after receive.
	 */

	@Test
	public void testSendBrowserReceiveBrowser() {
		try {
			TextMessage message = senderSession.createTextMessage();
			message.setText("testSendBrowserReceiveBrowser");

			int i = 0;
			while (i != 20) {
				message.setIntProperty("testSendBrowserReceiveBrowser", i);
				sender.send(message);
				i++;
			}

			Enumeration<?> enumerationSender = senderBrowser.getEnumeration();
			i = 0;
			while (enumerationSender.hasMoreElements()) {
				Object obj = enumerationSender.nextElement();
				assertTrue("No message was received", obj != null);
				if (obj == null)
					return;
				assertTrue(obj instanceof TextMessage);
				TextMessage msg = (TextMessage) obj;
				assertTrue(msg.getText().equals("testSendBrowserReceiveBrowser"));
				assertTrue(msg.getIntProperty("testSendBrowserReceiveBrowser") == i);
				i++;
			}
			assertEquals(i, 20);

			i = 0;
			while (i != 10) {
				Message m = receiver.receive(ProviderLoader.TIMEOUT);
				assertTrue(m instanceof TextMessage);
				TextMessage msg = (TextMessage) m;
				assertTrue(msg.getText().equals("testSendBrowserReceiveBrowser"));
				assertTrue(msg.getIntProperty("testSendBrowserReceiveBrowser") == i);
				i++;
			}

			Enumeration<?> enumerationReceiver = receiverBrowser.getEnumeration();
			i = 0;
			while (enumerationReceiver.hasMoreElements()) {
				Object obj = enumerationReceiver.nextElement();
				assertTrue("No message was received", obj != null);
				if (obj == null)
					return;
				assertTrue(obj instanceof TextMessage);
				TextMessage msg = (TextMessage) obj;
				assertTrue(msg.getText().equals("testSendBrowserReceiveBrowser"));
				assertTrue((msg.getIntProperty("testSendBrowserReceiveBrowser") - 10) == i);
				i++;
			}
			assertEquals(i, 10);

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a <code>QueueBrowser</cdeo> created with a message selector browses only the messages matching this
	 * selector.
	 */
	@Test
	public void testBrowserWithMessageSelector() {
		try {
			senderBrowser = senderSession.createBrowser(senderQueue, "pi = 3.14159");

			TextMessage message_1 = senderSession.createTextMessage();
			message_1.setText("testBrowserWithMessageSelector:message_1");
			TextMessage message_2 = senderSession.createTextMessage();
			message_2.setDoubleProperty("pi", 3.14159);
			message_2.setText("testBrowserWithMessageSelector:message_2");

			sender.send(message_1);
			sender.send(message_2);

			Enumeration<?> enumeration = senderBrowser.getEnumeration();
			int count = 0;
			while (enumeration.hasMoreElements()) {
				count++;
				Object obj = enumeration.nextElement();
				assertTrue("No message was received", obj != null);
				if (obj == null)
					return;
				assertTrue(obj instanceof TextMessage);
				TextMessage msg = (TextMessage) obj;
				assertEquals("testBrowserWithMessageSelector:message_2", msg.getText());
			}
			assertEquals(1, count);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Override
	public void setUp() {
		try {
			super.setUp();
			receiverBrowser = receiverSession.createBrowser(receiverQueue);
			senderBrowser = senderSession.createBrowser(senderQueue);
		} catch (JMSException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void tearDown() {
		try {
			if (receiverBrowser != null)
				receiverBrowser.close();
			if (senderBrowser != null)
				senderBrowser.close();
			super.tearDown();
		} catch (JMSException e) {
			e.printStackTrace();
		} finally {
			receiverBrowser = null;
			senderBrowser = null;
		}
	}
}
