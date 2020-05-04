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
#include <climits>
#include "MessageTest.h"
#include "cms/Utils.h"

////////////////////////////////////////////////////////////////////////////////
void MessageTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testSendRecvCloneProperty) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<TextMessage> message(session->createTextMessage("text-body"));
  message->setBooleanProperty("boolean", false);
  message->setByteProperty("byte", 60);
  message->setDoubleProperty("double", 642.5643);
  message->setFloatProperty("float", 0.564F);
  message->setIntProperty("int", 65438746);
  message->setLongProperty("long", 0xFFFFFFFF0000000LL);
  message->setShortProperty("short", 512);
  message->setStringProperty("string", "This is a test String");
  message->setStringProperty("JMSXGroupID", "hello");

  int bufflen = 4;
  unsigned char buff[4] = {0x00, 0x01, 0x00, 0x02};
  unsigned char buff2[4] = {0x00};

  message->setBytesProperty("bytes", &buff[0], 4);

  // Send message
  producer->send(message.get());

  message.reset(dynamic_cast<TextMessage *>(consumer->receive(3000)));
  EXPECT_TRUE(message != nullptr);
  EXPECT_FALSE(message->getBooleanProperty("boolean"));
  EXPECT_TRUE(message->getByteProperty("byte") == 60);
  EXPECT_TRUE(message->getDoubleProperty("double") == 642.5643);
  EXPECT_TRUE(message->getFloatProperty("float") == 0.564F);
  EXPECT_TRUE(message->getIntProperty("int") == 65438746);
  EXPECT_TRUE(message->getLongProperty("long") == 0xFFFFFFFF0000000LL);
  EXPECT_TRUE(message->getShortProperty("short") == 512);
  EXPECT_TRUE(message->getStringProperty("string") == "This is a test String");
  EXPECT_TRUE(message->getStringProperty("JMSXGroupID") == "hello");

  bufflen = message->getBytesProperty("bytes", buff2, sizeof(buff2));
  EXPECT_TRUE(bufflen == sizeof(buff));
  EXPECT_TRUE(memcmp(&buff[0], &buff2[0], sizeof(buff)) == 0);

  std::unique_ptr<cms::TextMessage> clonedMessage((cms::TextMessage *)message->clone());
  EXPECT_TRUE(clonedMessage != nullptr);
  EXPECT_FALSE(clonedMessage->getBooleanProperty("boolean"));
  EXPECT_TRUE(clonedMessage->getByteProperty("byte") == 60);
  EXPECT_TRUE(clonedMessage->getDoubleProperty("double") == 642.5643);
  EXPECT_TRUE(clonedMessage->getFloatProperty("float") == 0.564F);
  EXPECT_TRUE(clonedMessage->getIntProperty("int") == 65438746);
  EXPECT_TRUE(clonedMessage->getLongProperty("long") == 0xFFFFFFFF0000000LL);
  EXPECT_TRUE(clonedMessage->getShortProperty("short") == 512);
  EXPECT_TRUE(clonedMessage->getStringProperty("string") == "This is a test String");
  EXPECT_TRUE(clonedMessage->getStringProperty("JMSXGroupID") == "hello");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testCopy) {
  cms::Session *session(cmsProvider->getSession());
  this->cmsCorrelationID = "testcorrelationid";
  Destination *destination = cmsProvider->getDestination();
  Destination *replyto = cmsProvider->getTempDestination();

  this->cmsDeliveryMode = DeliveryMode::NON_PERSISTENT;
  this->cmsRedelivered = true;
  this->cmsType = "test type";
  this->cmsPriority = 5;
  this->readOnlyMessage = false;

  std::unique_ptr<Message> msg1(session->createMessage());

  msg1->setCMSCorrelationID(this->cmsCorrelationID);

  msg1->setCMSDestination(destination);
  msg1->setCMSReplyTo(replyto);
  msg1->setCMSDeliveryMode(this->cmsDeliveryMode);
  msg1->setCMSRedelivered(this->cmsRedelivered);
  msg1->setCMSType(this->cmsType);
  msg1->setCMSExpiration(this->cmsExpiration);
  msg1->setCMSPriority(this->cmsPriority);
  msg1->setCMSTimestamp(this->cmsTimestamp);

  std::unique_ptr<Message> msg2(msg1->clone());

  EXPECT_TRUE(msg1->getCMSMessageID() == msg2->getCMSMessageID());
  EXPECT_TRUE(msg1->getCMSCorrelationID() == msg2->getCMSCorrelationID());
  EXPECT_TRUE(msg1->getCMSDestination() != nullptr);
  EXPECT_TRUE(msg2->getCMSDestination() != nullptr);
  EXPECT_TRUE(msg1->getCMSDestination()->equals(*msg2->getCMSDestination()));
  if (msg1->getCMSReplyTo() != nullptr) {
    EXPECT_TRUE(msg1->getCMSReplyTo()->equals(*msg2->getCMSReplyTo()));
  }
  EXPECT_TRUE(msg1->getCMSDeliveryMode() == msg2->getCMSDeliveryMode());
  EXPECT_TRUE(msg1->getCMSRedelivered() == msg2->getCMSRedelivered());
  EXPECT_TRUE(msg1->getCMSType() == msg2->getCMSType());
  EXPECT_TRUE(msg1->getCMSExpiration() == msg2->getCMSExpiration());
  EXPECT_TRUE(msg1->getCMSPriority() == msg2->getCMSPriority());
  EXPECT_TRUE(msg1->getCMSTimestamp() == msg2->getCMSTimestamp());

  session->close();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testClearProperties) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  msg->setStringProperty("test", "test");
  msg->clearProperties();

  EXPECT_TRUE(!msg->propertyExists("test"));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testPropertyExists) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  msg->setStringProperty("test", "test");

  EXPECT_TRUE(msg->propertyExists("test"));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetBooleanProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "booleanProperty";
  msg->setBooleanProperty(name, true);

  EXPECT_TRUE(msg->getBooleanProperty(name));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetByteProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "byteProperty";
  msg->setByteProperty(name, (unsigned char)1);

  EXPECT_TRUE(msg->getByteProperty(name) == 1);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetShortProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "shortProperty";
  msg->setShortProperty(name, (short)1);

  EXPECT_TRUE(msg->getShortProperty(name) == 1);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetIntProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "intProperty";
  msg->setIntProperty(name, 1);

  EXPECT_TRUE(msg->getIntProperty(name) == 1);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetLongProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "longProperty";
  msg->setLongProperty(name, 1);

  EXPECT_TRUE(msg->getLongProperty(name) == 1);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetFloatProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "floatProperty";
  msg->setFloatProperty(name, 1.3F);

  EXPECT_FLOAT_EQ(msg->getFloatProperty(name), 1.3F);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetDoubleProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "doubleProperty";
  msg->setDoubleProperty(name, 1.3);

  EXPECT_DOUBLE_EQ(msg->getDoubleProperty(name), 1.3);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetStringProperty) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "stringProperty";
  msg->setStringProperty(name, name);

  EXPECT_TRUE(msg->getStringProperty(name) == name);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetPropertyNames) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string name = "floatProperty";
  msg->setFloatProperty(name, 1.3F);

  std::vector<std::string> propertyNames = msg->getPropertyNames();

  for (const auto &iter : propertyNames) {
    EXPECT_TRUE(iter == name);
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testSetEmptyPropertyName) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  EXPECT_THROW(msg->setStringProperty("", "Cheese"), cms::CMSException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testGetAndSetCMSXDeliveryCount) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  msg->setIntProperty("CMSXDeliveryCount", 1);
  int count = msg->getIntProperty("CMSXDeliveryCount");
  EXPECT_TRUE(count == 1) << "expected delivery count = 1";
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testBooleanPropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName1 = "property1";
  std::string propertyName2 = "property2";
  msg->setBooleanProperty(propertyName1, true);
  msg->setBooleanProperty(propertyName2, false);

  EXPECT_TRUE(msg->getBooleanProperty(propertyName1));
  EXPECT_TRUE(msg->getStringProperty(propertyName1) == "true");

  EXPECT_TRUE(!msg->getBooleanProperty(propertyName2));
  EXPECT_TRUE(msg->getStringProperty(propertyName2) == "false");

  EXPECT_THROW(msg->getByteProperty(propertyName1), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getShortProperty(propertyName1), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getIntProperty(propertyName1), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getLongProperty(propertyName1), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getFloatProperty(propertyName1), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getDoubleProperty(propertyName1), cms::MessageFormatException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testBytePropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName = "property";
  msg->setByteProperty(propertyName, (unsigned char)1);

  EXPECT_TRUE(msg->getByteProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getShortProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getIntProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getLongProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getStringProperty(propertyName) == "1");

  EXPECT_THROW(msg->getBooleanProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getFloatProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getDoubleProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testShortPropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName = "property";
  msg->setShortProperty(propertyName, (short)1);

  EXPECT_TRUE(msg->getShortProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getIntProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getLongProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getStringProperty(propertyName) == "1");

  EXPECT_THROW(msg->getBooleanProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getByteProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getFloatProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getDoubleProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testIntPropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName = "property";
  const int digit = (int)SHRT_MAX + 1;
  msg->setIntProperty(propertyName, digit);

  EXPECT_TRUE(msg->getIntProperty(propertyName) == digit);
  EXPECT_TRUE(msg->getLongProperty(propertyName) == digit);
  EXPECT_TRUE(msg->getStringProperty(propertyName) == std::to_string(digit));

  EXPECT_THROW(msg->getBooleanProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getByteProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getShortProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getFloatProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getDoubleProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testLongPropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName = "property";
  msg->setLongProperty(propertyName, 1);

  EXPECT_TRUE(msg->getLongProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getStringProperty(propertyName) == "1");

  EXPECT_THROW(msg->getBooleanProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getByteProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getShortProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getIntProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getFloatProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getDoubleProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testFloatPropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName = "property";
  msg->setFloatProperty(propertyName, (float)1.5);

  EXPECT_FLOAT_EQ(msg->getFloatProperty(propertyName), 1.5F);
  EXPECT_DOUBLE_EQ(msg->getDoubleProperty(propertyName), 1.5);
  EXPECT_TRUE(msg->getStringProperty(propertyName) == "1.5");

  EXPECT_THROW(msg->getBooleanProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getByteProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getShortProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getIntProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getLongProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testDoublePropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName = "property";
  msg->setDoubleProperty(propertyName, 1.5);

  EXPECT_DOUBLE_EQ(msg->getDoubleProperty(propertyName), 1.5);
  EXPECT_TRUE(msg->getStringProperty(propertyName) == "1.5");

  EXPECT_THROW(msg->getBooleanProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getByteProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getShortProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getIntProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getLongProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getFloatProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MessageTest, testStringPropertyConversion) {
  cms::Session *session(cmsProvider->getSession());
  std::unique_ptr<Message> msg(session->createMessage());

  std::string propertyName = "property";
  std::string stringValue = "true";

  msg->setStringProperty(propertyName, stringValue);

  EXPECT_TRUE(msg->getStringProperty(propertyName) == stringValue);
  EXPECT_TRUE(msg->getBooleanProperty(propertyName));

  stringValue = "1";
  msg->setStringProperty(propertyName, stringValue);
  EXPECT_TRUE(msg->getByteProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getShortProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getIntProperty(propertyName) == 1);
  EXPECT_TRUE(msg->getLongProperty(propertyName) == 1);

  stringValue = "1.5";
  msg->setStringProperty(propertyName, stringValue);
  EXPECT_FLOAT_EQ(msg->getFloatProperty(propertyName), 1.5F);
  EXPECT_DOUBLE_EQ(msg->getDoubleProperty(propertyName), 1.5);

  stringValue = "bad";
  msg->setStringProperty(propertyName, stringValue);

  EXPECT_THROW(msg->getByteProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getShortProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getIntProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getLongProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getFloatProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");
  EXPECT_THROW(msg->getDoubleProperty(propertyName), cms::MessageFormatException) << ("Should have thrown exception");

  EXPECT_FALSE(msg->getBooleanProperty(propertyName));
}

void MessageTest::TearDown() {}
