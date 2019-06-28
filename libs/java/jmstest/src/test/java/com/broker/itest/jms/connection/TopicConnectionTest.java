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

import javax.jms.InvalidClientIDException;
import javax.jms.JMSException;
import javax.jms.TopicConnection;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.testcase.TopicConnectionTestCase;

/**
 * Test topic-specific connection features.
 * 
 * Test setting of client ID which is relevant only for Durable Subscribtion
 */

public class TopicConnectionTest extends TopicConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that a call to <code>setClientID</code> will throw an <code>IllegalStateException</code> if a client ID has
	 * already been set see JMS javadoc
	 * http://java.sun.com/j2ee/sdk_1.3/techdocs/api/javax/jms/Connection.html#setClientID(java.lang.String)
	 */
	@Test
	public void testSetClientID_1() {
		try {
			// we start from a clean state for the connection
			subscriberConnection.close();
			subscriberConnection = null;

			subscriberConnection = subscriberTCF.createTopicConnection();
			// if the JMS provider does not set a client ID, we do.
			if (subscriberConnection.getClientID() == null) {
				subscriberConnection.setClientID("testSetClientID_1");
				assertEquals("testSetClientID_1", subscriberConnection.getClientID());
			}
			// now the connection has a client ID (either "testSetClientID_1" or one
			// set by the provider
			assertTrue(subscriberConnection.getClientID() != null);

			// a attempt to set a client ID should now throw an IllegalStateException
			subscriberConnection.setClientID("another client ID");
			fail("Should raise a javax.jms.IllegalStateException");
		} catch (javax.jms.IllegalStateException e) {
		} catch (JMSException e) {
			fail("Should raise a javax.jms.IllegalStateException, not a " + e);
		} catch (java.lang.IllegalStateException e) {
			fail("Should raise a javax.jms.IllegalStateException, not a java.lang.IllegalStateException");
		}
	}

	/**
	 * Test that a call to <code>setClientID</code> can occur only after connection creation and before any other action
	 * on the connection. <em>This test is relevant only if the ID is set by the JMS client</em> see JMS javadoc
	 * http://java.sun.com/j2ee/sdk_1.3/techdocs/api/javax/jms/Connection.html#setClientID(java.lang.String)
	 */
	@Test
	public void testSetClientID_2() {
		try {
			// we start from a clean state for the first connection
			subscriberConnection.close();
			subscriberConnection = null;

			subscriberConnection = subscriberTCF.createTopicConnection();
			// if the JMS provider has set a client ID, this test is not relevant
			if (subscriberConnection.getClientID() != null) {
				return;
			}

			// we start the connection
			subscriberConnection.start();

			// an attempt to set the client ID now should throw a
			// IllegalStateException
			subscriberConnection.setClientID("testSetClientID_2");
			fail("Should throw a javax.jms.IllegalStateException");
		} catch (javax.jms.IllegalStateException e) {
		} catch (JMSException e) {
			fail("Should raise a javax.jms.IllegalStateException, not a " + e);
		} catch (java.lang.IllegalStateException e) {
			fail("Should raise a javax.jms.IllegalStateException, not a java.lang.IllegalStateException");
		}
	}

	/**
	 * Test that if another connection with the same clientID is already running when <code>setClientID</code> is
	 * called, the JMS provider should detect the duplicate ID and throw an <code>InvalidClientIDException</code>
	 * <em>This test is relevant only if the ID is set by the JMS client</em> see JMS javadoc
	 * http://java.sun.com/j2ee/sdk_1.3/techdocs/api/javax/jms/Connection.html#setClientID(java.lang.String)
	 */
	@Test
	public void testSetClientID_3() {
		TopicConnection connection_2 = null;
		try {
			// we start from a clean state for the first connection
			subscriberConnection.close();
			subscriberConnection = null;

			subscriberConnection = subscriberTCF.createTopicConnection();
			// if the JMS provider has set a client ID, this test is not relevant
			if (subscriberConnection.getClientID() != null) {
				return;
			}
			// the JMS provider has not set a client ID, so we do
			subscriberConnection.setClientID("testSetClientID_3");
			assertEquals("testSetClientID_3", subscriberConnection.getClientID());

			// we create a new connection and try to set the same ID than for
			// subscriberConnection
			connection_2 = subscriberTCF.createTopicConnection();
			assertTrue(connection_2.getClientID() == null);
			connection_2.setClientID("testSetClientID_3");
			fail("Should throw a javax.jms.InvalidClientIDException");
		} catch (InvalidClientIDException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		} finally {
			if (connection_2 != null)
				try {
					connection_2.close();
				} catch (JMSException e1) {
				}
		}
	}
}
