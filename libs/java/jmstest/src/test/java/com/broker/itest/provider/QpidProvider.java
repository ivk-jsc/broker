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
 
package com.broker.itest.provider;

import java.net.ConnectException;
import java.net.URISyntaxException;
import java.util.HashMap;

import javax.jms.ConnectionFactory;
import javax.jms.JMSException;
import javax.jms.Queue;
import javax.jms.QueueConnection;
import javax.jms.QueueConnectionFactory;
import javax.jms.QueueSession;
import javax.jms.Topic;
import javax.jms.TopicConnection;
import javax.jms.TopicConnectionFactory;
import javax.jms.TopicSession;

import org.apache.qpid.client.AMQAnyDestination;
import org.apache.qpid.client.AMQConnectionFactory;
import org.apache.qpid.url.URLSyntaxException;

public class QpidProvider implements Provider {
	private String name = "qpid";
	private HashMap<String, Object> _factoryMap = new HashMap<String, Object>();
	private HashMap<String, Object> _queueMap = new HashMap<String, Object>();
	private HashMap<String, Object> _topicMap = new HashMap<String, Object>();
	private QueueConnection _queueConnection = null;
	private QueueSession _queueSession = null;
	private TopicConnection _topicConnection = null;
	private TopicSession _topicSession = null;

	public QpidProvider() {

	}

	@Override
	public String getName() {
		return name;
	}

	@Override
	public ConnectionFactory createConnectionFactory(String name) throws ConnectException {
		Object result = _factoryMap.get(name);
		if (result == null) {
			try {
				result = new AMQConnectionFactory("amqp://guest:guest@clientid/?brokerlist='tcp://localhost:5672'");//context.lookup("qpidConnectionFactory");
				_factoryMap.put(name, result);
			} catch (URLSyntaxException e) {
				throw new ConnectException(e.getMessage());
			}
		}
		return (ConnectionFactory) result;
	}

	@Override
	public QueueConnectionFactory createQueueConnectionFactory(String name) throws ConnectException {
		Object result = _factoryMap.get(name);
		if (result == null) {
			try {
				result = new AMQConnectionFactory("amqp://guest:guest@clientid/?brokerlist='tcp://localhost:5672'");
				_factoryMap.put(name, result);
			} catch (URLSyntaxException e) {
				throw new ConnectException(e.getMessage());
			}
		}
		return (QueueConnectionFactory) result;
	}

	@Override
	public TopicConnectionFactory createTopicConnectionFactory(String name) throws ConnectException {
		Object result = _factoryMap.get(name);
		if (result == null) {
			try {
				result = new AMQConnectionFactory("amqp://guest:guest@clientid/?brokerlist='tcp://localhost:5672'");
				_factoryMap.put(name, result);
			} catch (URLSyntaxException e) {
				throw new ConnectException(e.getMessage());
			}
		}
		return (TopicConnectionFactory) result;
	}

	@Override
	public Queue createQueue(String name) throws ConnectException, JMSException {

		Object result = _queueMap.get(name);
		if (result == null) {

			try {
				result = new AMQAnyDestination(name + "; {create:always, node:{ type: queue }}");//, delete:always
				_queueMap.put(name, result);
			} catch (URISyntaxException e) {
				throw new ConnectException(e.getMessage());
			}
		}
		return (Queue) result;
	}

	@Override
	public Topic createTopic(String name) throws ConnectException, JMSException {

		Object result = _topicMap.get(name);
		if (result == null) {
			try {
				result = new AMQAnyDestination(name + "; {create:always, node:{ type: topic }}");//, delete:always
				_topicMap.put(name, result);
			} catch (URISyntaxException e) {
				throw new ConnectException(e.getMessage());
			}
		}
		return (Topic) result;
	}

	@Override
	public void deleteQueue(javax.jms.Destination queue) throws ConnectException, JMSException {
		_queueMap.remove(queue);
	}

	@Override
	public void deleteTopic(javax.jms.Destination topic) throws ConnectException, JMSException {
		_topicMap.remove(topic);
	}

	@Override
	public void deleteConnectionFactory(String name) {
		_factoryMap.remove(name);
	}

	@Override
	public void deleteTopicConnectionFactory(String name) {
		_factoryMap.remove(name);
	}

	@Override
	public void deleteQueueConnectionFactory(String name) {
		_factoryMap.remove(name);
	}

	@Override
	public void disconnect() throws JMSException {
		if (_queueConnection != null) {
			_queueConnection.close();
			_queueConnection = null;
		}
		if (_topicConnection != null) {
			_topicConnection.close();
			_topicConnection = null;
		}
	}
}
