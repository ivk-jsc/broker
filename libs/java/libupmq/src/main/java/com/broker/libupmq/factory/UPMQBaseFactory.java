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

import java.net.URI;
import java.util.Properties;

import javax.naming.Context;

public class UPMQBaseFactory {

	public static final String DEFAULT_BROKER_URL = "tcp://localhost:12345";
	public static final String DEFAULT_USER = null;
	public static final String DEFAULT_PASSWORD = null;

	String _brokerURI = null;
	String _userName = null;
	String _password = null;

	private Properties _properties;

	public UPMQBaseFactory() {
		setBrokerURI(DEFAULT_BROKER_URL);
		setUserName(DEFAULT_USER);
		setPassword(DEFAULT_PASSWORD);
	}

	public UPMQBaseFactory(String brokerURI) {
		setBrokerURI(brokerURI);
		setUserName(DEFAULT_USER);
		setPassword(DEFAULT_PASSWORD);
	}

	public UPMQBaseFactory(URI brokerURI) {
		setBrokerURI(brokerURI.toString());
		setUserName(DEFAULT_USER);
		setPassword(DEFAULT_PASSWORD);
	}

	public UPMQBaseFactory(String userName, String password, String brokerURI) {
		setBrokerURI(brokerURI);
		setUserName(userName);
		setPassword(password);
	}

	public UPMQBaseFactory(String userName, String password, URI brokerURI) {
		setBrokerURI(brokerURI.toString());
		setUserName(userName);
		setPassword(password);

	}

	public void setUserName(String userName) {
		_userName = userName;
	}

	public String getUserName() {
		return _userName;
	}

	public void setPassword(String password) {
		_password = password;
	}

	public String getPassword() {
		return _password;
	}

	public void setBrokerURI(String brokerURI) {
		_brokerURI = brokerURI;
	}

	public String getBrokerURI() {
		return _brokerURI;
	}

	public void setProperties(Properties properties) {
		_properties = properties;

		String temp = properties.getProperty(Context.PROVIDER_URL);
		if (temp != null && temp.length() > 0) {
			setBrokerURI(temp);
		}
	}

	public Properties getProperties() {
		return _properties;
	}
}
