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
 
package com.broker.libupmq.context;

import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.concurrent.ConcurrentHashMap;

import javax.jms.ConnectionFactory;
import javax.jms.Queue;
import javax.jms.QueueConnectionFactory;
import javax.jms.Topic;
import javax.jms.TopicConnectionFactory;
import javax.naming.Context;
import javax.naming.NamingException;
import javax.naming.spi.InitialContextFactory;

import com.broker.libupmq.destination.UPMQQueue;
import com.broker.libupmq.destination.UPMQTopic;
import com.broker.libupmq.factory.UPMQConnectionFactory;
import com.broker.libupmq.factory.UPMQQueueConnectionFactory;
import com.broker.libupmq.factory.UPMQTopicConnectionFactory;
import com.broker.libupmq.factory.UPMQXAConnectionFactory;
import com.broker.libupmq.factory.UPMQXAQueueConnectionFactory;
import com.broker.libupmq.factory.UPMQXATopicConnectionFactory;

public class UPMQInitialContextFactory implements InitialContextFactory {

	private static final String[] DEFAULT_CONNECTION_FACTORY_NAMES = { "ConnectionFactory", "XAConnectionFactory", "QueueConnectionFactory", "XAQueueConnectionFactory", "TopicConnectionFactory",
			"XATopicConnectionFactory" };

	private String connectionPrefix = "connection.";
	private String queuePrefix = "queue.";
	private String topicPrefix = "topic.";

	@Override
	public Context getInitialContext(Hashtable<?, ?> environment) throws NamingException {
		// lets create a factory
		Map<String, Object> data = new ConcurrentHashMap<String, Object>();
		String[] names = getConnectionFactoryNames(environment);
		for (int i = 0; i < names.length; i++) {
			ConnectionFactory factory = null;
			String name = names[i];

			try {
				factory = createConnectionFactory(name, environment);
			} catch (Exception e) {
				throw new NamingException("Invalid broker URL");

			}
			/*
			 * if( broker==null ) { try { broker = factory.getEmbeddedBroker(); }
			 * catch (JMSException e) { log.warn("Failed to get embedded
			 * broker", e); } }
			 */
			data.put(name, factory);
		}

		createQueues(data, environment);
		createTopics(data, environment);
		/*
		 * if (broker != null) { data.put("destinations",
		 * broker.getDestinationContext(environment)); }
		 */
		data.put("dynamicQueues", new UPMQCreateContext() {
			@Override
			protected Object createEntry(String name) {
				return new UPMQQueue(name);
			}
		});
		data.put("dynamicTopics", new UPMQCreateContext() {
			@Override
			protected Object createEntry(String name) {
				return new UPMQTopic(name);
			}
		});

		return createContext(environment, data);
	}

	// Properties
	// -------------------------------------------------------------------------
	public String getTopicPrefix() {
		return topicPrefix;
	}

	public void setTopicPrefix(String topicPrefix) {
		this.topicPrefix = topicPrefix;
	}

	public String getQueuePrefix() {
		return queuePrefix;
	}

	public void setQueuePrefix(String queuePrefix) {
		this.queuePrefix = queuePrefix;
	}

	// Implementation methods
	// -------------------------------------------------------------------------

	protected UPMQReadOnlyContext createContext(Hashtable environment, Map<String, Object> data) {
		return new UPMQReadOnlyContext(environment, data);
	}

	protected ConnectionFactory createConnectionFactory(String name, Hashtable environment) throws URISyntaxException {
		Hashtable temp = new Hashtable(environment);
		if (DEFAULT_CONNECTION_FACTORY_NAMES[1].equals(name) || DEFAULT_CONNECTION_FACTORY_NAMES[3].equals(name) || DEFAULT_CONNECTION_FACTORY_NAMES[5].equals(name)) {
			// don't try to mod environment, it may be readonly
			temp.put("XA", String.valueOf(true));
		}
		String prefix = connectionPrefix + name + ".";
		for (Iterator iter = environment.entrySet().iterator(); iter.hasNext();) {
			Map.Entry entry = (Map.Entry) iter.next();
			String key = (String) entry.getKey();
			if (key.startsWith(prefix)) {
				// Rename the key...
				temp.remove(key);
				key = key.substring(prefix.length());
				temp.put(key, entry.getValue());
			}
		}

		if (name.startsWith("Queue") || name.startsWith("XAQueue")) {
			return createQueueConnectionFactory(temp);
		} else if (name.startsWith("Topic") || name.startsWith("XATopic")) {
			return createTopicConnectionFactory(temp);
		} else {
			return createConnectionFactory(temp);
		}
	}

