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

#include "SelectorTest.h"
#include <cstdint>
#include "cms/Message.h"
#include <cms/InvalidSelectorException.h>
#include <fake_cpp14.h>

typedef int8_t byte;

using namespace cms;

////////////////////////////////////////////////////////////////////////////////
void SelectorTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testBasicSelectors) {
  assertSelector("name = 'John'", false);
  assertSelector("name = 'James'", true);
  assertSelector("rank = 123", true);
  assertSelector("rank = 124", false);
  assertSelector("rank > 100", true);
  assertSelector("rank < 124", true);
  assertSelector("rank > 124", false);
  assertSelector("rank < 123", false);
  assertSelector("rank >= 123", true);
  assertSelector("rank <= 124", true);
  assertSelector("rank >= 124", false);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testBooleanSelector) {
  assertSelector("(trueProp OR falseProp) AND trueProp", true);
  assertSelector("(trueProp OR falseProp) AND falseProp", false);
  assertSelector("trueProp", true);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testJMSPropertySelectors) {
  assertSelector("JMSType = 'selector-test'", true);
  assertSelector("JMSType = 'crap'", false);
  assertSelector("JMSType = 'selector-test' OR JMSType='crap'", true);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testPropertyTypes) {
  assertSelector("byteProp = 123", true);
  assertSelector("byteProp = 10", false);

  assertSelector("byteProp2 = 33", true);
  assertSelector("byteProp2 = 10", false);

  assertSelector("shortProp = 123", true);
  assertSelector("shortProp = 10", false);

  assertSelector("intProp = 123", true);
  assertSelector("intProp = 10", false);

  assertSelector("longProp = 123", true);
  assertSelector("longProp = 10", false);

  assertSelector("floatProp = 123.0", true);
  assertSelector("floatProp = 10.0", false);

  assertSelector("doubleProp = 123.0", true);
  assertSelector("doubleProp = 10.0", false);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testAndSelectors) {
  assertSelector("name = 'James' and rank < 200", true);
  assertSelector("name = 'James' and rank > 200", false);
  assertSelector("name = 'Foo' and rank < 200", false);
  assertSelector("unknown = 'Foo' and anotherUnknown < 200", false);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testOrSelectors) {
  assertSelector("name = 'James' or rank < 200", true);
  assertSelector("name = 'James' or rank > 200", true);
  assertSelector("name = 'Foo' or rank < 200", true);
  assertSelector("name = 'Foo' or rank > 200", false);
  assertSelector("unknown = 'Foo' or anotherUnknown < 200", false);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testBetween) {
  assertSelector("rank between 100 and 150", true);
  assertSelector("rank between 10 and 120", false);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testIn) {
  assertSelector("name in ('James', 'Bob', 'Gromit')", true);
  assertSelector("name in ('Bob', 'James', 'Gromit')", true);
  assertSelector("name in ('Gromit', 'Bob', 'James')", true);

  assertSelector("name in ('Gromit', 'Bob', 'Cheddar')", false);
  assertSelector("name not in ('Gromit', 'Bob', 'Cheddar')", true);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testIsNull) {
  assertSelector("dummy is null", true);
  assertSelector("dummy is not null", false);
  assertSelector("name is not null", true);
  assertSelector("name is null", false);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testStringQuoteParsing) { assertSelector("quote = '''In God We Trust'''", true); }

////////////////////////////////////////////////////////////////////////////////
TEST_F(SelectorTest, testLikeComparisons) {
  assertSelector("quote LIKE '''In G_d We Trust'''", true);
  assertSelector("quote LIKE '''In Gd_ We Trust'''", false);
  assertSelector("quote NOT LIKE '''In G_d We Trust'''", false);
  assertSelector("quote NOT LIKE '''In Gd_ We Trust'''", true);

  assertSelector("foo LIKE '%oo'", true);
  assertSelector("foo LIKE '%ar'", false);
  assertSelector("foo NOT LIKE '%oo'", false);
  assertSelector("foo NOT LIKE '%ar'", true);

  assertSelector("foo LIKE '!_%' ESCAPE '!'", true);
  assertSelector("quote LIKE '!_%' ESCAPE '!'", false);
  assertSelector("foo NOT LIKE '!_%' ESCAPE '!'", false);
  assertSelector("quote NOT LIKE '!_%' ESCAPE '!'", true);

  assertSelector("punctuation LIKE '!#$&()*+,-./:;<=>?@[\\]^`{|}~'", true);
}

TEST_F(SelectorTest, testInvalidSelector) { assertInvalidSelector("=TEST 'test'"); }

////////////////////////////////////////////////////////////////////////////////
void SelectorTest::assertInvalidSelector(const std::string &selector) {
  std::unique_ptr<cms::Session> session(cmsProvider->getConnection()->createSession());

  EXPECT_THROW(std::unique_ptr<cms::MessageConsumer> consumer(session->createConsumer(cmsProvider->getDestination(), selector)),
               cms::InvalidSelectorException)
      << "should throw InvalidSelectorException";
}

////////////////////////////////////////////////////////////////////////////////
void SelectorTest::assertSelector(const std::string &selector, bool expected) {
  std::unique_ptr<cms::Session> session(cmsProvider->getConnection()->createSession());
  std::unique_ptr<cms::MessageProducer> producer(session->createProducer(cmsProvider->getDestination()));
  std::unique_ptr<cms::MessageConsumer> consumer(session->createConsumer(cmsProvider->getDestination(), selector));

  std::unique_ptr<cms::Message> msg(session->createMessage());

  doMessage(msg.get());
  producer->send(msg.get());

  const std::unique_ptr<cms::Message> msgret(consumer->receive(3000));

  if (expected) {
    EXPECT_TRUE(msgret != nullptr);
  } else {
    doClean();
    EXPECT_TRUE(msgret == nullptr);
  }

  consumer->close();
  producer->close();
  session->close();
}

////////////////////////////////////////////////////////////////////////////////
void SelectorTest::doMessage(cms::Message *message) {
  message->setCMSType("selector-test");
  message->setStringProperty("name", "James");
  message->setStringProperty("location", "London");

  message->setByteProperty("byteProp", static_cast<byte>(123));
  message->setByteProperty("byteProp2", static_cast<byte>(33));
  message->setShortProperty("shortProp", static_cast<short>(123));
  message->setIntProperty("intProp", static_cast<int>(123));
  message->setLongProperty("longProp", static_cast<long>(123));
  message->setFloatProperty("floatProp", static_cast<float>(123.0f));
  message->setDoubleProperty("doubleProp", static_cast<double>(123.0));

  message->setIntProperty("rank", 123);
  message->setIntProperty("version", 2);
  message->setStringProperty("quote", "'In God We Trust'");
  message->setStringProperty("foo", "_foo");
  message->setStringProperty("punctuation", "!#$&()*+,-./:;<=>?@[\\]^`{|}~");
  message->setBooleanProperty("trueProp", true);
  message->setBooleanProperty("falseProp", false);
}

////////////////////////////////////////////////////////////////////////////////
void SelectorTest::doClean() {
  cms::Session *session(cmsProvider->getSession());
  (void)session;
  cms::MessageConsumer *consumer(cmsProvider->getConsumer());
  consumer->start();
  // lets consume any outstanding messages from previous test runs
  cms::Message *message;
  while ((message = consumer->receive(3000)) != nullptr) {
    delete message;
  }
  consumer->stop();
}
////////////////////////////////////////////////////////////////////////////////
void SelectorTest::TearDown() {}
