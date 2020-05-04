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
#include "ConnectionStartStopTest.h"

////////////////////////////////////////////////////////////////////////////////
void ConnectionStartStopTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());

  cmsProvider->cleanUpDestination();

  std::unique_ptr<ConnectionFactory> factory(ConnectionFactory::createCMSConnectionFactory(cmsProvider->getBrokerURL()));

  startedConnection.reset(factory->createConnection());
  startedConnection->start();
  stoppedConnection.reset(factory->createConnection());
}
void ConnectionStartStopTest::testStoppedConsumerHoldsMessagesTillStarted() {
  std::unique_ptr<Session> startedSession(startedConnection->createSession());
  std::unique_ptr<Session> stoppedSession(stoppedConnection->createSession());

  // Setup the consumers.
  std::unique_ptr<Topic> topic(startedSession->createTopic("test"));
  std::unique_ptr<MessageConsumer> startedConsumer(startedSession->createConsumer(topic.get()));
  std::unique_ptr<MessageConsumer> stoppedConsumer(stoppedSession->createConsumer(topic.get()));

  // Send the message.
  std::unique_ptr<MessageProducer> producer(startedSession->createProducer(topic.get()));
  std::unique_ptr<TextMessage> message(startedSession->createTextMessage("Hello"));
  producer->send(message.get());

  // Test the assertions.
  std::unique_ptr<Message> m(startedConsumer->receive(3000));
  EXPECT_TRUE(m != nullptr);

  m.reset(stoppedConsumer->receive(2000));
  EXPECT_TRUE(m == nullptr);

  stoppedConnection->start();
  m.reset(stoppedConsumer->receive(3000));
  EXPECT_TRUE(m != nullptr);

  startedSession->close();
  stoppedSession->close();
}
////////////////////////////////////////////////////////////////////////////////
TEST_F(ConnectionStartStopTest, testStoppedConsumerHoldsMessagesTillStarted) { testStoppedConsumerHoldsMessagesTillStarted(); }

////////////////////////////////////////////////////////////////////////////////
TEST_F(ConnectionStartStopTest, testMultipleConnectionStops) {
  testStoppedConsumerHoldsMessagesTillStarted();
  stoppedConnection->stop();
  testStoppedConsumerHoldsMessagesTillStarted();
  stoppedConnection->stop();
  testStoppedConsumerHoldsMessagesTillStarted();
}

void ConnectionStartStopTest::TearDown() {
  startedConnection->close();
  stoppedConnection->close();

  startedConnection.reset(nullptr);
  stoppedConnection.reset(nullptr);
}
