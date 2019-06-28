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

import javax.jms.ConnectionFactory;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Queue;
import javax.jms.QueueConnectionFactory;
import javax.jms.Topic;
import javax.jms.TopicConnectionFactory;

/**
 * Simple JMS provider interface. <br />
 * JMS Provider has to implement this simple interface to be able to use the test suite.
 */
public interface Provider {

	/**
	 * Returns the name of the JMS Provider.
	 *
	 * @return name of the JMS Provider
	 */
	public String getName();

	/**
	 * Creates a <code>ConnectionFactory</code>.
	 *
	 * @since JMS 1.1
	 * @param name
	 *            of the <code>ConnectionFactory</code>
	 */
	public ConnectionFactory createConnectionFactory(String name) throws ConnectException;

	/**
	 * Creates a <code>QueueConnectionFactory</code>.
	 *
	 * @param name
	 *            of the <code>QueueConnectionFactory</code>
	 */
	public QueueConnectionFactory createQueueConnectionFactory(String name) throws ConnectException;

	/**
	 * Creates a <code>TopicConnectionFactory</code>.
	 *
	 * @param name
	 *            of the <code>TopicConnectionFactory</code>
	 */
	public TopicConnectionFactory createTopicConnectionFactory(String name) throws ConnectException;

	/**
	 * Creates a <code>Queue</code>.
	 *
	 * @param name
	 *            of the <code>Queue</code>
	 */
	public Queue createQueue(String name) throws ConnectException, JMSException;

	/**
	 * Creates a <code>Topic</code>.
	 *
	 * @param name
	 *            of the <code>Topic</code>
	 */
	public Topic createTopic(String name) throws ConnectException, JMSException;

	/**
	 * Removes the <code>Queue</code>
	 *
	 * @param queue
	 */
	public void deleteQueue(Destination queue) throws ConnectException, JMSException;

	/**
	 * Removes the <code>Topic</code>
	 *
	 * @param topic
	 */
	public void deleteTopic(Destination topic) throws ConnectException, JMSException;

	/**
	 * Removes the <code>ConnectionFactory</code> of name <code>name</code> from JNDI and deletes it
	 *
	 * @since JMS 1.1
	 * @param name
	 *            JNDI name of the <code>ConnectionFactory</code>
	 */
	public void deleteConnectionFactory(String name);

	/**
	 * Removes the <code>QueueConnectionFactory</code> of name <code>name</code> from JNDI and deletes it
	 *
	 * @param name
	 *            JNDI name of the <code>QueueConnectionFactory</code>
	 */
	public void deleteQueueConnectionFactory(String name);

	/**
	 * Removes the <code>TopicConnectionFactory</code> of name <code>name</code> from JNDI and deletes it
	 *
	 * @param name
	 *            JNDI name of the <code>TopicConnectionFactory</code>
	 */
	public void deleteTopicConnectionFactory(String name);

	public void disconnect() throws JMSException;
}