	protected String[] getConnectionFactoryNames(Map environment) {
		String factoryNames = (String) environment.get("connectionFactoryNames");
		if (factoryNames != null) {
			List<String> list = new ArrayList<String>();
			for (StringTokenizer enumeration = new StringTokenizer(factoryNames, ","); enumeration.hasMoreTokens();) {
				list.add(enumeration.nextToken().trim());
			}
			int size = list.size();
			if (size > 0) {
				String[] answer = new String[size];
				list.toArray(answer);
				return answer;
			}
		}
		return DEFAULT_CONNECTION_FACTORY_NAMES;
	}

	protected void createQueues(Map<String, Object> data, Hashtable environment) {
		for (Iterator iter = environment.entrySet().iterator(); iter.hasNext();) {
			Map.Entry entry = (Map.Entry) iter.next();
			String key = entry.getKey().toString();
			if (key.startsWith(queuePrefix)) {
				String jndiName = key.substring(queuePrefix.length());
				data.put(jndiName, createQueue(entry.getValue().toString()));
			}
		}
	}

	protected void createTopics(Map<String, Object> data, Hashtable environment) {
		for (Iterator iter = environment.entrySet().iterator(); iter.hasNext();) {
			Map.Entry entry = (Map.Entry) iter.next();
			String key = entry.getKey().toString();
			if (key.startsWith(topicPrefix)) {
				String jndiName = key.substring(topicPrefix.length());
				data.put(jndiName, createTopic(entry.getValue().toString()));
			}
		}
	}

	/**
	 * Factory method to create new Queue instances
	 */
	protected Queue createQueue(String name) {
		return new UPMQQueue(name);
	}

	/**
	 * Factory method to create new Topic instances
	 */
	protected Topic createTopic(String name) {
		return new UPMQTopic(name);
	}

	/**
	 * Factory method to create a new connection factory from the given environment
	 */
	protected ConnectionFactory createConnectionFactory(Hashtable environment) throws URISyntaxException {
		UPMQConnectionFactory answer = needsXA(environment) ? new UPMQXAConnectionFactory() : new UPMQConnectionFactory();
		Properties properties = new Properties();
		properties.putAll(environment);
		answer.setProperties(properties);
		return answer;
	}

	/**
	 * Factory method to create a new connection factory from the given environment
	 */
	protected QueueConnectionFactory createQueueConnectionFactory(Hashtable environment) throws URISyntaxException {
		UPMQQueueConnectionFactory answer = needsXA(environment) ? new UPMQXAQueueConnectionFactory() : new UPMQQueueConnectionFactory();
		Properties properties = new Properties();
		properties.putAll(environment);
		answer.setProperties(properties);
		return answer;
	}

	/**
	 * Factory method to create a new connection factory from the given environment
	 */
	protected TopicConnectionFactory createTopicConnectionFactory(Hashtable environment) throws URISyntaxException {
		UPMQTopicConnectionFactory answer = needsXA(environment) ? new UPMQXATopicConnectionFactory() : new UPMQTopicConnectionFactory();
		Properties properties = new Properties();
		properties.putAll(environment);
		answer.setProperties(properties);
		return answer;
	}

	private boolean needsXA(Hashtable environment) {
		boolean isXA = Boolean.parseBoolean((String) environment.get("XA"));
		// property not applicable to connectionfactory so remove
		environment.remove("XA");
		return isXA;
	}

	public String getConnectionPrefix() {
		return connectionPrefix;
	}

	public void setConnectionPrefix(String connectionPrefix) {
		this.connectionPrefix = connectionPrefix;
	}

}
