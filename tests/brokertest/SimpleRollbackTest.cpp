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
#include "SimpleRollbackTest.h"

#include "CMSListener.h"

////////////////////////////////////////////////////////////////////////////////
void SimpleRollbackTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL(), cms::Session::SESSION_TRANSACTED);
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SimpleRollbackTest, testRollbacks) {
  try {
    // Create CMS Object for Comms
    cms::Session *session(cmsProvider->getSession());

    CMSListener listener(session);

    cms::MessageConsumer *consumer = cmsProvider->getConsumer();
    consumer->setMessageListener(&listener);
    cms::MessageProducer *producer = cmsProvider->getProducer();
    producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

    std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage());

    for (unsigned int i = 0; i < IntegrationCommon::defaultMsgCount; ++i) {
      std::ostringstream lcStream;
      lcStream << "SimpleTest - Message #" << i << std::ends;
      txtMessage->setText(lcStream.str());
      producer->send(txtMessage.get());
    }

    session->commit();

    // Wait for the messages to get here
    listener.asyncWaitForMessages(IntegrationCommon::defaultMsgCount, cmsProvider->minTimeout);
    unsigned int numReceived = listener.getNumReceived();
    EXPECT_EQ(numReceived, IntegrationCommon::defaultMsgCount);

    session->commit();

    for (unsigned int i = 0; i < 5; ++i) {
      std::ostringstream lcStream;
      lcStream << "SimpleTest - Message #" << i << std::ends;
      txtMessage->setText(lcStream.str());
      producer->send(txtMessage.get());
    }

    listener.reset();
    session->rollback();

    listener.reset();
    txtMessage->setText("SimpleTest - Message after Rollback");
    producer->send(txtMessage.get());
    session->commit();

    // Wait for the messages to get here
    listener.asyncWaitForMessages(1, cmsProvider->minTimeout);
    EXPECT_TRUE(listener.getNumReceived() == 1);

    listener.reset();
    txtMessage->setText("SimpleTest - Message after Rollback");
    producer->send(txtMessage.get());
    session->commit();

    // Wait for the messages to get here
    listener.asyncWaitForMessages(1, cmsProvider->minTimeout);
    EXPECT_TRUE(listener.getNumReceived() == 1) << " but current value is " << listener.getNumReceived();
    session->commit();

  } catch (std::exception &ex) {
    EXPECT_TRUE(false) << ex.what();
  }
}
void SimpleRollbackTest::TearDown() {}
