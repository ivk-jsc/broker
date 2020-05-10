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

#include "TransactionTest.h"
#include <algorithm>
#include <string>
#include <utility>
#include <fake_cpp14.h>
////////////////////////////////////////////////////////////////////////////////
void TransactionTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL(), cms::Session::SESSION_TRANSACTED);
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TransactionTest, testSendReceiveTransactedBatches) {
  // Create CMS Object for Comms
  cms::Session *session = cmsProvider->getSession();
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();

  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
  std::vector<std::string> msg(batchSize);
  size_t ii = 0;
  std::generate(msg.begin(), msg.end(), [&ii]() { return "Batch Message " + std::to_string(ii++); });

  for (size_t j = 0; j < batchCount - 8; j++) {
    for (size_t i = 0; i < batchSize; i++) {
      std::unique_ptr<TextMessage> message(session->createTextMessage(msg[i]));
      EXPECT_NO_THROW(producer->send(message.get())) << "Send should not throw an exception here.";
    }

    EXPECT_NO_THROW(session->commit()) << "Session Commit should not throw an exception here:";

    for (size_t i = 0; i < batchSize; i++) {
      std::unique_ptr<TextMessage> message;
      EXPECT_NO_THROW(message.reset(dynamic_cast<TextMessage *>(consumer->receive(3000)))) << "Receive Shouldn't throw a Message here:";

      ASSERT_TRUE(message != nullptr) << "Failed to receive all messages in batch";
      EXPECT_TRUE(msg[i] == message->getText());
    }

    EXPECT_NO_THROW(session->commit()) << "Session Commit should not throw an exception here:";
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TransactionTest, testSendRollback) {
  // Create CMS Object for Comms
  cms::Session *session = cmsProvider->getSession();
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();

  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<TextMessage> outbound1(session->createTextMessage("First Message"));
  std::unique_ptr<TextMessage> outbound2(session->createTextMessage("Second Message"));

  // sends a message
  EXPECT_NO_THROW(producer->send(outbound1.get()));
  EXPECT_NO_THROW(session->commit());

  // sends a message that gets rollbacked
  std::unique_ptr<Message> rollback(session->createTextMessage("I'm going to get rolled back."));
  EXPECT_NO_THROW(producer->send(rollback.get()));

  EXPECT_NO_THROW(session->rollback());

  // sends a message
  EXPECT_NO_THROW(producer->send(outbound2.get()));
  EXPECT_NO_THROW(session->commit());

  // receives the first messag

  std::unique_ptr<TextMessage> inbound1;
  EXPECT_NO_THROW(inbound1.reset(dynamic_cast<TextMessage *>(consumer->receive(3000))));

  // receives the second message
  std::unique_ptr<TextMessage> inbound2;
  EXPECT_NO_THROW(inbound2.reset(dynamic_cast<TextMessage *>(consumer->receive(3000))));

  // validates that the rollbacked was not consumed
  EXPECT_NO_THROW(session->commit());

  ASSERT_TRUE(inbound1 != nullptr);
  outbound1->setReadable();
  EXPECT_EQ(outbound1->getText(), inbound1->getText());

  ASSERT_TRUE(inbound2 != nullptr);
  outbound2->setReadable();
  EXPECT_EQ(outbound2->getText(), inbound2->getText())
      << "invalid order : ou1-id[" << outbound1->getCMSMessageID() << "] : in1-id[" << inbound1->getCMSMessageID() << "]\n"
      << "invalid order : ou2-id[" << outbound2->getCMSMessageID() << "] : in2-id[" << inbound2->getCMSMessageID() << "]";
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TransactionTest, testSendRollbackCommitRollback) {
  // Create CMS Object for Comms
  cms::Session *session = cmsProvider->getSession();
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();

  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<TextMessage> outbound1(session->createTextMessage("First Message"));
  std::unique_ptr<TextMessage> outbound2(session->createTextMessage("Second Message"));

  // sends them and then rolls back.
  producer->send(outbound1.get());
  producer->send(outbound2.get());
  session->rollback();

  // Send one and commit.
  producer->send(outbound1.get());
  session->commit();

  // receives the first message
  std::unique_ptr<TextMessage> inbound1(dynamic_cast<TextMessage *>(consumer->receive(3000)));
  std::unique_ptr<TextMessage> inboundEmpty(dynamic_cast<TextMessage *>(consumer->receive(3000)));
  EXPECT_TRUE(nullptr == inboundEmpty) << "must be empty, but : " << inboundEmpty->getText();
  ASSERT_TRUE(inbound1 != nullptr);
  outbound1->setReadable();
  EXPECT_EQ(outbound1->getText(), inbound1->getText());

  session->rollback();

  inbound1.reset(dynamic_cast<TextMessage *>(consumer->receive(5000)));
  inboundEmpty.reset(dynamic_cast<TextMessage *>(consumer->receive(5000)));
  EXPECT_TRUE(nullptr == inboundEmpty) << "expect empty, but got =>" << inboundEmpty->getText();
  ASSERT_TRUE(inbound1 != nullptr);
  outbound2->setReadable();
  EXPECT_EQ(outbound1->getText(), inbound1->getText());

  // validates that the rollbacked was not consumed
  session->commit();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TransactionTest, testSendSessionClose) {
  cmsProvider->getProducer()->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<TextMessage> outbound1(cmsProvider->getSession()->createTextMessage("First Message"));
  std::unique_ptr<TextMessage> outbound2(cmsProvider->getSession()->createTextMessage("Second Message"));

  // sends a message
  EXPECT_NO_THROW(cmsProvider->getProducer()->send(outbound1.get()));
  EXPECT_NO_THROW(cmsProvider->getSession()->commit());

  // sends a message that gets rolled back
  std::unique_ptr<cms::Message> rollback(cmsProvider->getSession()->createTextMessage("I'm going to get rolled back."));
  EXPECT_NO_THROW(cmsProvider->getProducer()->send(rollback.get()));
  EXPECT_NO_THROW(cmsProvider->getConsumer()->close());

  EXPECT_NO_THROW(cmsProvider->reconnectSession());

  // sends a message
  EXPECT_NO_THROW(cmsProvider->getProducer()->send(outbound2.get()));
  EXPECT_NO_THROW(cmsProvider->getSession()->commit());

  // receives the first message
  std::unique_ptr<TextMessage> inbound1;
  EXPECT_NO_THROW(inbound1.reset(dynamic_cast<TextMessage *>(cmsProvider->getConsumer()->receive(3000))));

  // receives the second message
  std::unique_ptr<cms::TextMessage> inbound2;
  EXPECT_NO_THROW(inbound2.reset(dynamic_cast<TextMessage *>(cmsProvider->getConsumer()->receive(3000))));

  // validates that the rolled back was not consumed
  EXPECT_NO_THROW(cmsProvider->getSession()->commit());

  ASSERT_TRUE(inbound1 != nullptr);
  outbound1->setReadable();
  EXPECT_TRUE(outbound1->getText() == inbound1->getText());

  ASSERT_TRUE(inbound2 != nullptr);
  outbound2->setReadable();
  EXPECT_TRUE(outbound2->getText() == inbound2->getText());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TransactionTest, testWithTTLSet) {
  cmsProvider->getProducer()->setDeliveryMode(DeliveryMode::PERSISTENT);

  cms::MessageConsumer *consumer = cmsProvider->getConsumer();

  const std::size_t NUM_MESSAGES = 50;
  std::vector<std::pair<std::string, std::string>> msg(NUM_MESSAGES);
  size_t ii = 0;
  std::generate(msg.begin(), msg.end(), [&ii]() { return std::make_pair("", "message" + std::to_string(ii++)); });
  // sends a message
  for (std::size_t i = 0; i < NUM_MESSAGES; ++i) {
    std::unique_ptr<TextMessage> outbound1(cmsProvider->getSession()->createTextMessage(msg[i].second));
    EXPECT_NO_THROW(
        cmsProvider->getProducer()->send(outbound1.get(), cms::DeliveryMode::PERSISTENT, cmsProvider->getProducer()->getPriority(), 120 * 1000));
    msg[i].first = outbound1->getCMSMessageID();
  }

  cmsProvider->getSession()->commit();

  // cmsSleep(3000);

  for (std::size_t i = 0; i < NUM_MESSAGES; ++i) {
    // receives the second message
    std::unique_ptr<TextMessage> inbound1;
    EXPECT_NO_THROW(inbound1.reset(dynamic_cast<TextMessage *>(consumer->receive(3000))));
    ASSERT_TRUE(inbound1 != nullptr);
    EXPECT_EQ(msg[i].first, inbound1->getCMSMessageID());
    EXPECT_EQ(msg[i].second, inbound1->getText());
  }

  EXPECT_NO_THROW(cmsProvider->getSession()->commit());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TransactionTest, testSessionCommitAfterConsumerClosed) {
  ConnectionFactory *factory = ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL());
  std::unique_ptr<cms::Connection> connection(factory->createConnection());

  {
    std::unique_ptr<cms::Session> session(connection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
    std::unique_ptr<cms::Queue> queue(session->createQueue("testSessionCommitAfterConsumerClosed"));
    std::unique_ptr<cms::MessageProducer> producer(session->createProducer(queue.get()));

    std::unique_ptr<cms::Message> message(session->createTextMessage("Hello"));
    producer->send(message.get());
    producer->close();
    session->close();
  }

  std::unique_ptr<cms::Session> session(connection->createSession(cms::Session::SESSION_TRANSACTED));
  std::unique_ptr<cms::Queue> queue(session->createQueue("testSessionCommitAfterConsumerClosed"));
  std::unique_ptr<cms::MessageConsumer> consumer(session->createConsumer(queue.get()));

  connection->start();

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  ASSERT_TRUE(message.get() != nullptr);

  consumer->close();
  session->commit();
}

void TransactionTest::TearDown() {}
