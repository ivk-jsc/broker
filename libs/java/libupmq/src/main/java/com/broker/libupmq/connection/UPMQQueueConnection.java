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
 
package com.broker.libupmq.connection;

import javax.jms.JMSException;
import javax.jms.QueueConnection;
import javax.jms.QueueSession;

import com.broker.libupmq.session.UPMQQueueSession;

public class UPMQQueueConnection extends UPMQConnection implements QueueConnection {

	public UPMQQueueConnection(String providerURI, String userName, String password) throws JMSException {
		super(providerURI, userName, password);
	}

	@Override
	public QueueSession createQueueSession(boolean transacted, int acknowledgeMode) throws JMSException {
		synchronized (lock) {
			checkClosed();
			final UPMQQueueSession session = new UPMQQueueSession(this, transacted, acknowledgeMode);
			addSession(session);
			return session;
		}
	}
}
