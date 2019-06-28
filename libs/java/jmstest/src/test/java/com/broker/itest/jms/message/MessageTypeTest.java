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
 
package com.broker.itest.jms.message;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Enumeration;
import java.util.Vector;

import javax.jms.BytesMessage;
import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.ObjectMessage;
import javax.jms.StreamMessage;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the different types of messages provided by JMS. <br />
 * JMS provides 6 types of messages which differs by the type of their body:
 * <ol>
 * <li><code>Message</code> which doesn't have a body</li>
 * <li><code>TextMessage</code> with a <code>String</code> as body</li>
 * <li><code>ObjectMessage</code> with any <code>Object</code> as body</li>
 * <li><code>BytesMessage</code> with a body made of <code>bytes</code></li>
 * <li><code>MapMessage</code> with name-value pairs of Java primitives in its body</li>
 * <li><code>StreamMessage</code> with a stream of Java primitives as body</li>
 * </ol>
 * <br />
 * For each of this type of message, we test that a message can be sent and received with an empty body or not.
 */
public class MessageTypeTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Send a <code>StreamMessage</code> with 2 Java primitives in its body (a <code>
	 * String</code> and a <code>double</code>). <br />
	 * Receive it and test that the values of the primitives of the body are correct
	 */
	@Test
	public void testStreamMessage_2() {
		try {
			StreamMessage message = senderSession.createStreamMessage();
			message.writeString("pi");
			message.writeDouble(3.14159);
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of StreamMessage.\n", m instanceof StreamMessage);
			StreamMessage msg = (StreamMessage) m;
			assertEquals("pi", msg.readString());
			assertEquals(3.14159, msg.readDouble(), 0);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>StreamMessage</code> with an empty body. <br />
	 * Receive it and test if the message is effectively an instance of <code>StreamMessage</code>
	 */
	@Test
	public void testStreamMessage_1() {
		try {
			StreamMessage message = senderSession.createStreamMessage();
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of StreamMessage.\n", msg instanceof StreamMessage);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test in MapMessage the conversion between <code>getObject("foo")</code> and <code>getDouble("foo")</code> (the
	 * later returning a java.lang.Double and the former a double)
	 */
	@Test
	public void testMapMessageConversion() {
		try {
			MapMessage message = senderSession.createMapMessage();
			message.setDouble("pi", 3.14159);
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of MapMessage.\n", m instanceof MapMessage);
			MapMessage msg = (MapMessage) m;
			assertTrue(msg.getObject("pi") instanceof Double);
			assertEquals(3.14159, ((Double) msg.getObject("pi")).doubleValue(), 0);
			assertEquals(3.14159, msg.getDouble("pi"), 0);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the if the name parameter of the set methods of a <code>MapMessage</code> is <code>null</code>, the
	 * method must throw the error <code>java.lang.IllegalArgumentException</code>. <br />
	 * 
	 * @since JMS 1.1
	 */
	@Test
	public void testNullInSetMethodsForMapMessage() {
		try {
			MapMessage message = senderSession.createMapMessage();
			message.setBoolean(null, true);
			fail("Should throw an IllegalArgumentException");
		} catch (IllegalArgumentException e) {
		} catch (JMSException e) {
			fail("Should throw an IllegalArgumentException, not a" + e);
		}
	}

	/**
	 * Test that the if the name parameter of the set methods of a <code>MapMessage</code> is an empty String, the
	 * method must throw the error <code>java.lang.IllegalArgumentException</code>. <br />
	 * 
	 * @since JMS 1.1
	 */
	@Test
	public void testEmptyStringInSetMethodsForMapMessage() {
		try {
			MapMessage message = senderSession.createMapMessage();
			message.setBoolean("", true);
			fail("Should throw an IllegalArgumentException");
		} catch (IllegalArgumentException e) {
		} catch (JMSException e) {
			fail("Should throw an IllegalArgumentException, not a" + e);
		}
	}

	/**
	 * Test that the <code>MapMessage.getMapNames()</code> method returns an empty <code>Enumeration</code> when no map
	 * has been defined before. <br />
	 * Also test that the same method returns the correct names of the map.
	 */
	@Test
	public void testgetMapNames() {
		try {
			MapMessage message = senderSession.createMapMessage();
			Enumeration<?> e = message.getMapNames();
			assertTrue("No map yet defined.\n", !e.hasMoreElements());
			message.setDouble("pi", 3.14159);
			e = message.getMapNames();
			assertEquals("pi", (e.nextElement()));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>MapMessage</code> with 2 Java primitives in its body (a <code>
	 * String</code> and a <code>double</code>). <br />
	 * Receive it and test that the values of the primitives of the body are correct
	 */
	@Test
	public void testMapMessage_2() {
		try {
			MapMessage message = senderSession.createMapMessage();
			message.setString("name", "pi");
			message.setDouble("value", 3.14159);
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of MapMessage.\n", m instanceof MapMessage);
			MapMessage msg = (MapMessage) m;
			assertEquals("pi", msg.getString("name"));
			assertEquals(3.14159, msg.getDouble("value"), 0);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>MapMessage</code> with an empty body. <br />
	 * Receive it and test if the message is effectively an instance of <code>MapMessage</code>
	 */
	@Test
	public void testMapMessage_1() {
		try {
			MapMessage message = senderSession.createMapMessage();
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of MapMessage.\n", msg instanceof MapMessage);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send an <code>ObjectMessage</code> with a <code>Vector</code> (composed of a <code>
	 * String</code> and a <code>double</code>) in its body. <br />
	 * Receive it and test that the values of the primitives of the body are correct
	 */
	@Test
	public void testObjectMessage_2() {
		try {
			@SuppressWarnings("rawtypes")
			Vector<Comparable> vector = new Vector<Comparable>();
			vector.add("pi");
			vector.add(new Double(3.14159));

			ObjectMessage message = senderSession.createObjectMessage();
			message.setObject(vector);
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of ObjectMessage.\n", m instanceof ObjectMessage);
			ObjectMessage msg = (ObjectMessage) m;
			assertEquals(vector, msg.getObject());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>ObjectMessage</code> with an empty body. <br />
	 * Receive it and test if the message is effectively an instance of <code>ObjectMessage</code>
	 */
	@Test
	public void testObjectMessage_1() {
		try {
			ObjectMessage message = senderSession.createObjectMessage();
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of ObjectMessage.\n", msg instanceof ObjectMessage);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>BytesMessage</code> with 2 Java primitives in its body (a <code>
	 * String</code> and a <code>double</code>). <br />
	 * Receive it and test that the values of the primitives of the body are correct
	 */
	@Test
	public void testBytesMessage_2() {
		try {
			byte[] bytes = new String("pi").getBytes();
			BytesMessage message = senderSession.createBytesMessage();
			message.writeBytes(bytes);
			message.writeDouble(3.14159);
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of BytesMessage.\n", m instanceof BytesMessage);
			BytesMessage msg = (BytesMessage) m;
			byte[] receivedBytes = new byte[bytes.length];
			msg.readBytes(receivedBytes);
			assertEquals(new String(bytes), new String(receivedBytes));
			assertEquals(3.14159, msg.readDouble(), 0);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>BytesMessage</code> with an empty body. <br />
	 * Receive it and test if the message is effectively an instance of <code>BytesMessage</code>
	 */
	@Test
	public void testBytesMessage_1() {
		try {
			BytesMessage message = senderSession.createBytesMessage();
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of BytesMessage.\n", msg instanceof BytesMessage);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>TextMessage</code> with a <code>String</code> in its body. <br />
	 * Receive it and test that the received <code>String</code> corresponds to the sent one.
	 */
	@Test
	public void testTextMessage_2() {
		try {
			TextMessage message = senderSession.createTextMessage();
			message.setText("testTextMessage_2");
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of TextMessage.\n", m instanceof TextMessage);
			TextMessage msg = (TextMessage) m;
			assertEquals("testTextMessage_2", msg.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Send a <code>TextMessage</code> with an empty body. <br />
	 * Receive it and test if the message is effectively an instance of <code>TextMessage</code>
	 */
	@Test
	public void testTextMessage_1() {
		try {
			TextMessage message = senderSession.createTextMessage();
			sender.send(message);

			Message msg = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of TextMessage.\n", msg instanceof TextMessage);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}
}
