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
 
package com.broker.libupmq.factory;

import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.JMSException;

import com.broker.libupmq.connection.UPMQConnection;

public class UPMQConnectionFactory extends UPMQBaseFactory implements ConnectionFactory {

	@Override
	public Connection createConnection() throws JMSException {
		return new UPMQConnection(getBrokerURI(), getUserName(), getPassword());
	}

	@Override
	public Connection createConnection(String userName, String password) throws JMSException {
		setUserName(userName);
		setPassword(password);
		return createConnection();
	}
}
