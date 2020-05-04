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
#include "JmsMessageGroupTest.h"

////////////////////////////////////////////////////////////////////////////////
void JmsMessageGroupTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(JmsMessageGroupTest, testMessageSend) {
  std::string GROUPID = "TEST-GROUP-ID";

  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));
  txtMessage->setStringProperty("JMSXGroupID", GROUPID);

  // Send some text messages
  EXPECT_NO_THROW(producer->send(txtMessage.get()));

  std::unique_ptr<cms::Message> message;
  EXPECT_NO_THROW(message.reset(consumer->receive(3000)));
  EXPECT_TRUE(message != nullptr);
  EXPECT_TRUE(message->getStringProperty("JMSXGroupID") == GROUPID);
}

void JmsMessageGroupTest::TearDown() {}
