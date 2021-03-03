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
#include "AsyncSenderTest.h"

#include "CMSListener.h"

////////////////////////////////////////////////////////////////////////////////
void AsyncSenderTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(AsyncSenderTest, testAsyncSends) {
  cms::Session *session(cmsProvider->getSession());

  CMSListener listener(session);

  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  consumer->stop();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
  std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));
  std::unique_ptr<cms::TextMessage> txtMessage2(session->createTextMessage("TEST MESSAGE 2"));

  for (unsigned int i = 0; i < IntegrationCommon::defaultMsgCount; ++i) {
    EXPECT_NO_THROW(producer->send(txtMessage.get()));
  }

  for (unsigned int i = 0; i < IntegrationCommon::defaultMsgCount; ++i) {
    EXPECT_NO_THROW(producer->send(txtMessage2.get()));
  }

  EXPECT_NO_THROW(consumer->setMessageListener(&listener));

  EXPECT_NO_THROW(listener.asyncWaitForMessages(IntegrationCommon::defaultMsgCount * 2, cmsProvider->minTimeout));

  const unsigned int numReceived = listener.getNumReceived();
  EXPECT_TRUE(numReceived == IntegrationCommon::defaultMsgCount * 2);
}

void AsyncSenderTest::TearDown() {}
