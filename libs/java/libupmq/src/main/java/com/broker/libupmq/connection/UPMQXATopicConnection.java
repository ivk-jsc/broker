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
import javax.jms.Session;
import javax.jms.XASession;
import javax.jms.XATopicConnection;
import javax.jms.XATopicSession;

import com.broker.libupmq.session.UPMQXASession;
import com.broker.libupmq.session.UPMQXATopicSession;

public class UPMQXATopicConnection extends UPMQTopicConnection implements XATopicConnection {

	public UPMQXATopicConnection(String providerURI, String userName, String password) throws JMSException {
		super(providerURI, userName, password);
	}

	@Override
	public XATopicSession createXATopicSession() throws JMSException {
		synchronized (lock) {
			checkClosed();
			final UPMQXATopicSession session = new UPMQXATopicSession(this, true, Session.SESSION_TRANSACTED);
			addSession(session);
			return session;
		}
	}

	@Override
	public XASession createXASession() throws JMSException {
		synchronized (lock) {
			checkClosed();
			final UPMQXASession session = new UPMQXASession(this, true, Session.SESSION_TRANSACTED);
			addSession(session);
			return session;
		}
	}
}
