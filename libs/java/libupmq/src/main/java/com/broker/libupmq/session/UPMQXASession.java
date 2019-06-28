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
 
package com.broker.libupmq.session;

import javax.jms.JMSException;
import javax.jms.Session;
import javax.jms.XASession;
import javax.transaction.xa.XAResource;

import com.broker.libupmq.connection.UPMQConnection;

public class UPMQXASession extends UPMQSession implements XASession {

	public UPMQXASession(UPMQConnection connectionImpl, boolean transacted, int acknowledgeMode) throws JMSException {
		super(connectionImpl, transacted, acknowledgeMode);
	}

	@Override
	public Session getSession() throws JMSException {
		return this;
	}

	@Override
	public XAResource getXAResource() {
		// TODO not implemented
		throw new UnsupportedOperationException("Not supported yet.");
	}
}
