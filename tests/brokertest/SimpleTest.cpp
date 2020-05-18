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

#include "SimpleTest.h"

#include "CMSListener.h"

#include <cms/Connection.h>
#include <cms/ConnectionFactory.h>
#include <cms/Session.h>
#include <fake_cpp14.h>
#include <list>
#include <array>
#include <algorithm>

using namespace cms;
////////////////////////////////////////////////////////////////////////////////
void SimpleTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testAutoAck) {
  cms::Session *session(cmsProvider->getSession());

  CMSListener listener(session);

  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE 1"));
  std::unique_ptr<cms::TextMessage> txt2Message(session->createTextMessage("TEST MESSAGE 2"));

  for (unsigned int i = 0; i < IntegrationCommon::defaultMsgCount; ++i) {
    producer->send(txtMessage.get());
  }

  for (unsigned int i = 0; i < IntegrationCommon::defaultMsgCount; ++i) {
    producer->send(txt2Message.get());
  }

  consumer->setMessageListener(&listener);

  listener.asyncWaitForMessages(IntegrationCommon::defaultMsgCount * 2);

  unsigned int numReceived = listener.getNumReceived();
  EXPECT_EQ(numReceived, (IntegrationCommon::defaultMsgCount * 2)) << "invalid order or count => " << listener.inputMessagesToString();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testClientAck) {
  cmsProvider->setAckMode(cms::Session::CLIENT_ACKNOWLEDGE);
  cmsProvider->reconnectSession();

  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());

  CMSListener listener(session);

  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::PERSISTENT);
  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE 1"));
  std::unique_ptr<cms::TextMessage> txt2Message(session->createTextMessage("TEST MESSAGE 2"));
  // std::unique_ptr<cms::BytesMessage> bytesMessage( session->createBytesMessage() );

  for (unsigned int i = 0; i < IntegrationCommon::defaultMsgCount; ++i) {
    producer->send(txtMessage.get());
  }

  for (unsigned int i = 0; i < IntegrationCommon::defaultMsgCount; ++i) {
    producer->send(txt2Message.get());
  }

  consumer->setMessageListener(&listener);

  // Wait for the messages to get here
  listener.asyncWaitForMessages(IntegrationCommon::defaultMsgCount * 2);

  unsigned int numReceived = listener.getNumReceived();
  EXPECT_EQ(numReceived, IntegrationCommon::defaultMsgCount * 2) << "invalid order or count => " << listener.inputMessagesToString();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testProducerWithNullDestination) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());

  CMSListener listener(session);

  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getNoDestProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));

  producer->send(cmsProvider->getDestination(), txtMessage.get());
  consumer->setMessageListener(&listener);
  // Wait for the messages to get here
  listener.asyncWaitForMessages(1);

  unsigned int numReceived = listener.getNumReceived();
  EXPECT_TRUE(numReceived == 1);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testProducerSendWithNullDestination) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());

  CMSListener listener(session);

  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));

  EXPECT_THROW(producer->send(nullptr, txtMessage.get()), cms::InvalidDestinationException) << "Should Throw an InvalidDestinationException";

  producer = cmsProvider->getNoDestProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  EXPECT_THROW(producer->send(nullptr, txtMessage.get()), cms::UnsupportedOperationException) << "Should Throw an UnsupportedOperationException";
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testProducerSendToNonDefaultDestination) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());

  CMSListener listener(session);

  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));
  std::unique_ptr<cms::Destination> destination(session->createTemporaryTopic());

  EXPECT_THROW(producer->send(destination.get(), txtMessage.get()), cms::UnsupportedOperationException)
      << "Should Throw an UnsupportedOperationException";

  session->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testSyncReceive) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));

  // Send some text messages
  producer->send(txtMessage.get());

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  EXPECT_TRUE(message != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testSyncReceiveClientAck) {
  cmsProvider->setAckMode(cms::Session::CLIENT_ACKNOWLEDGE);
  cmsProvider->reconnectSession();

  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::PERSISTENT);

  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));

  // Send some text messages
  producer->send(txtMessage.get());

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  message->acknowledge();
  EXPECT_TRUE(message.get() != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testMultipleConnections) {
  std::unique_ptr<cms::ConnectionFactory> factory(ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL()));
  std::unique_ptr<cms::Connection> connection1(factory->createConnection());
  connection1->start();
  std::unique_ptr<cms::Connection> connection2(factory->createConnection());
  connection2->start();

  // debug
  // EXPECT_TRUE(connection1->getClientID() != connection2->getClientID());

  std::unique_ptr<cms::Session> session1(connection1->createSession());
  std::unique_ptr<cms::Session> session2(connection2->createSession());

  std::unique_ptr<cms::Topic> topic(session1->createTemporaryTopic());  // debug UUID::randomUUID().toString()

  std::unique_ptr<cms::MessageConsumer> consumer1(session1->createConsumer(topic.get()));
  std::unique_ptr<cms::MessageConsumer> consumer2(session2->createConsumer(topic.get()));

  std::unique_ptr<cms::MessageProducer> producer(session2->createProducer(topic.get()));
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::TextMessage> textMessage(session2->createTextMessage("TEST MESSAGE"));

  // Send some text messages
  producer->send(textMessage.get());

  // std::unique_ptr<cms::Message> message;
  cms::Message *msg1 = nullptr;
  cms::Message *msg2 = nullptr;

  msg1 = consumer1->receive(3000);
  EXPECT_TRUE(msg1 != nullptr);

  msg2 = consumer2->receive(3000);
  EXPECT_TRUE(msg2 != nullptr);

  delete msg1;
  delete msg2;

  // Clean up if we can
  consumer1->close();
  consumer2->close();
  producer->close();
  session1->close();
  session2->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testMultipleSessions) {
  // Create CMS Object for Comms
  std::unique_ptr<cms::Session> session1(cmsProvider->getConnection()->createSession());
  std::unique_ptr<cms::Session> session2(cmsProvider->getConnection()->createSession());

  std::unique_ptr<cms::Topic> topic(session1->createTemporaryTopic());  // debug UUID::randomUUID().toString()

  std::unique_ptr<cms::MessageConsumer> consumer1(session1->createConsumer(topic.get()));
  std::unique_ptr<cms::MessageConsumer> consumer2(session2->createConsumer(topic.get()));

  std::unique_ptr<cms::MessageProducer> producer(session2->createProducer(topic.get()));
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::TextMessage> textMessage(session2->createTextMessage("TEST MESSAGE"));

  cms::Message *msg1 = nullptr;
  cms::Message *msg2 = nullptr;

  // Send some text messages
  producer->send(textMessage.get());

  msg1 = consumer1->receive(3000);
  EXPECT_TRUE(msg1 != nullptr);

  msg2 = consumer2->receive(3000);
  EXPECT_TRUE(msg2 != nullptr);

  delete msg1;
  delete msg2;

  // Clean up if we can
  consumer1->close();
  consumer2->close();
  producer->close();
  session1->close();
  session2->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testReceiveAlreadyInQueue) {
  // Create CMS Object for Comms
  std::unique_ptr<cms::ConnectionFactory> factory(ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL()));
  std::unique_ptr<cms::Connection> connection(factory->createConnection());
  std::unique_ptr<cms::Session> session(connection->createSession());
  std::unique_ptr<cms::Topic> topic(session->createTopic(CMSProvider::newUUID()));  // debug UUID::randomUUID().toString()
  std::unique_ptr<cms::MessageConsumer> consumer(session->createConsumer(topic.get()));
  std::unique_ptr<cms::MessageProducer> producer(session->createProducer(topic.get()));
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
  std::unique_ptr<cms::TextMessage> textMessage(session->createTextMessage("TEST MESSAGE"));

  // Send some text messages
  producer->send(textMessage.get());

  // cmsSleep(250);

  connection->start();

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  EXPECT_TRUE(message.get() != nullptr);

  // Clean up if we can
  consumer->close();
  producer->close();
  session->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testQuickCreateAndDestroy) {
  std::unique_ptr<cms::ConnectionFactory> factory(ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL()));
  std::unique_ptr<cms::Connection> connection(factory->createConnection());
  std::unique_ptr<cms::Session> session(connection->createSession());

  session.reset(nullptr);
  connection.reset(nullptr);

  connection.reset(factory->createConnection());
  session.reset(connection->createSession());
  connection->start();

  session.reset(nullptr);
  connection.reset(nullptr);

  for (int i = 0; i < 10; ++i) {
    CMSProvider lcmsProvider(this->getBrokerURL());  // this->getBrokerURL() ..cmsProvider->getBrokerURL()
    lcmsProvider.getSession();
    lcmsProvider.getConsumer();
    lcmsProvider.getProducer();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testBytesMessageSendRecv) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::BytesMessage> bytesMessage(session->createBytesMessage());

  bytesMessage->writeBoolean(true);
  bytesMessage->writeByte(127);
  bytesMessage->writeDouble(123456.789);
  bytesMessage->writeInt(65537);
  bytesMessage->writeString("TEST-STRING");

  // Send some text messages
  producer->send(bytesMessage.get());

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  EXPECT_TRUE(message.get() != nullptr);

  EXPECT_THROW(message->setStringProperty("FOO", "BAR"), cms::CMSException) << "Should throw an CMSException";

  auto *bytesMessage2 = dynamic_cast<cms::BytesMessage *>(message.get());
  EXPECT_TRUE(bytesMessage2 != nullptr);
  EXPECT_THROW(bytesMessage2->writeBoolean(false), cms::CMSException) << "Should throw an CMSException";

  EXPECT_TRUE(bytesMessage2->getBodyLength() > 0);

  unsigned char *result = bytesMessage2->getBodyBytes();
  EXPECT_TRUE(result != nullptr);

  bytesMessage2->reset();

  EXPECT_TRUE(bytesMessage2->readBoolean() == true);
  EXPECT_TRUE(bytesMessage2->readByte() == 127);
  EXPECT_TRUE(bytesMessage2->readDouble() == 123456.789);
  EXPECT_TRUE(bytesMessage2->readInt() == 65537);
  EXPECT_TRUE(bytesMessage2->readString() == "TEST-STRING");
}

