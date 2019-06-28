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
 
package com.broker.itest.testcase;

import java.security.InvalidParameterException;

import javax.jms.Queue;
import javax.jms.QueueConnection;
import javax.jms.QueueConnectionFactory;
import javax.jms.QueueReceiver;
import javax.jms.QueueSender;
import javax.jms.QueueSession;
import javax.jms.Session;

import org.junit.After;
import org.junit.Before;

import com.broker.itest.provider.Provider;
import com.broker.itest.provider.ProviderLoader;

public abstract class QueueConnectionTestCase extends ProviderLoader {

	private Provider admin;
	private String queueName;

	protected Queue senderQueue;
	protected QueueSender sender;
	protected QueueConnectionFactory senderQCF;
	protected QueueConnection senderConnection;
	protected QueueSession senderSession;

	protected Queue receiverQueue;
	protected QueueReceiver receiver;
	protected QueueConnectionFactory receiverQCF;
	protected QueueConnection receiverConnection;
	protected QueueSession receiverSession;

	protected abstract String getTestName();

	@Before
	public void setUp() {
		try {

			if ((queueName = getTestName()) == null) {
				throw new InvalidParameterException("test name is null");
			}
			if ((admin = getProvider()) == null) {
				throw new InvalidParameterException("jms provider is null");
			}

			if (senderQCF == null)
				senderQCF = admin.createQueueConnectionFactory(qcfName);
			if (senderQueue == null)
				senderQueue = admin.createQueue(queueName);

			senderConnection = senderQCF.createQueueConnection();
			senderSession = senderConnection.createQueueSession(false, Session.AUTO_ACKNOWLEDGE);
			sender = senderSession.createSender(senderQueue);

			if (receiverQCF == null)
				receiverQCF = admin.createQueueConnectionFactory(qcfName);
			if (receiverQueue == null)
				receiverQueue = admin.createQueue(queueName);

			receiverConnection = receiverQCF.createQueueConnection();
			receiverSession = receiverConnection.createQueueSession(false, Session.AUTO_ACKNOWLEDGE);
			receiver = receiverSession.createReceiver(receiverQueue);

			senderConnection.start();
			receiverConnection.start();

		} catch (Exception e) {
			// XXX
			e.printStackTrace();
		}
	}

	@After
	public void tearDown() {
		try {
			if (senderConnection != null)
				senderConnection.close();

			if (receiverConnection != null)
				receiverConnection.close();

			if (admin != null) {
				admin.deleteQueueConnectionFactory(qcfName);
				admin.deleteQueue(senderQueue);
				admin.disconnect();
				admin = null;
			}
		} catch (Exception e) {
			// XXX 
			e.printStackTrace();
		} finally {
			senderQueue = null;
			sender = null;
			senderQCF = null;
			senderSession = null;
			senderConnection = null;

			receiverQueue = null;
			receiver = null;
			receiverQCF = null;
			receiverSession = null;
			receiverConnection = null;
		}
	}
}
