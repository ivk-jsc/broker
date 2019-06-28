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

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.jms.Connection;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageProducer;
import javax.jms.Session;
import javax.jms.TextMessage;

import com.broker.libupmq.factory.UPMQConnectionFactory;
import com.broker.libupmq.utils.StopWatch;
import com.broker.libupmq.utils.TimeUtils;

public class SimpleStress {

	private static int threads;
	private static int messages;
	private static CountDownLatch waitStart;
	private static CountDownLatch waitEnd;

	public static void main(String[] args) throws InterruptedException {
		System.out.println("Service start");

		threads = 100;
		messages = 100;

		waitStart = new CountDownLatch(threads);
		waitEnd = new CountDownLatch(threads);

		List<SendRecvThread> threadList = new ArrayList<>(threads);
		for (int i = 0; i < threads; i++)
			threadList.add(new SendRecvThread(i));

		ExecutorService service = Executors.newFixedThreadPool(threads);
		for (int i = 0; i < threads; i++)
			service.submit(threadList.get(i));

		while (waitStart.getCount() != 0) {
			System.out.println("Threads not ready yet");
			Thread.sleep(10);
		}

		while (waitEnd.getCount() != 0) {
			Thread.sleep(1000);
		}

		service.shutdown();

		System.out.println("Service end");
	}

	public static class SendRecvThread implements Runnable {

		private int name;
		private int received;
		private Connection connection;
		private Session session;
		private Destination destination;
		private MessageProducer producer;
		private MessageConsumer consumer;
		private TextMessage messageTx;

		public SendRecvThread(int i) {
			name = i;
			try {
				connection = new UPMQConnectionFactory().createConnection();
				connection.start();
				session = connection.createSession(false, Session.AUTO_ACKNOWLEDGE);
				destination = session.createQueue(String.valueOf(name));
				producer = session.createProducer(destination);

				messageTx = session.createTextMessage("test text");
			} catch (JMSException e) {
				System.out.println("Thread: " + name + " failed");
				e.printStackTrace();
			}
		}

		@Override
		public void run() {
			try {
				System.out.println("Thread " + name + " created and wait");

				waitStart.countDown();
				waitStart.await();
				System.out.println("Thread " + name + " started");
				StopWatch watch = new StopWatch();
				for (int i = 0; i < messages; i++)
					producer.send(messageTx);

				System.out.println("Thread " + name + " sended " + messages + " messages");

				consumer = session.createConsumer(destination);

				Message messageRx;
				while ((messageRx = consumer.receive(3000)) != null)
					received++;

				System.out.println("Thread " + name + " send and received " + received + " messages in " + TimeUtils.printDuration(watch.taken()));

				producer.close();
				consumer.close();
				session.close();
				connection.close();

				waitEnd.countDown();
				waitEnd.await();
				System.out.println("Thread " + name + " ended");
			} catch (Exception ex) {
				ex.printStackTrace();
			}
		}
	}
}