////////////////////////////////////////////////////////////////////////////////
namespace {
class Listener : public cms::MessageListener {
 private:
  bool passed = false;
  bool triggered = false;

 public:
  Listener() = default;

  ~Listener() override = default;

  bool isPassed() { return passed; }

  bool isTriggered() { return triggered; }

  void onMessage(const cms::Message *message) final {
    try {
      triggered = true;
      const auto *bytesMessage = dynamic_cast<const cms::BytesMessage *>(message);

      EXPECT_TRUE(bytesMessage != nullptr);
      EXPECT_TRUE(bytesMessage->getBodyLength() > 0);

      unsigned char *result = bytesMessage->getBodyBytes();
      EXPECT_TRUE(result != nullptr);
      passed = true;
    } catch (...) {
      passed = false;
    }
  }
};
}  // namespace

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testBytesMessageSendRecvAsync) {
  Listener listener;

  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  consumer->setMessageListener(&listener);
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::BytesMessage> bytesMessage(session->createBytesMessage());

  bytesMessage->writeBoolean(true);
  bytesMessage->writeByte(127);
  bytesMessage->writeDouble(123456.789);
  bytesMessage->writeInt(65537);
  bytesMessage->writeString("TEST-STRING");

  // Send some text messages
  producer->send(bytesMessage.get());

  int count = 0;
  while (!listener.isTriggered() && ((count++) < 30)) {
    cmsSleep(100);
  }

  consumer->setMessageListener(nullptr);
  EXPECT_TRUE(listener.isPassed());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testNewCloseDelete) {
  cms::ConnectionFactory *factory = nullptr;
  cms::Connection *connection = nullptr;
  cms::Session *session = nullptr;
  cms::MessageProducer *producer = nullptr;
  cms::MessageConsumer *consumer = nullptr;

  factory = ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL());

  connection = factory->createConnection();
  connection->start();
  session = connection->createSession();
  producer = session->createProducer(cmsProvider->getDestination());
  consumer = session->createConsumer(cmsProvider->getDestination());

  consumer->close();
  producer->close();
  session->close();
  connection->close();

  delete consumer;
  delete producer;
  delete session;
  delete connection;

  connection = factory->createConnection();
  connection->start();
  session = connection->createSession();
  producer = session->createProducer(cmsProvider->getDestination());
  consumer = session->createConsumer(cmsProvider->getDestination());

  connection->close();
  session->close();
  consumer->close();
  producer->close();

  delete connection;
  delete session;
  delete consumer;
  delete producer;

  connection = factory->createConnection();
  connection->start();
  session = connection->createSession();
  producer = session->createProducer(cmsProvider->getDestination());
  consumer = session->createConsumer(cmsProvider->getDestination());

  delete connection;
  delete session;
  delete consumer;
  delete producer;
}
///////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testDeleteDestinationWithSubscribersFails) {
  std::unique_ptr<TemporaryQueue> queue(cmsProvider->getSession()->createTemporaryQueue());
  std::unique_ptr<MessageConsumer> consumer(cmsProvider->getSession()->createConsumer(queue.get()));

  // This message delivery should NOT work since the temp connection is now closed.
  EXPECT_THROW(queue->destroy(), CMSException) << "Should fail with CMSException as Subscribers are active";
  cmsProvider->getSession()->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testPublishFailsForDestoryedTempDestination) {
  std::unique_ptr<TemporaryQueue> queue(cmsProvider->getSession()->createTemporaryQueue());
  std::unique_ptr<MessageProducer> producer(cmsProvider->getSession()->createProducer(queue.get()));

  std::unique_ptr<TextMessage> message(cmsProvider->getSession()->createTextMessage("First"));
  producer->send(message.get());

  message.reset(cmsProvider->getSession()->createTextMessage("Second"));

  queue->destroy();
  cmsProvider->getSession()->close();

  EXPECT_THROW(producer->send(message.get()), IllegalStateException)
      << "Should throw a InvalidDestinationException since temp destination should not exist anymore.";
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testDuplicateClientID) {
  std::unique_ptr<cms::ConnectionFactory> factory(ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL()));

  std::unique_ptr<cms::Connection> connection1;
  std::unique_ptr<cms::Connection> connection2;

  // 1st test - set in createConnection
  connection1.reset(factory->createConnection("username", "password", "clientID"));
  EXPECT_THROW(connection2.reset(factory->createConnection("username", "password", "clientID")), cms::InvalidClientIdException)
      << "Should fail with InvalidClientIdException as duplicate clientID";
  connection1->close();
  if (connection2) {
    connection2->close();
  }

  // 2nd test - set after createConnection
  connection1.reset(factory->createConnection());
  // connection1->setClientID("client_ID");
  EXPECT_THROW(connection1->setClientID("client_ID"), cms::InvalidClientIdException)
      << "Should fail with InvalidClientIdException as duplicate clientID, because clientID was set into createConnection";
  connection2.reset(factory->createConnection());
  EXPECT_THROW(connection2->setClientID("client_ID"), cms::InvalidClientIdException)
      << "Should fail with InvalidClientIdException as duplicate clientID";
  connection1->close();
  if (connection2) {
    connection2->close();
  }
}

