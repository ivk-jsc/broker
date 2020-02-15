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

import java.util.Scanner;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.jms.Connection;
import javax.jms.Destination;
import javax.jms.ExceptionListener;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageListener;
import javax.jms.Session;
import javax.jms.TextMessage;

import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.factory.UPMQConnectionFactory;

public class SimpleConsumer {

	private static String brokerURI = UPMQConnectionFactory.DEFAULT_BROKER_URL;
	private static String destURI = UPMQDestination.QUEUE_PREFIX + UPMQDestination.DEFAULT_DESTINATION;
//    private static String destURI = UPMQDestination.TOPIC_PREFIX + UPMQDestination.DEFAULT_DESTINATION;

	public static void main(String[] args) throws Exception {

		if (args.length == 2) {
			brokerURI = args[0];
			destURI = args[1];
		}

		System.out.println("=====================================================");
		System.out.println("simpleconsumer start " + "(" + brokerURI + " " + destURI + ")");
		System.out.println("=====================================================");

		Consumer consumer = new Consumer();
		Thread brokerThread = new Thread(consumer);
		brokerThread.setDaemon(false);
		brokerThread.start();

		System.out.println("Press 'q' and Enter to quit");
		try (Scanner in = new Scanner(System.in)) {
			in.nextLine();
		}

		consumer.stop();

		brokerThread.join();

		System.out.println("=====================================================");
		System.out.println("simpleconsumer end");
		System.out.println("=====================================================");
	}

	public static class Consumer implements Runnable, MessageListener, ExceptionListener {

		private int i;
        private AtomicBoolean running = new AtomicBoolean(false);

		public static class MsgListener implements MessageListener {

			private int i;

			@Override
			public void onMessage(Message message) {
				try {
					if (message instanceof TextMessage) {
						TextMessage textMessage = (TextMessage) message;
						String text = textMessage.getText();
						i++;
						System.out.println("Message #" + i + " Received: " + text);
					} else {
						i++;
						System.out.println("Message #" + i + " Received: " + message);
					}
				} catch (JMSException jmse) {
					System.out.println("Caught: " + jmse);
					jmse.printStackTrace();
				}
			}
		}

		public static class ExListener implements ExceptionListener {

			@Override
			public void onException(JMSException jmse) {
				System.out.println("Caught: " + jmse);
				jmse.printStackTrace();
			}
		}

        void stop() {
            running.set(false);
        }

		@Override
		public void run() {
		    running.set(true);
			try {

                UPMQConnectionFactory factory = new UPMQConnectionFactory();
                factory.setBrokerURI(brokerURI);

                Connection connection = factory.createConnection();
                connection.start();
                connection.setExceptionListener(new ExListener());

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

                MessageConsumer consumer = session.createConsumer(destination);

                while (running.get()) {
                    Message message = consumer.receive(3000);
                    if (message != null) {
                        if (message instanceof TextMessage) {
                            TextMessage textMessage = (TextMessage) message;
                            String text = textMessage.getText();
                            i++;
                            System.out.println("Message #" + i + " Received: " + text);
                        } else {
                            i++;
                            System.out.println("Message #" + i + " Received: " + message);
                        }
                    }
                }

                consumer.close();
                session.close();
                connection.close();
			} catch (Exception e) {
				e.printStackTrace();
				System.exit(1);
			}
		}

		@Override
		public void onMessage(Message message) {
			try {
				if (message instanceof TextMessage) {
					TextMessage textMessage = (TextMessage) message;
					String text = textMessage.getText();
					i++;
					System.out.println("Message #" + i + " Received: " + text);
				} else {
					i++;
					System.out.println("Message #" + i + " Received: " + message);
				}
			} catch (JMSException jmse) {
				System.out.println("Caught: " + jmse);
				jmse.printStackTrace();
			}
		}

		@Override
		public void onException(JMSException jmse) {
			System.out.println("Caught: " + jmse);
			jmse.printStackTrace();
		}
	}
}
