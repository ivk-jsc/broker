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
#include "DurableTest.h"

////////////////////////////////////////////////////////////////////////////////
void DurableTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(DurableTest, testDurableConsumer) {
  try {
    cms::Session *session(cmsProvider->getSession());
    std::unique_ptr<cms::Topic> topic(cmsProvider->getSession()->createTopic("durable_test"));
    std::unique_ptr<cms::MessageConsumer> consumer(cmsProvider->getSession()->createDurableConsumer(topic.get(), "subscription_name", ""));
    std::unique_ptr<cms::MessageProducer> producer(cmsProvider->getSession()->createProducer(topic.get()));

    // Send a text message to the consumer while its active
    std::unique_ptr<cms::TextMessage> txtMessage(session->createTextMessage("TEST MESSAGE"));
    producer->send(txtMessage.get());
    std::unique_ptr<cms::Message> received(consumer->receive(3000));

    EXPECT_TRUE(received != nullptr);

    consumer->close();
    producer->close();

    consumer.reset(nullptr);
    producer.reset(nullptr);

    cmsProvider->reconnectSession();

    session = cmsProvider->getSession();
    // producer = cmsProvider->getProducer();
    producer.reset(cmsProvider->getSession()->createProducer(topic.get()));

    // Send some messages while there is no consumer active.
    for (int i = 0; i < MSG_COUNT; ++i) {
      producer->send(txtMessage.get());
    }

    // consumer = cmsProvider->getConsumer();
    consumer.reset(cmsProvider->getSession()->createDurableConsumer(topic.get(), "subscription_name", ""));

    // Send some messages while there is no consumer active.
    for (int i = 0; i < MSG_COUNT; ++i) {
      producer->send(txtMessage.get());
    }

    for (int i = 0; i < MSG_COUNT * 2; i++) {
      received.reset(consumer->receive(3000));

      EXPECT_TRUE(received != nullptr) << "Failed to receive all messages in batch";
    }

    // Remove the subscription after the consumer is forcibly closed.
    // cmsProvider->unsubscribe();
    session->unsubscribe("subscription_name");
  } catch (CMSException &ex) {
    EXPECT_TRUE(false) << ex.getMessage() << '\n' << ex.getStackTraceString();
  }
}

void DurableTest::TearDown() {}
