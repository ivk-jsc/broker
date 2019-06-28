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

import org.apache.activemq.ActiveMQConnection;
import org.apache.activemq.ActiveMQConnectionFactory;
import org.apache.activemq.command.ActiveMQDestination;

public class ActiveMQProvider implements Provider {
	private String name = "activemq";
	private HashMap<String, Object> _factoryMap = new HashMap<String, Object>();
	private HashMap<String, Object> _queueMap = new HashMap<String, Object>();
	private HashMap<String, Object> _topicMap = new HashMap<String, Object>();
	private QueueConnection _queueConnection = null;
	private QueueSession _queueSession = null;
	private TopicConnection _topicConnection = null;
	private TopicSession _topicSession = null;

	public ActiveMQProvider() {

	}

	@Override
	public String getName() {
		return name;
	}

	@Override
	public ConnectionFactory createConnectionFactory(String name) throws ConnectException {
		Object result = _factoryMap.get(name);
		if (result == null) {
			result = new ActiveMQConnectionFactory();
			_factoryMap.put(name, result);
		}
		return (ConnectionFactory) result;
	}

	@Override
	public QueueConnectionFactory createQueueConnectionFactory(String name) throws ConnectException {
		Object result = _factoryMap.get(name);
		if (result == null) {
			result = new ActiveMQConnectionFactory();
			_factoryMap.put(name, result);
		}
		return (QueueConnectionFactory) result;
	}

	@Override
	public TopicConnectionFactory createTopicConnectionFactory(String name) throws ConnectException {
		Object result = _factoryMap.get(name);
		if (result == null) {
			result = new ActiveMQConnectionFactory();
			_factoryMap.put(name, result);
		}
		return (TopicConnectionFactory) result;
	}

	@Override
	public Queue createQueue(String name) throws ConnectException, JMSException {

		Object result = _queueMap.get(name);
		if (result == null) {
			if (_queueSession == null) {
				QueueConnectionFactory factory = new ActiveMQConnectionFactory();
				_queueConnection = factory.createQueueConnection();
				_queueSession = _queueConnection.createQueueSession(false, QueueSession.AUTO_ACKNOWLEDGE);
			}
			result = _queueSession.createQueue(name);
			_queueMap.put(name, result);
		}
		return (Queue) result;
	}

	@Override
	public Topic createTopic(String name) throws ConnectException, JMSException {

		Object result = _topicMap.get(name);
		if (result == null) {
			if (_topicSession == null) {
				TopicConnectionFactory factory = new ActiveMQConnectionFactory();
				_topicConnection = factory.createTopicConnection();
				_topicSession = _topicConnection.createTopicSession(false, QueueSession.AUTO_ACKNOWLEDGE);
			}
			result = _topicSession.createTopic(name);
			_queueMap.put(name, result);
		}
		return (Topic) result;
	}

	@Override
	public void deleteQueue(javax.jms.Destination queue) throws ConnectException, JMSException {
		ActiveMQDestination destImpl = (ActiveMQDestination) queue;
		_queueMap.remove(destImpl);

		if (_queueConnection != null) {
			ActiveMQConnection conn = (ActiveMQConnection) _queueConnection;
			conn.destroyDestination((ActiveMQDestination) queue);
		}
	}

	@Override
	public void deleteTopic(javax.jms.Destination topic) throws ConnectException, JMSException {
		ActiveMQDestination destImpl = (ActiveMQDestination) topic;
		_topicMap.remove(destImpl);

		if (_topicConnection != null) {
			ActiveMQConnection conn = (ActiveMQConnection) _topicConnection;
			conn.destroyDestination((ActiveMQDestination) topic);
		}
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