///////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testClientTimeToLive) {
  std::unique_ptr<TemporaryQueue> queue(cmsProvider->getSession()->createTemporaryQueue());
  std::unique_ptr<MessageProducer> producer(cmsProvider->getSession()->createProducer(queue.get()));

  std::unique_ptr<Message> message(cmsProvider->getSession()->createMessage());

  long long timeToLive = 10000;

  producer->send(message.get(), Session::AUTO_ACKNOWLEDGE, 7, timeToLive);

  EXPECT_TRUE(message->getCMSDestination() != nullptr);
  ASSERT_EQ((int)Session::AUTO_ACKNOWLEDGE, message->getCMSDeliveryMode());
  ASSERT_EQ(7, message->getCMSPriority());

  queue->destroy();
}

///////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testServerTimeToLive) {
  std::unique_ptr<TemporaryQueue> queue(cmsProvider->getSession()->createTemporaryQueue());
  std::unique_ptr<MessageProducer> producer(cmsProvider->getSession()->createProducer(queue.get()));
  std::unique_ptr<MessageConsumer> consumer(cmsProvider->getSession()->createConsumer(queue.get()));

  std::unique_ptr<Message> message(cmsProvider->getSession()->createMessage());

  long long timeToLive = 3000;

  producer->send(message.get(), Session::AUTO_ACKNOWLEDGE, 7, timeToLive);

  cmsSleep(static_cast<unsigned int>(timeToLive * 2));

  message.reset(consumer->receive(3000));
  EXPECT_TRUE(message.get() == nullptr);

  consumer->close();
  queue->destroy();
}

