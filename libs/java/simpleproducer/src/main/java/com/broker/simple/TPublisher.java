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

import java.io.*;

import javax.jms.Connection;
import javax.jms.Destination;
import javax.jms.MessageProducer;
import javax.jms.Session;
import javax.jms.TextMessage;

import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.factory.UPMQConnectionFactory;

public class TPublisher {
    public static void main(String[] args) {
        new TPublisher().send();
    }

    private void send() {
// Фиксация параметров соединения и имени очереди
        String brokerURI = UPMQConnectionFactory.DEFAULT_BROKER_URL;
        String destURI = UPMQDestination.TOPIC_PREFIX + UPMQDestination.DEFAULT_DESTINATION;

        try {
//Вывод на экран параметров соединения и мени очереди
            System.out.println("ConnectionFactory URI: " + brokerURI);
            System.out.println("Destination name: " + destURI);

//Создание фабрики соединений

            UPMQConnectionFactory factory = new UPMQConnectionFactory();
            factory.setBrokerURI(brokerURI);

//Создание JMS-объектов
            Connection connection = factory.createConnection();
            Session session = connection.createSession(false, Session.AUTO_ACKNOWLEDGE);
            Destination queue = session.createTopic(destURI);
            MessageProducer sender = session.createProducer(queue);

            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
//Передача сообщений
            String messageText = null;
            while (true) {
                System.out.println("Enter message to send or 'quit':");
                messageText = reader.readLine();
                if ("quit".equals(messageText)) {
                    break;
                }
                TextMessage message = session.createTextMessage(messageText);
                sender.send(message);
            }
//Выход
            System.out.println("Exiting...");
            reader.close();
            connection.close();
            System.out.println("Goodbye!");
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}