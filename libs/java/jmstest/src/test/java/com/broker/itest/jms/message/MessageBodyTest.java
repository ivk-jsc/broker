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

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageNotWriteableException;
import javax.jms.ObjectMessage;
import javax.jms.TextMessage;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.provider.ProviderLoader;
import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Tests on message body.
 */
public class MessageBodyTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that the <code>TextMessage.clearBody()</code> method does nto clear the message properties.
	 */
	@Test
	public void testClearBody_2() {
		try {
			TextMessage message = senderSession.createTextMessage();
			message.setStringProperty("prop", "foo");
			message.clearBody();
			assertEquals("3.11.1 Clearing a message's body does not clear its property entries.\n", "foo", message.getStringProperty("prop"));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that the <code>TextMessage.clearBody()</code> effectively clear the body of the message
	 */
	@Test
	public void testClearBody_1() {
		try {
			TextMessage message = senderSession.createTextMessage();
			message.setText("bar");
			message.clearBody();
			assertEquals("3 .11.1 the clearBody method of Message resets the value of the message body " + "to the 'empty' initial message value as set by the message type's create "
					+ "method provided by Session.\n", null, message.getText());
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a call to the <code>TextMessage.setText()</code> method on a received message raises a
	 * <code>javax.jms.MessageNotWriteableException</code>.
	 */
	@Test
	public void testWriteOnReceivedBody() {
		try {
			TextMessage message = senderSession.createTextMessage();
			message.setText("foo");
			sender.send(message);

			Message m = receiver.receive(ProviderLoader.TIMEOUT);
			assertTrue("The message should be an instance of TextMessage.\n", m instanceof TextMessage);
			TextMessage msg = (TextMessage) m;
			msg.setText("bar");
			fail("should raise a MessageNotWriteableException (3.11.2)");
		} catch (MessageNotWriteableException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a call to the <code>TextMessage.getBody()</code> method on a returns same value as previously setted
	 * <code>javax.jms.jmsException</code>.
	 */
	@Test
	public void testGetTextMessageBody() {
		try {
			String content = "abc";
			TextMessage message = senderSession.createTextMessage();
			message.setText(content);
			String result = message.getText();//message.getBody(String.class);
			assertTrue(" Text Message body is the same ", content.equals(result));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a call to the <code>ObjectMessage.getBody()</code> method on a returns same value as previously setted
	 * <code>javax.jms.jmsException</code>.
	 */
	@Test
	public void testGetObjectMessageBody() {
		try {
			String content = "checking getBody()";
			Exception excep = new Exception(content);
			ObjectMessage message = senderSession.createObjectMessage();
			message.setObject(excep);
			java.io.Serializable tmp = message.getObject();//message.getBody(java.io.Serializable.class);
			assertTrue(" Instance retrieved is the same as expected ", tmp instanceof Exception);
			Exception result = (Exception) tmp;
			assertTrue(" Object Message body  content is the same as expected ", result.getMessage().equals(content));
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that a call to the <code>MapMessage.getBody()</code> method on a returns same value as previously setted
	 * <code>javax.jms.jmsException</code>.
	 */
	@Test
	public void testGetMapMessageBody() {
		//TODO: JMS 2.0
		//		try {
		//			String content = "checking getBody()";
		//			MapMessage message = senderSession.createMapMessage();
		//			message.setObject("except", new Integer(12));
		//			Map tmp = message.getBody(java.util.Map.class);
		//			assertTrue(" Instance retrieved is the same as expected ", tmp.get("except") instanceof Integer);
		//			Integer result = (Integer) tmp.get("except");
		//			assertTrue(" Map Message body  content is the same as expected ", result.intValue() == 12);
		//		} catch (JMSException e) {
		//			fail(e);
		//		}
	}

	/**
	 * Test that a call to the <code>BytesMessage.getBody()</code> method on a returns same value as previously setted
	 * <code>javax.jms.jmsException</code>.
	 */
	@Test
	public void testGetBytesMessageBody() {
		//TODO: JMS 2.0
		//		try {
		//			byte[] content = new byte[20];
		//			content[3] = 12;
		//			BytesMessage message = senderSession.createBytesMessage();
		//			message.writeBytes(content);
		//			message.reset();
		//			byte[] result = message.getBody(byte[].class);
		//			assertTrue(" get body result is not null ", result != null);
		//			assertTrue(" Bytes Message body  content is the same as expected ", result[3] == 12);
		//		} catch (JMSException e) {
		//			fail(e);
		//		}
	}
}
