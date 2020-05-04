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
#include "TextMessageTest.h"

////////////////////////////////////////////////////////////////////////////////
void TextMessageTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TextMessageTest, testSendRecvCloneTextMessage) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<TextMessage> message(session->createTextMessage("TEST TEXT"));
  message->setStringProperty("string", "TEST PROPERTY");

  // Send message
  producer->send(message.get());

  message.reset((cms::TextMessage *)consumer->receive(3000));
  EXPECT_TRUE(message != nullptr);
  EXPECT_TRUE(message->getText() == "TEST TEXT");
  EXPECT_TRUE(message->getStringProperty("string") == "TEST PROPERTY");

  std::unique_ptr<cms::TextMessage> clonedMessage((cms::TextMessage *)message->clone());
  EXPECT_TRUE(clonedMessage != nullptr);
  EXPECT_TRUE(clonedMessage->getText() == "TEST TEXT");
  EXPECT_TRUE(clonedMessage->getStringProperty("string") == "TEST PROPERTY");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(TextMessageTest, testClearBody) {
  const char *testText = "This is some test Text";

  Session *session = cmsProvider->getSession();
  std::unique_ptr<TextMessage> textMessage(session->createTextMessage());

  textMessage->setText(testText);

  textMessage->setReadable();

  EXPECT_TRUE(textMessage->getText() == testText);

  textMessage->clearBody();

  EXPECT_TRUE(textMessage->getText().empty());
}
////////////////////////////////////////////////////////////////////////////////

void TextMessageTest::TearDown() {}
