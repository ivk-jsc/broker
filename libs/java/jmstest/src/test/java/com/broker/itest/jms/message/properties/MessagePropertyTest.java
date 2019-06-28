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
import java.util.concurrent.atomic.AtomicBoolean;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageFormatException;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the <code>javax.jms.Message</code> properties. <br />
 * See JMS Specification, 3.5 Message Properties (p.32-37)
 */
public class MessagePropertyTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that any other class than <code>Boolean, Byte, Short, Integer, Long,
	 * Float, Double</code> and <code>String</code> used in the <code>Message.setObjectProperty()</code> method throws a
	 * <code>javax.jms.MessageFormatException</code>.
	 */
	@Test
	public void testSetObjectProperty_2() {
		try {
			Message message = senderSession.createMessage();
			message.setObjectProperty("prop", new AtomicBoolean(true));
			fail("3.5.5 An attempt to use any other class [than Boolean, Byte,...,String] must throw " + "a JMS MessageFormatException.\n");
		} catch (MessageFormatException e) {
		} catch (JMSException e) {
			fail("Should throw a javax.jms.MessageFormatException, not a " + e);
		}
	}

	/**
	 * if a property is set as a <code>Float</code> with the <code>Message.setObjectProperty()</code> method, it can be
	 * retrieve directly as a <code>double</code> by <code>Message.getFloatProperty()</code>
	 */
	@Test
	public void testSetObjectProperty_1() {
		try {
			Message message = senderSession.createMessage();
			message.setObjectProperty("pi", new Float(3.14159f));
			assertEquals(3.14159f, message.getFloatProperty("pi"), 0);
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a <code>null</code> value is returned by the <code>Message.getObjectProperty()</code> method if a
	 * property by the specified name does not exits.
	 */
	@Test
	public void testGetObjectProperty() {
		try {
			Message message = senderSession.createMessage();
			assertEquals("3.5.5 A null value is returned [by the getObjectProperty method] if a property by the specified " + "name does not exits.\n", null, message.getObjectProperty("prop"));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a <code>null</code> value is returned by the <code>Message.getStringProperty()</code> method if a
	 * property by the specified name does not exits.
	 */
	@Test
	public void testGetStringProperty() {
		try {
			Message message = senderSession.createMessage();
			assertEquals("3.5.5 A null value is returned [by the getStringProperty method] if a property by the specified " + "name does not exits.\n", null, message.getStringProperty("prop"));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that an attempt to get a <code>double</code> property which does not exist throw a
	 * <code>java.lang.NullPointerException</code>
	 */
	@Test
	public void testGetDoubleProperty() {
		try {
			Message message = senderSession.createMessage();
			message.getDoubleProperty("prop");
			fail("Should raise a NullPointerException.\n");
		} catch (NullPointerException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that an attempt to get a <code>float</code> property which does not exist throw a
	 * <code>java.lang.NullPointerException</code>
	 */
	@Test
	public void testGetFloatProperty() {
		try {
			Message message = senderSession.createMessage();
			message.getFloatProperty("prop");
			fail("Should raise a NullPointerException.\n");
		} catch (NullPointerException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that an attempt to get a <code>long</code> property which does not exist throw a
	 * <code>java.lang.NumberFormatException</code>
	 */
	@Test
	public void testGetLongProperty() {
		try {
			Message message = senderSession.createMessage();
			message.getLongProperty("prop");
			fail("Should raise a NumberFormatException.\n");
		} catch (NumberFormatException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that an attempt to get a <code>int</code> property which does not exist throw a
	 * <code>java.lang.NumberFormatException</code>
	 */
	@Test
	public void testGetIntProperty() {
		try {
			Message message = senderSession.createMessage();
			message.getIntProperty("prop");
			fail("Should raise a NumberFormatException.\n");
		} catch (NumberFormatException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that an attempt to get a <code>short</code> property which does not exist throw a
	 * <code>java.lang.NumberFormatException</code>
	 */
	@Test
	public void testGetShortProperty() {
		try {
			Message message = senderSession.createMessage();
			message.getShortProperty("prop");
			fail("Should raise a NumberFormatException.\n");
		} catch (NumberFormatException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that an attempt to get a <code>byte</code> property which does not exist throw a
	 * <code>java.lang.NumberFormatException</code>
	 */
	@Test
	public void testGetByteProperty() {
		try {
			Message message = senderSession.createMessage();
			message.getByteProperty("prop");
			fail("Should raise a NumberFormatException.\n");
		} catch (NumberFormatException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that an attempt to get a <code>boolean</code> property which does not exist returns <code>false</code>
	 */
	@Test
	public void testGetBooleanProperty() {
		try {
			Message message = senderSession.createMessage();
			assertEquals(false, message.getBooleanProperty("prop"));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>Message.getPropertyNames()</code> method does not return the name of the JMS standard header
	 * fields (e.g. <code>JMSCorrelationID</code>.
	 */
	@Test
	public void testGetPropertyNames() {
		try {
			Message message = senderSession.createMessage();
			message.setJMSCorrelationID("foo");
			Enumeration<?> e = message.getPropertyNames();
			assertTrue("3.5.6 The getPropertyNames method does not return the names of " + "the JMS standard header field [e.g. JMSCorrelationID].\n", !e.hasMoreElements());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>Message.getPropertyNames()</code> method returns an empty <code>java.util.Enumeration</code>
	 * if there is no properties. <br />
	 * If there are some, test that it properly return their names.
	 */
	@Test
	public void testPropertyIteration() {
		try {
			Message message = senderSession.createMessage();
			Enumeration<?> e = message.getPropertyNames();
			assertTrue("No property yet defined.\n", !e.hasMoreElements());
			message.setDoubleProperty("pi", 3.14159);
			e = message.getPropertyNames();
			assertEquals("One property defined of name 'pi'.\n", "pi", e.nextElement());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>Message.clearProperties()</code> method does not clear the value of the Message's body.
	 */
	@Test
	public void testClearProperties_2() {
		try {
			TextMessage message = senderSession.createTextMessage();
			message.setText("foo");
			message.clearProperties();
			assertEquals("3.5.7 Clearing a message's  property entries does not clear the value of its body.\n", "foo", message.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>Message.clearProperties()</code> method deletes all the properties of the Message.
	 */
	@Test
	public void testClearProperties_1() {
		try {
			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("prop", "foo");
			message.clearProperties();
			assertEquals("3.5.7 A message's properties are deleted by the clearProperties method.\n", null, message.getStringProperty("prop"));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}
}
