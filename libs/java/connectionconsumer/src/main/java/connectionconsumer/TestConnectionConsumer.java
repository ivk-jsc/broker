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
 
package connectionconsumer;

import javax.naming.Context;
import javax.naming.InitialContext;

import com.broker.libupmq.factory.UPMQConnectionFactory;

import javax.jms.Topic;
import javax.jms.Session;
import javax.jms.Message;
import javax.jms.TopicSession;
import javax.jms.TopicPublisher;
import javax.jms.TopicConnection;
import javax.jms.TopicConnectionFactory;

import java.util.Properties;

import javax.jms.ConnectionConsumer;

public class TestConnectionConsumer
{
	public static void main(String[] args) throws Exception
	{			
		Properties props = new Properties();
		props.put(Context.INITIAL_CONTEXT_FACTORY, "com.broker.libupmq.context.UPMQInitialContextFactory");
		props.put(Context.PROVIDER_URL, UPMQConnectionFactory.DEFAULT_BROKER_URL);	
		InitialContext ctx = new InitialContext(props);
		TopicConnectionFactory factory = (TopicConnectionFactory) ctx.lookup("TopicConnectionFactory");
		Topic topic = (Topic) ctx.lookup("topic://testTopic");

		
		// create a topic connection
		TopicConnection topicConn = factory.createTopicConnection();
		
		// create a server session pool
		TestServerSessionPool ssPool = new TestServerSessionPool(topicConn);
		topicConn.setExceptionListener(ssPool);

		// create a topic connection consumer
		ConnectionConsumer connConsumer = topicConn.createConnectionConsumer(topic, null, ssPool, 10);

		// start the connection
		topicConn.start();
		
		// send some messages to the newly created connection consumer
		int ackMode = Session.AUTO_ACKNOWLEDGE;
		TopicSession session = topicConn.createTopicSession(false, ackMode);
		TopicPublisher publisher = session.createPublisher(topic);

		Message msg = session.createMessage();

		for (int i = 0; i < 100; i++)
		{
			publisher.publish(msg);
		}

		System.out.println("sent 100 messages");

		publisher.close();

		// wait for connection consumer
		while (true)
		{
			Thread.sleep(100);
		}
	}
}
