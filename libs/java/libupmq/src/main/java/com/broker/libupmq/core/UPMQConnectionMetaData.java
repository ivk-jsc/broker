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
 
package com.broker.libupmq.core;

import java.util.Arrays;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;

import javax.jms.ConnectionMetaData;
import javax.jms.JMSException;

import com.broker.protocol.Protocol;
import com.broker.protocol.Protocol.ClientVersion;
import com.broker.protocol.Protocol.ServerVersion;

public class UPMQConnectionMetaData implements ConnectionMetaData {

	private Protocol.ClientVersion _clientVersion;
	private Protocol.ServerVersion _serverVersion;

	private static final String JMSX_GROUP_ID = "JMSXGroupID";
	private static final String JMSX_GROUP_SEQ = "JMSXGroupSeq";
	private static final String JMSX_DELIVERY_COUNT = "JMSXDeliveryCount";
	private static final List<String> jmsxPropertyNames = Collections.unmodifiableList(Arrays.asList(new String[] { JMSX_GROUP_ID, JMSX_GROUP_SEQ, JMSX_DELIVERY_COUNT }));

	public UPMQConnectionMetaData(ClientVersion clientVersion, ServerVersion serverVersion) {
		_clientVersion = clientVersion;
		_serverVersion = serverVersion;
	}

	@Override
	public String getJMSVersion() throws JMSException {
		return getJMSMajorVersion() + "." + getJMSMinorVersion() + "." + _clientVersion.getClientRevisionVersion();
	}

	@Override
	public int getJMSMajorVersion() throws JMSException {
		return _clientVersion.getClientMajorVersion();
	}

	@Override
	public int getJMSMinorVersion() throws JMSException {
		return _clientVersion.getClientMinorVersion();
	}

	@Override
	public String getJMSProviderName() throws JMSException {
		//		return _serverVersion.getServerName();
		return null;
	}

	@Override
	public String getProviderVersion() throws JMSException {
		return getProviderMajorVersion() + "." + getProviderMinorVersion() + "." + _serverVersion.getServerRevisionVersion();
	}

	@Override
	public int getProviderMajorVersion() throws JMSException {
		return _serverVersion.getServerMajorVersion();
	}

	@Override
	public int getProviderMinorVersion() throws JMSException {
		return _serverVersion.getServerMinorVersion();
	}

	@Override
	@SuppressWarnings("rawtypes")
	public Enumeration getJMSXPropertyNames() throws JMSException {
		return Collections.enumeration(jmsxPropertyNames);
	}
}
