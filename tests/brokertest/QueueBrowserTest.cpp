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

#include "QueueBrowserTest.h"
#include <sstream>
#include <fake_cpp14.h>

////////////////////////////////////////////////////////////////////////////////
void QueueBrowserTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}
////////////////////////////////////////////////////////////////////////////////
TEST_F(QueueBrowserTest, testReceiveBrowseReceive) {
  cms::Session *session(cmsProvider->getSession());

  std::unique_ptr<cms::Queue> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<cms::MessageConsumer> consumer(session->createConsumer(queue.get()));
  std::unique_ptr<cms::MessageProducer> producer(session->createProducer(queue.get()));

  std::unique_ptr<cms::TextMessage> message1(session->createTextMessage("First Message"));
  std::unique_ptr<cms::TextMessage> message2(session->createTextMessage("Second Message"));
  std::unique_ptr<cms::TextMessage> message3(session->createTextMessage("Third Message"));

  EXPECT_NO_THROW(producer->send(message1.get()));
  EXPECT_NO_THROW(producer->send(message2.get()));
  EXPECT_NO_THROW(producer->send(message3.get()));
  EXPECT_NO_THROW(producer->close());

  // Get the first.
  std::unique_ptr<cms::TextMessage> inbound;
  EXPECT_NO_THROW(inbound.reset(dynamic_cast<cms::TextMessage *>(consumer->receive(3000))));
  EXPECT_TRUE(inbound != nullptr);
  message1->setReadable();
  EXPECT_EQ(message1->getText(), inbound->getText());

  EXPECT_NO_THROW(consumer->close());

  std::unique_ptr<cms::QueueBrowser> browser;
  EXPECT_NO_THROW(browser.reset(session->createBrowser(queue.get())));
  cms::MessageEnumeration *enumeration = nullptr;
  EXPECT_NO_THROW(enumeration = browser->getEnumeration());
  EXPECT_TRUE(enumeration != nullptr);
  // browse the second
  EXPECT_TRUE(enumeration->hasMoreMessages()) << "should have received the second message";
  EXPECT_NO_THROW(inbound.reset(dynamic_cast<cms::TextMessage *>(enumeration->nextMessage())));
  EXPECT_TRUE(inbound != nullptr);
  message2->setReadable();
  EXPECT_EQ(message2->getText(), inbound->getText());

  // browse the third.
  EXPECT_TRUE(enumeration->hasMoreMessages()) << "should have received the third message";
  EXPECT_NO_THROW(inbound.reset(dynamic_cast<cms::TextMessage *>(enumeration->nextMessage())));
  EXPECT_TRUE(inbound != nullptr);
  message3->setReadable();
  EXPECT_EQ(message3->getText(), inbound->getText());

  // There should be no more.
  bool tooMany = false;
  while (enumeration->hasMoreMessages()) {
    tooMany = true;
  }
  EXPECT_TRUE(!tooMany) << "should not have browsed any more messages";
  EXPECT_NO_THROW(browser->close());

  EXPECT_NO_THROW(browser.reset(session->createBrowser(queue.get())));
  EXPECT_NO_THROW(enumeration = browser->getEnumeration());
  // browse the second
  EXPECT_TRUE(enumeration->hasMoreMessages()) << "should have received the second message";
  EXPECT_NO_THROW(inbound.reset(dynamic_cast<cms::TextMessage *>(enumeration->nextMessage())));
  EXPECT_TRUE(inbound != nullptr);
  EXPECT_EQ(message2->getText(), inbound->getText());
  // browse the third.
  EXPECT_TRUE(enumeration->hasMoreMessages()) << "should have received the third message";
  EXPECT_NO_THROW(inbound.reset(dynamic_cast<cms::TextMessage *>(enumeration->nextMessage())));
  EXPECT_TRUE(inbound != nullptr);
  EXPECT_EQ(message3->getText(), inbound->getText());

  // Re-open the consumer
  EXPECT_NO_THROW(consumer.reset(session->createConsumer(queue.get())));
  // Receive the second.
  EXPECT_NO_THROW(inbound.reset(dynamic_cast<cms::TextMessage *>(consumer->receive(3000))));
  EXPECT_TRUE(inbound != nullptr);
  EXPECT_EQ(message2->getText(), inbound->getText());
  // Receive the third.
  EXPECT_NO_THROW(inbound.reset(dynamic_cast<cms::TextMessage *>(consumer->receive(3000))));
  EXPECT_TRUE(inbound != nullptr);
  EXPECT_EQ(message3->getText(), inbound->getText());

  EXPECT_NO_THROW(consumer->close());
  EXPECT_NO_THROW(browser->close());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(QueueBrowserTest, testBrowseReceive) {
  std::unique_ptr<cms::TextMessage> inbound;

  cms::Session *session(cmsProvider->getSession());

  std::unique_ptr<cms::Queue> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<cms::MessageProducer> producer(session->createProducer(queue.get()));
  std::unique_ptr<cms::TextMessage> message1(session->createTextMessage("First Message"));

  producer->send(message1.get());

  // create browser first
  std::unique_ptr<cms::QueueBrowser> browser(session->createBrowser(queue.get()));
  cms::MessageEnumeration *enumeration = browser->getEnumeration();

  // create consumer
  std::unique_ptr<cms::MessageConsumer> consumer(session->createConsumer(queue.get()));

  // browse the first message
  EXPECT_TRUE(enumeration->hasMoreMessages()) << "should have received the first message";
  inbound.reset(dynamic_cast<cms::TextMessage *>(enumeration->nextMessage()));
  EXPECT_TRUE(inbound.get() != nullptr);
  message1->setReadable();
  EXPECT_EQ(message1->getText(), inbound->getText());

  // Receive the first message.
  inbound.reset(dynamic_cast<cms::TextMessage *>(consumer->receive(3000)));
  EXPECT_TRUE(inbound.get() != nullptr);
  EXPECT_EQ(message1->getText(), inbound->getText());

  consumer->close();
  browser->close();
  producer->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(QueueBrowserTest, testQueueBrowserWith2Consumers) {
  std::stringstream sstr;

  static const int numMessages = 5;

  Connection *connection = cmsProvider->getConnection();

  EXPECT_TRUE(connection != nullptr);

  std::unique_ptr<cms::Session> session(connection->createSession(cms::Session::CLIENT_ACKNOWLEDGE));

  std::unique_ptr<cms::Queue> queue(
      session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));  // testQueueBrowserWith2Consumers
  std::unique_ptr<cms::Queue> queuePrefetch10(session->createQueue(
      ::testing::UnitTest::GetInstance()->current_test_info()->name()));  // testQueueBrowserWith2Consumers?consumer.prefetchSize=10
  std::unique_ptr<cms::Queue> queuePrefetch1(session->createQueue(
      ::testing::UnitTest::GetInstance()->current_test_info()->name()));  // testQueueBrowserWith2Consumers?consumer.prefetchSize=1

  std::unique_ptr<ConnectionFactory> factory(cms::ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL()));
  std::unique_ptr<Connection> connection2(factory->createConnection());

  connection2->start();

  std::unique_ptr<cms::Session> session2(connection2->createSession(cms::Session::AUTO_ACKNOWLEDGE));

  std::unique_ptr<cms::MessageProducer> producer(session->createProducer(queue.get()));
  std::unique_ptr<cms::MessageConsumer> consumer(session->createConsumer(queuePrefetch10.get()));

  producer->setDeliveryMode(cms::DeliveryMode::NON_PERSISTENT);

  for (int i = 0; i < numMessages; i++) {
    sstr.str("");
    sstr << "Message: " << i;
    std::unique_ptr<cms::TextMessage> message(session->createTextMessage(sstr.str()));
    producer->send(message.get());
  }

  std::unique_ptr<cms::QueueBrowser> browser(session2->createBrowser(queuePrefetch1.get()));
  cms::MessageEnumeration *browserView = browser->getEnumeration();

  std::vector<std::unique_ptr<cms::Message>> messages;
  for (int i = 0; i < numMessages; i++) {
    std::unique_ptr<cms::Message> m1(consumer->receive(3000));
    sstr.str("");
    sstr << "m1 is null for index: " << i;
    EXPECT_TRUE(m1 != nullptr) << sstr.str();
    messages.emplace_back(std::move(m1));
  }

  for (int i = 0; i < numMessages && browserView->hasMoreMessages(); i++) {
    cms::Message *m1 = messages[i].get();
    std::unique_ptr<cms::Message> m2(browserView->nextMessage());
    sstr.str("");
    sstr << "m2 is null for index: " << i;
    EXPECT_TRUE(m2 != nullptr) << sstr.str();
    EXPECT_EQ(m1->getCMSMessageID(), m2->getCMSMessageID());
  }

  EXPECT_FALSE(browserView->hasMoreMessages()) << "nothing left in the browser";
  std::unique_ptr<cms::TextMessage> messageEmpty;
  messageEmpty.reset(dynamic_cast<TextMessage *>(consumer->receiveNoWait()));
  EXPECT_TRUE(messageEmpty == nullptr) << "expect empty message, but got " << messageEmpty->getText() << " [" << messageEmpty->getCMSMessageID()
                                       << "]";

  for (auto &msg : messages) {
    EXPECT_NO_THROW(msg->acknowledge());
  }
  messages.clear();

  browser->close();
  producer->close();
  consumer->close();
  session->close();
  session2->close();
}

void QueueBrowserTest::TearDown() {}
