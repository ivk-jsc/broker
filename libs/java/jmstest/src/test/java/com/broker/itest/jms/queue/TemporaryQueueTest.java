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

import javax.jms.QueueReceiver;
import javax.jms.TemporaryQueue;

import org.junit.Rule;
import org.junit.rules.TestName;

import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the <code>javax.jms.TemporaryQueue</code> features.
 */
public class TemporaryQueueTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	private TemporaryQueue tempQueue;
	private QueueReceiver tempReceiver;

	/**
	 * Test a TemporaryQueue
	 */
	//	public void testTemporaryQueue() {
	//		try {
	//			// we stop both sender and receiver connections
	//			senderConnection.stop();
	//			receiverConnection.stop();
	//			// we create a temporary queue to receive messages
	//			tempQueue = receiverSession.createTemporaryQueue();
	//			// we recreate the sender because it has been
	//			// already created with a Destination as parameter
	//			sender = senderSession.createSender(null);
	//			// we create a receiver on the temporary queue
	//			tempReceiver = receiverSession.createReceiver(tempQueue);
	//			receiverConnection.start();
	//			senderConnection.start();
	//
	//			TextMessage message = senderSession.createTextMessage();
	//			message.setText("testTemporaryQueue");
	//			sender.send(tempQueue, message);
	//
	//			Message m = tempReceiver.receive(TestConfig.TIMEOUT);
	//			assertTrue("No message was received", m != null);
	//			if (m == null)
	//				return;
	//			assertTrue(m instanceof TextMessage);
	//			TextMessage msg = (TextMessage) m;
	//			assertEquals("testTemporaryQueue", msg.getText());
	//		} catch (JMSException e) {
	//			fail(e);
	//		}
	//	}

}
