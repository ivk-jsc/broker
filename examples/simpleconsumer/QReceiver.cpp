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

#include <cms/ConnectionFactory.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/ExceptionListener.h>
#include <cms/Message.h>
#include <memory>
#include <thread>
#include <chrono>

class QReceiver : public cms::MessageListener {
  bool stop = false;

 public:
  void receive() {
    // Фиксация параметров соединения и имени очереди
    std::string brokerURI = cms::ConnectionFactory::DEFAULT_URI();
    std::string destURI = "defaultDestination";

    try {
      // Вывод на экран параметров соединениямени очереди
      std::cout << "ConnectionFactory URI: " << brokerURI << std::endl;
      std::cout << "Destination name: " << destURI << std::endl;

      //Создание фабрики соединений

      std::unique_ptr<cms::ConnectionFactory> factory(cms::ConnectionFactory::createCMSConnectionFactory(brokerURI));

      //Создание JMS-объектов
      std::unique_ptr<cms::Connection> connection(factory->createConnection());
      std::unique_ptr<cms::Session> session(connection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
      std::unique_ptr<cms::Destination> queue(session->createQueue(destURI));
      std::unique_ptr<cms::MessageConsumer> receiver(session->createConsumer(queue.get()));
      receiver->setMessageListener(this);
      connection->start();

      //Ожидание останова
      while (!stop) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      //Выход
      std::cout << "Exiting..." << std::endl;

      connection->close();
      std::cout << "Goodbye!" << std::endl;
    } catch (cms::CMSException &e) {
      e.printStackTrace();
      exit(1);
    }
  }

  void onMessage(const cms::Message *message) override {
    try {
      const auto msg = dynamic_cast<const cms::TextMessage *>(message);
      const std::string msgText = (msg != nullptr) ? msg->getText() : "";
      std::cout << msgText << std::endl;
      if (msgText == "stop") {
        stop = true;
      }
    } catch (cms::CMSException &e) {
      e.printStackTrace();
      stop = true;
    }
  }
};

int qreceiver_main() {
  QReceiver qReceiver;
  qReceiver.receive();
  return 0;
}