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
 
package com.broker.itest.jms.selector;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import javax.jms.DeliveryMode;
import javax.jms.JMSException;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the message selector features of JMS
 */
public class SelectorTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that an empty string as a message selector indicates that there is no message selector for the message
	 * consumer.
	 */
	@Test
	public void testEmptyStringAsSelector() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "");
			receiverConnection.start();

			TextMessage message = senderSession.createTextMessage();
			message.setText("testEmptyStringAsSelector");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			assertEquals("testEmptyStringAsSelector", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Tats that String literals are well handled by the message selector. <br />
	 * <ul>
	 * <li><code>"string = 'literal''s;"</code> is <code>true</code> for "literal's" and <code>false</code> for
	 * "literal"</li>
	 * </ul>
	 */

	@Test
	public void testStringLiterals() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "string = 'literal''s'");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setStringProperty("string", "literal");
			dummyMessage.setText("testStringLiterals:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("string", "literal's");
			message.setText("testStringLiterals:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			assertEquals("testStringLiterals:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the JMS property <code>JMSDeliveryMode</code> is treated as having the values <code>'PERSISTENT'</code>
	 * or <code>'NON_PERSISTENT'</code> when used in a message selector (chapter 3.8.1.3).
	 */
	@Test
	public void testJMSDeliveryModeInSelector() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "JMSDeliveryMode = 'PERSISTENT'");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setText("testJMSDeliveryModeInSelector:1");
			// send a dummy message in *non persistent* mode
			sender.send(dummyMessage, DeliveryMode.NON_PERSISTENT, sender.getPriority(), sender.getTimeToLive());

			TextMessage message = senderSession.createTextMessage();
			message.setText("testJMSDeliveryModeInSelector:2");
			// send a message in *persistent*
			sender.send(message, DeliveryMode.PERSISTENT, sender.getPriority(), sender.getTimeToLive());

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			// only the message sent in persistent mode should be received.
			assertEquals(DeliveryMode.PERSISTENT, msg.getJMSDeliveryMode());
			assertEquals("testJMSDeliveryModeInSelector:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Test
	/**
	 * Test that conversions that apply to the <code>get</code> methods for properties do not apply when a property is
	 * used in a message selector expression. Based on the example of chapter 3.8.1.1 about identifiers.
	 */
	public void testIdentifierConversion() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "NumberOfOrders > 1");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setStringProperty("NumberOfOrders", "2");
			dummyMessage.setText("testIdentifierConversion:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setIntProperty("NumberOfOrders", 2);
			message.setText("testIdentifierConversion:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			assertEquals("testIdentifierConversion:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the message selector using the filter example provided by the JMS specifications. <br />
	 * <ul>
	 * <li><code>"JMSType = 'car' AND color = 'blue' AND weight > 2500"</code></li>
	 * </ul>
	 */
	@Test
	public void testSelectorExampleFromSpecs() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "JMSType = 'car' AND color = 'blue' AND weight > 2500");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setJMSType("car");
			dummyMessage.setStringProperty("color", "red");
			dummyMessage.setLongProperty("weight", 3000);
			dummyMessage.setText("testSelectorExampleFromSpecs:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setJMSType("car");
			message.setStringProperty("color", "blue");
			message.setLongProperty("weight", 3000);
			message.setText("testSelectorExampleFromSpecs:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			assertEquals("testSelectorExampleFromSpecs:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the ">" condition in message selector. <br />
	 * <ul>
	 * <li><code>"weight > 2500"</code> is <code>true</code> for 3000 and <code>false</code> for 1000</li>
	 * </ul>
	 */
	@Test
	public void testGreaterThan() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "weight > 2500");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setLongProperty("weight", 1000);
			dummyMessage.setText("testGreaterThan:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setLongProperty("weight", 3000);
			message.setText("testGreaterThan:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			assertEquals("testGreaterThan:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the "=" condition in message selector. <br />
	 * <ul>
	 * <li><code>"weight > 2500"</code> is <code>true</code> for 2500 and <code>false</code> for 1000</li>
	 * </ul>
	 */
	@Test
	public void testEquals() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "weight = 2500");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setLongProperty("weight", 1000);
			dummyMessage.setText("testEquals:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setLongProperty("weight", 2500);
			message.setText("testEquals:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			assertEquals("testEquals:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the "<>" (not equal) condition in message selector. <br />
	 * <ul>
	 * <li><code>"weight <> 2500"</code> is <code>true</code> for 1000 and <code>false</code> for 2500</li>
	 * </ul>
	 */
	@Test
	public void testNotEquals() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "weight <> 2500");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setLongProperty("weight", 2500);
			dummyMessage.setText("testEquals:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setLongProperty("weight", 1000);
			message.setText("testEquals:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("No message was received", msg != null);
			if (msg == null)
				return;
			assertEquals("testEquals:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the BETWEEN condition in message selector. <br />
	 * <ul>
	 * <li>"age BETWEEN 15 and 19" is <code>true</code> for 17 and <code>false</code> for 20</li>
	 * </ul>
	 */
	@Test
	public void testBetween() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "age BETWEEN 15 and 19");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setIntProperty("age", 20);
			dummyMessage.setText("testBetween:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setIntProperty("age", 17);
			message.setText("testBetween:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("Message not received", msg != null);
			if (msg == null)
				return;
			assertTrue("Message of another test: " + msg.getText(), msg.getText().startsWith("testBetween"));
			assertEquals("testBetween:2", msg.getText());

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the IN condition in message selector. <br />
	 * <ul>
	 * <li>"Country IN ('UK', 'US', 'France')" is <code>true</code> for 'UK' and <code>false</code> for 'Peru'</li>
	 * </ul>
	 */
	@Test
	public void testIn() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "Country IN ('UK', 'US', 'France')");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setStringProperty("Country", "Peru");
			dummyMessage.setText("testIn:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("Country", "UK");
			message.setText("testIn:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("Message not received", msg != null);
			if (msg == null)
				return;
			assertTrue("Message of another test: " + msg.getText(), msg.getText().startsWith("testIn"));
			assertEquals("testIn:2", msg.getText());

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the LIKE ... ESCAPE condition in message selector <br />
	 * <ul>
	 * <li>"underscored LIKE '\_%' ESCAPE '\'" is <code>true</code> for '_foo' and <code>false</code> for 'bar'</li>
	 * </ul>
	 */
	@Test
	public void testLikeEscape() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "underscored LIKE '\\_%' ESCAPE '\\'");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setStringProperty("underscored", "bar");
			dummyMessage.setText("testLikeEscape:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("underscored", "_foo");
			message.setText("testLikeEscape:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("Message not received", msg != null);
			if (msg == null)
				return;
			assertTrue("Message of another test: " + msg.getText(), msg.getText().startsWith("testLikeEscape"));
			assertEquals("testLikeEscape:2", msg.getText());

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the LIKE condition with '_' in the pattern. <br />
	 * <ul>
	 * <li>"word LIKE 'l_se'" is <code>true</code> for 'lose' and <code>false</code> for 'loose'</li>
	 * </ul>
	 */
	@Test
	public void testLike_2() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "word LIKE 'l_se'");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setStringProperty("word", "loose");
			dummyMessage.setText("testLike_2:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("word", "lose");
			message.setText("testLike_2:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("Message not received", msg != null);
			if (msg == null)
				return;
			assertTrue("Message of another test: " + msg.getText(), msg.getText().startsWith("testLike_2"));
			assertEquals("testLike_2:2", msg.getText());

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test the LIKE condition with '%' in the pattern. <br />
	 * <ul>
	 * <li>"phone LIKE '12%3'" is <code>true</code> for '12993' and <code>false</code> for '1234'</li>
	 * </ul>
	 */
	@Test
	public void testLike_1() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "phone LIKE '12%3'");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setStringProperty("phone", "1234");
			dummyMessage.setText("testLike_1:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("phone", "12993");
			message.setText("testLike_1:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("Message not received", msg != null);
			if (msg == null)
				return;
			assertTrue("Message of another test: " + msg.getText(), msg.getText().startsWith("testLike_1"));
			assertEquals("testLike_1:2", msg.getText());

		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Test
	/**
	 * Test the <code>NULL</code> value in message selector. <br />
	 * <ul>
	 * <li><code>"prop IS NULL"</code></li>
	 * </ul>
	 */
	public void testNull() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiverConnection.stop();

			receiver = receiverSession.createReceiver(receiverQueue, "prop_name IS NULL");
			receiverConnection.start();

			TextMessage dummyMessage = senderSession.createTextMessage();
			dummyMessage.setStringProperty("prop_name", "not null");
			dummyMessage.setText("testNull:1");
			sender.send(dummyMessage);

			TextMessage message = senderSession.createTextMessage();
			message.setText("testNull:2");
			sender.send(message);

			TextMessage msg = (TextMessage) receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue(msg != null);
			if (msg == null)
				return;
			assertEquals("testNull:2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}
}
