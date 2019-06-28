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
 
package com.broker.simple;

import java.util.Random;

import javax.jms.Connection;
import javax.jms.DeliveryMode;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MessageProducer;
import javax.jms.Session;
import javax.jms.TextMessage;

import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.factory.UPMQConnectionFactory;
import com.broker.libupmq.utils.StopWatch;
import com.broker.libupmq.utils.TimeUtils;

public class SimpleProducer {

	private static String brokerURI = UPMQConnectionFactory.DEFAULT_BROKER_URL;
	private static String destURI = UPMQDestination.QUEUE_PREFIX + UPMQDestination.DEFAULT_DESTINATION;
//    private static String destURI = UPMQDestination.TOPIC_PREFIX + UPMQDestination.DEFAULT_DESTINATION;
	private static int numMessages = 10000;

	public static void main(String[] args) {

		if (args.length == 3) {
			brokerURI = args[0];
			destURI = args[1];
			numMessages = Integer.parseInt(args[2]);
		}

		System.out.println("=====================================================");
		System.out.println("simpleproducer start " + "(" + brokerURI + " " + destURI + " " + numMessages + ")");
		System.out.println("=====================================================");

		try {

			UPMQConnectionFactory factory = new UPMQConnectionFactory();
			factory.setBrokerURI(brokerURI);

			Connection connection = factory.createConnection();
			connection.start();

			Session session = connection.createSession(false, Session.AUTO_ACKNOWLEDGE);

			Destination destination = null;
			if (destURI.startsWith(UPMQDestination.QUEUE_PREFIX))
				destination = session.createQueue(destURI);
			else if (destURI.startsWith(UPMQDestination.TOPIC_PREFIX))
				destination = session.createTopic(destURI);
			if (destURI.startsWith(UPMQDestination.TEMP_QUEUE_PREFIX))
				destination = session.createTemporaryQueue();
			else if (destURI.startsWith(UPMQDestination.TEMP_TOPIC_PREFIX))
				destination = session.createTemporaryTopic();

			if (destination == null)
				throw new JMSException("invalid destination");

			MessageProducer producer = session.createProducer(destination);
			producer.setDeliveryMode(DeliveryMode.PERSISTENT);

			Random generator = new Random();

			String s = "Message " + generator.nextInt(1_000_000_000) + ":";

			StopWatch watch = new StopWatch();
			for (int i = 0; i < numMessages; i++) {

				String ss = s + (i + 1);
				TextMessage textMessage = session.createTextMessage(ss);

				producer.send(textMessage);

				System.out.printf("Sent message: %s\n", ss);
			}
			System.out.println("Send in " + TimeUtils.printDuration(watch.taken()));

			producer.close();
			session.close();
			connection.close();

		} catch (Exception e) {
			e.printStackTrace();
			System.exit(1);
		}

		System.out.println("=====================================================");
		System.out.println("simpleproducer end");
		System.out.println("=====================================================");
	}
}