///////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, testPriority) {
  std::unique_ptr<TemporaryQueue> queue(cmsProvider->getSession()->createTemporaryQueue());
  std::unique_ptr<MessageProducer> producer(cmsProvider->getSession()->createProducer(queue.get()));

  std::unique_ptr<Message> message(cmsProvider->getSession()->createMessage());

  for (int i = 0; i < 10; i++) {
    producer->send(message.get(), Session::AUTO_ACKNOWLEDGE, i, 0);
  }

  std::unique_ptr<MessageConsumer> consumer(cmsProvider->getSession()->createConsumer(queue.get()));

  for (int j = 9; j >= 0; j--) {
    message.reset(consumer->receive(3000));
    EXPECT_TRUE(message.get() != nullptr);
    EXPECT_EQ(message->getCMSPriority(), j) << "invalid order msg : " << message->getCMSMessageID();
  }

  consumer->close();
  producer->close();
  queue->destroy();
  cmsProvider->getSession()->close();
}
TEST_F(SimpleTest, testRoundRobin) {
  std::unique_ptr<TemporaryQueue> queue(cmsProvider->getSession()->createTemporaryQueue());
  std::unique_ptr<MessageProducer> producer(cmsProvider->getSession()->createProducer(queue.get()));
  std::unique_ptr<MessageConsumer> consumer1(cmsProvider->getSession()->createConsumer(queue.get()));

  std::unique_ptr<cms::ConnectionFactory> connFactory(cmsProvider->getConnectionFactory()->createCMSConnectionFactory(cmsProvider->getBrokerURL()));
  std::unique_ptr<cms::Connection> connection(connFactory->createConnection());
  connection->start();
  std::unique_ptr<cms::Session> session(connection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
  std::unique_ptr<MessageConsumer> consumer2(session->createConsumer(queue.get()));

  for (int i = 0; i < 10; i++) {
    std::unique_ptr<Message> message(cmsProvider->getSession()->createMessage());
    message->setIntProperty("num", i);
    producer->send(message.get());
  }

  std::unique_ptr<Message> in1;
  std::unique_ptr<Message> in2;
  size_t in1Counter = 0;
  size_t in2Counter = 0;

  std::array<int, 10> items = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (int count(0); count < 5; ++count) {
    in1.reset(consumer2->receive(13000));
    in2.reset(consumer1->receive(13000));
    EXPECT_TRUE((in1 != nullptr || in2 != nullptr));
    if (in1) {
      ++in1Counter;
      int fromIn1 = in1->getIntProperty("num");
      EXPECT_TRUE(std::any_of(items.begin(), items.end(), [&fromIn1](int i) { return i == fromIn1; }))
          << "invalid order msg : " << in1->getCMSMessageID();
    }
    if (in2) {
      ++in2Counter;
      int fromIn2 = in2->getIntProperty("num");
      EXPECT_TRUE(std::any_of(items.begin(), items.end(), [&fromIn2](int i) { return i == fromIn2; }))
          << "invalid order msg : " << in2->getCMSMessageID();
    }
  }
  EXPECT_EQ((in1Counter + in2Counter), items.size())
      << "expected " << items.size() << " messages, but had received " << (in1Counter + in2Counter);

  consumer1->close();
  consumer2->close();
  producer->close();
  queue->destroy();
  cmsProvider->getSession()->close();
  session.reset();
  connection.reset();
  connFactory.reset();
}
///////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, DISABLED_testConnectionCount) {
  std::list<std::unique_ptr<cms::Connection>> connections;

  for (int i = 0; i < 1023; ++i) {
    EXPECT_NO_THROW(connections.emplace_back(cmsProvider->getConnectionFactory()->createConnection()));
  }
  EXPECT_ANY_THROW(connections.emplace_back(cmsProvider->getConnectionFactory()->createConnection()));
}
///////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, DISABLED_testSessionsCount) {
  std::list<std::unique_ptr<cms::Session>> sessions;

  cmsProvider->getSession()->close();

  for (int i = 0; i < 1024; ++i) {
    EXPECT_NO_THROW(sessions.emplace_back(cmsProvider->getConnection()->createSession()));
  }
  EXPECT_ANY_THROW(sessions.emplace_back(cmsProvider->getConnection()->createSession()));
}
///////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleTest, DISABLED_testDestinationCount) {
  std::list<std::unique_ptr<cms::TemporaryQueue>> destinations;

  cmsProvider->reconnectSession();

  for (int i = 0; i < 1023; ++i) {
    EXPECT_NO_THROW(destinations.emplace_back(cmsProvider->getSession()->createTemporaryQueue()));
  }
  EXPECT_ANY_THROW(destinations.emplace_back(cmsProvider->getSession()->createTemporaryQueue()));
  for (auto &dest : destinations) {
    dest->destroy();
  }
  cmsProvider->reconnectSession();
}
///////////////////////////////////////////////////////////////////////////////

void SimpleTest::TearDown() {}
