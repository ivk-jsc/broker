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


import javax.jms.*;

import com.broker.libupmq.destination.UPMQDestination;
import com.broker.libupmq.factory.UPMQConnectionFactory;

public class QReceiver implements MessageListener {
    private boolean stop = false;
    public static void main(String[] args) {
        new QReceiver().receive();
    }
    public void receive() {
// Фиксация параметров соединения и имени очереди
        String brokerURI = UPMQConnectionFactory.DEFAULT_BROKER_URL;
        String destURI = UPMQDestination.QUEUE_PREFIX + UPMQDestination.DEFAULT_DESTINATION;


        try {
///Вывод на экран параметров соединения и мени очереди
            System.out.println("ConnectionFactory URI: " + brokerURI);
            System.out.println("Destination name: " + destURI);

//Создание фабрики соединений

            UPMQConnectionFactory factory = new UPMQConnectionFactory();
            factory.setBrokerURI(brokerURI);

//Создание JMS-объектов
            Connection connection = factory.createConnection();
            Session session = connection.createSession(false, Session.AUTO_ACKNOWLEDGE);
            Destination queue = session.createQueue(destURI);
            MessageConsumer receiver = session.createConsumer(queue);
            receiver.setMessageListener(this);
            connection.start();

//Ожидание останова
            while (!stop) {
                Thread.sleep(1000);
            }
//Выход
            System.out.println("Exiting...");
            connection.close();
            System.out.println("Goodbye!");
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }

    public void onMessage(Message message) {
        try {
            String msgText = ((TextMessage) message).getText();
            System.out.println(msgText);
            if ("stop".equals(msgText))
                stop = true;
        } catch (JMSException e) {
            e.printStackTrace();
            stop = true;
        }
    }
}

