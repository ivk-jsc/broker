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

#include <fake_cpp14.h>
#include "ClientAckTest.h"

////////////////////////////////////////////////////////////////////////////////
namespace {

class MyMessageListener : public cms::MessageListener {
 private:
  bool dontAck;

 public:
  explicit MyMessageListener(bool dontAck_ = false) : MessageListener(), dontAck(dontAck_) {}

  ~MyMessageListener() override = default;

  void onMessage(const Message *message) override {
    EXPECT_TRUE(message != nullptr);

    if (!dontAck) {
      try {
        message->acknowledge();
      } catch (CMSException &e) {
        e.printStackTrace();
      }
    }
  }
};
}  // namespace

////////////////////////////////////////////////////////////////////////////////
void ClientAckTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(ClientAckTest, testAckedMessageAreConsumed) {
  Connection *connection = this->cmsProvider->getConnection();
  connection->start();

  std::unique_ptr<Session> session(connection->createSession(Session::CLIENT_ACKNOWLEDGE));
  std::unique_ptr<Destination> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<MessageProducer> producer(session->createProducer(queue.get()));

  std::unique_ptr<TextMessage> msg1(session->createTextMessage("Hello"));
  producer->send(msg1.get());

  // Consume the message...
  std::unique_ptr<MessageConsumer> consumer(session->createConsumer(queue.get()));
  std::unique_ptr<Message> msg(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  msg->acknowledge();

  cmsSleep(1000);

  // Reset the session->
  session->close();
  session.reset(connection->createSession(Session::CLIENT_ACKNOWLEDGE));

  // Attempt to Consume the message...
  consumer.reset(session->createConsumer(queue.get()));
  msg.reset(consumer->receive(1000));
  EXPECT_TRUE(msg == nullptr) << "invalid behaviour => " << dynamic_cast<cms::TextMessage *>(msg.get())->getText();

  session->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(ClientAckTest, testLastMessageAcked) {
  Connection *connection = this->cmsProvider->getConnection();
  connection->start();

  std::unique_ptr<Session> session(connection->createSession(Session::CLIENT_ACKNOWLEDGE));
  std::unique_ptr<Destination> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<MessageProducer> producer(session->createProducer(queue.get()));

  std::unique_ptr<TextMessage> msg1(session->createTextMessage("Hello1"));
  std::unique_ptr<TextMessage> msg2(session->createTextMessage("Hello2"));
  std::unique_ptr<TextMessage> msg3(session->createTextMessage("Hello3"));
  EXPECT_NO_THROW(producer->send(msg1.get()));
  EXPECT_NO_THROW(producer->send(msg2.get()));
  EXPECT_NO_THROW(producer->send(msg3.get()));

  // Consume the message...
  std::unique_ptr<MessageConsumer> consumer;
  EXPECT_NO_THROW(consumer.reset(session->createConsumer(queue.get())));
  std::unique_ptr<Message> msg;
  EXPECT_NO_THROW(msg.reset(consumer->receive(3000)));
  EXPECT_TRUE(msg != nullptr);
  EXPECT_NO_THROW(msg.reset(consumer->receive(3000)));
  EXPECT_TRUE(msg != nullptr);
  EXPECT_NO_THROW(msg.reset(consumer->receive(3000)));
  EXPECT_TRUE(msg != nullptr);
  EXPECT_NO_THROW(msg->acknowledge());

  cmsSleep(3000);

  // Reset the session->
  EXPECT_NO_THROW(session->close());
  EXPECT_NO_THROW(session.reset(connection->createSession(Session::CLIENT_ACKNOWLEDGE)));

  // Attempt to Consume the message...
  EXPECT_NO_THROW(consumer.reset(session->createConsumer(queue.get())));
  msg.reset(nullptr);
  EXPECT_NO_THROW(msg.reset(consumer->receive(3000)));
  EXPECT_TRUE(msg == nullptr) << "expect empty message, but got " << msg->getCMSMessageID() << " : "
                              << dynamic_cast<TextMessage *>(msg.get())->getText();

  EXPECT_NO_THROW(session->close());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(ClientAckTest, testFirstMessageAcked) {
  Connection *connection = this->cmsProvider->getConnection();
  connection->start();

  std::unique_ptr<Session> session(connection->createSession(Session::CLIENT_ACKNOWLEDGE));
  std::unique_ptr<Destination> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<MessageProducer> producer(session->createProducer(queue.get()));

  std::unique_ptr<TextMessage> msg1(session->createTextMessage("Hello1"));
  std::unique_ptr<TextMessage> msg2(session->createTextMessage("Hello2"));
  std::unique_ptr<TextMessage> msg3(session->createTextMessage("Hello3"));
  producer->send(msg1.get());
  producer->send(msg2.get());
  producer->send(msg3.get());

  // Consume the message...
  std::unique_ptr<MessageConsumer> consumer(session->createConsumer(queue.get()));
  std::unique_ptr<Message> msg(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  auto *textMessage1 = dynamic_cast<TextMessage *>(msg.get());
  EXPECT_TRUE(textMessage1->getText() == "Hello1");

  session->close();

  // Reset the session->
  session.reset(connection->createSession(Session::CLIENT_ACKNOWLEDGE));

  // Attempt to Consume the message...
  consumer.reset(session->createConsumer(queue.get()));

  msg.reset(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  textMessage1 = dynamic_cast<TextMessage *>(msg.get());
  EXPECT_EQ(textMessage1->getText(), "Hello1");

  msg.reset(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  auto *textMessage2 = dynamic_cast<TextMessage *>(msg.get());
  EXPECT_EQ(textMessage2->getText(), "Hello2");

  msg.reset(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  auto *textMessage3 = dynamic_cast<TextMessage *>(msg.get());
  EXPECT_EQ(textMessage3->getText(), "Hello3");

  textMessage3->acknowledge();

  session->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(ClientAckTest, testUnAckedMessageAreNotConsumedOnSessionClose) {
  Connection *connection = this->cmsProvider->getConnection();
  connection->start();

  std::unique_ptr<Session> session(connection->createSession(Session::CLIENT_ACKNOWLEDGE));
  std::unique_ptr<Destination> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<MessageProducer> producer(session->createProducer(queue.get()));

  std::unique_ptr<TextMessage> msg1(session->createTextMessage("Hello"));
  producer->send(msg1.get());

  // Consume the message...
  std::unique_ptr<MessageConsumer> consumer(session->createConsumer(queue.get()));
  std::unique_ptr<Message> msg(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  // Don't ack the message.

  // Reset the session->  This should cause the unacknowledged message to be re-delivered.
  session->close();
  session.reset(connection->createSession(Session::CLIENT_ACKNOWLEDGE));

  // Attempt to Consume the message...
  consumer.reset(session->createConsumer(queue.get()));
  msg.reset(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  msg->acknowledge();

  session->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(ClientAckTest, testAckedMessageAreConsumedAsync) {
  Connection *connection = this->cmsProvider->getConnection();
  connection->start();

  MyMessageListener listener(false);

  std::unique_ptr<Session> session(connection->createSession(Session::CLIENT_ACKNOWLEDGE));
  std::unique_ptr<Destination> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<MessageProducer> producer(session->createProducer(queue.get()));

  std::unique_ptr<TextMessage> msg1(session->createTextMessage("Hello"));
  producer->send(msg1.get());

  // Consume the message...
  std::unique_ptr<MessageConsumer> consumer(session->createConsumer(queue.get()));
  consumer->setMessageListener(&listener);

  cmsSleep(3000);

  // Reset the session->
  session->close();

  session.reset(connection->createSession(Session::CLIENT_ACKNOWLEDGE));

  // Attempt to Consume the message...
  consumer.reset(session->createConsumer(queue.get()));
  std::unique_ptr<Message> msg(consumer->receive(3000));
  EXPECT_TRUE(msg == nullptr);

  session->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(ClientAckTest, testUnAckedMessageAreNotConsumedOnSessionCloseAsync) {
  Connection *connection = this->cmsProvider->getConnection();
  connection->start();

  // Don't send an ack
  MyMessageListener listener(true);

  std::unique_ptr<Session> session(connection->createSession(Session::CLIENT_ACKNOWLEDGE));
  std::unique_ptr<Destination> queue(session->createQueue(::testing::UnitTest::GetInstance()->current_test_info()->name()));
  std::unique_ptr<MessageProducer> producer(session->createProducer(queue.get()));

  std::unique_ptr<TextMessage> msg1(session->createTextMessage("Hello"));
  producer->send(msg1.get());

  // Consume the message...
  std::unique_ptr<MessageConsumer> consumer(session->createConsumer(queue.get()));
  consumer->setMessageListener(&listener);
  // Don't ack the message.

  // Reset the session-> This should cause the Unacked message to be redelivered.
  session->close();

  // cmsSleep(5000);
  session.reset(connection->createSession(Session::CLIENT_ACKNOWLEDGE));

  // Attempt to Consume the message...
  consumer.reset(session->createConsumer(queue.get()));
  std::unique_ptr<Message> msg(consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);
  msg->acknowledge();

  session->close();
}

void ClientAckTest::TearDown() {}
