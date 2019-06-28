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
 
package com.broker.itest.jms.message.properties;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Enumeration;

import javax.jms.ConnectionMetaData;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the JMSX defined properties. <br />
 * See JMS Specification, 3.5.9 JMS Defined Properties
 */
public class JMSXPropertyTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that the JMSX property <code>JMSXGroupID</code> is supported.
	 */
	@Test
	public void testSupportsJMSXGroupID() {
		try {
			boolean found = false;
			ConnectionMetaData metaData = senderConnection.getMetaData();
			Enumeration<?> e = metaData.getJMSXPropertyNames();
			while (e.hasMoreElements()) {
				String jmsxPropertyName = (String) e.nextElement();
				if (jmsxPropertyName.equals("JMSXGroupID")) {
					found = true;
				}
			}
			assertTrue("JMSXGroupID property is not supported", found);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the JMSX property <code>JMSXGroupID</code> works
	 */
	@Test
	public void testJMSXGroupID_1() {
		try {
			String groupID = "testSupportsJMSXGroupID_1:group";
			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("JMSXGroupID", groupID);
			message.setText("testSupportsJMSXGroupID_1");
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(m instanceof TextMessage);
			TextMessage msg = (TextMessage) m;
			assertEquals(groupID, msg.getStringProperty("JMSXGroupID"));
			assertEquals("testSupportsJMSXGroupID_1", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the JMSX property <code>JMSXGroupSeq</code> is supported.
	 */
	@Test
	public void testSupportsJMSXGroupSeq() {
		try {
			boolean found = false;
			ConnectionMetaData metaData = senderConnection.getMetaData();
			Enumeration<?> e = metaData.getJMSXPropertyNames();
			while (e.hasMoreElements()) {
				String jmsxPropertyName = (String) e.nextElement();
				if (jmsxPropertyName.equals("JMSXGroupSeq")) {
					found = true;
				}
			}
			assertTrue("JMSXGroupSeq property is not supported", found);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the JMSX property <code>JMSXDeliveryCount</code> is supported.
	 */
	@Test
	public void testSupportsJMSXDeliveryCount() {
		try {
			boolean found = false;
			ConnectionMetaData metaData = senderConnection.getMetaData();
			Enumeration<?> e = metaData.getJMSXPropertyNames();
			while (e.hasMoreElements()) {
				String jmsxPropertyName = (String) e.nextElement();
				if (jmsxPropertyName.equals("JMSXDeliveryCount")) {
					found = true;
				}
			}
			assertTrue("JMSXDeliveryCount property is not supported", found);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the JMSX property <code>JMSXDeliveryCount</code> works.
	 */
	@Test
	public void testJMSXDeliveryCount() {
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
			message.setText("testJMSXDeliveryCount");
			sender.send(message);
			// ... and commit the *producer* transaction
			senderSession.commit();

			// we receive a message...
			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(m != null);
			assertTrue(m instanceof TextMessage);
			TextMessage msg = (TextMessage) m;
			// ... which is the one which was sent...
			assertEquals("testJMSXDeliveryCount", msg.getText());
			// ...and has not been redelivered
			assertEquals(false, msg.getJMSRedelivered());
			// ... so it has been delivered once
			int jmsxDeliveryCount = msg.getIntProperty("JMSXDeliveryCount");
			assertEquals(1, jmsxDeliveryCount);
			// we rollback the *consumer* transaction
			receiverSession.rollback();

			// we receive again a message
			m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(m != null);
			assertTrue(m instanceof TextMessage);
			msg = (TextMessage) m;
			// ... which is still the one which was sent...
			assertEquals("testJMSXDeliveryCount", msg.getText());
			// .. but this time, it has been redelivered
			assertEquals(true, msg.getJMSRedelivered());
			// ... so it has been delivered a second time
			jmsxDeliveryCount = msg.getIntProperty("JMSXDeliveryCount");
			assertEquals(2, jmsxDeliveryCount);
			receiverSession.commit();
		} catch (JMSException e) {
			fail(e.getMessage());
		} catch (Exception e) {
			fail(e.getMessage());
		}
	}
}
