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

#include "MapMessageTest.h"

#include <algorithm>
#include <fake_cpp14.h>

////////////////////////////////////////////////////////////////////////////////
void MapMessageTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testSendRecvCloneMapMessage) {
  Session *session = cmsProvider->getSession();
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  std::unique_ptr<MapMessage> message(session->createMapMessage());

  EXPECT_TRUE(message->getMapNames().empty());
  EXPECT_FALSE(message->itemExists("Something"));

  std::vector<signed char> data;

  data.push_back(2);
  data.push_back(4);
  data.push_back(8);
  data.push_back(16);
  data.push_back(32);

  message->setBoolean("boolean", false);
  message->setByte("byte", 127);
  message->setChar("char", (unsigned short)'a');
  message->setShort("short", 32000);
  message->setInt("int", 6789999);
  message->setLong("long", 0xFFFAAA33345LL);
  message->setFloat("float", 0.000012F);
  message->setDouble("double", 64.54654);
  message->setBytes("bytes", data);

  producer->send(message.get());

  message.reset((cms::MapMessage *)consumer->receive(3000));
  EXPECT_TRUE(message != nullptr);

  EXPECT_FALSE(message->getBoolean("boolean"));
  EXPECT_TRUE(message->getByte("byte") == 127);
  EXPECT_TRUE(message->getChar("char") == (unsigned short)'a');
  EXPECT_TRUE(message->getShort("short") == 32000);
  EXPECT_TRUE(message->getInt("int") == 6789999);
  EXPECT_TRUE(message->getLong("long") == 0xFFFAAA33345LL);
  EXPECT_TRUE(message->getFloat("float") == 0.000012F);
  EXPECT_TRUE(message->getDouble("double") == 64.54654);
  EXPECT_TRUE(message->getBytes("bytes") == data);

  std::unique_ptr<cms::MapMessage> clonedMessage((cms::MapMessage *)message->clone());
  EXPECT_TRUE(clonedMessage != nullptr);

  EXPECT_FALSE(clonedMessage->getBoolean("boolean"));
  EXPECT_TRUE(clonedMessage->getByte("byte") == 127);
  EXPECT_TRUE(clonedMessage->getChar("char") == (unsigned short)'a');
  EXPECT_TRUE(clonedMessage->getShort("short") == 32000);
  EXPECT_TRUE(clonedMessage->getInt("int") == 6789999);
  EXPECT_TRUE(clonedMessage->getLong("long") == 0xFFFAAA33345LL);
  EXPECT_TRUE(clonedMessage->getFloat("float") == 0.000012F);
  EXPECT_TRUE(clonedMessage->getDouble("double") == 64.54654);
  EXPECT_TRUE(clonedMessage->getBytes("bytes") == data);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testEmptyMapSendReceive) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::MapMessage> mapMessage(session->createMapMessage());

  // Send some text messages
  producer->send(mapMessage.get());

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  EXPECT_TRUE(message != nullptr);

  auto *recvMapMessage = dynamic_cast<cms::MapMessage *>(message.get());
  EXPECT_TRUE(recvMapMessage != nullptr);
  EXPECT_FALSE(recvMapMessage->itemExists("SomeKey"));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testMapWithEmptyStringValue) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::MapMessage> mapMessage(session->createMapMessage());

  mapMessage->setString("String1", "");
  mapMessage->setString("String2", "value");

  // Send some text messages
  producer->send(mapMessage.get());

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  EXPECT_TRUE(message != nullptr);

  auto *recvMapMessage = dynamic_cast<MapMessage *>(message.get());
  EXPECT_TRUE(recvMapMessage != nullptr);
  EXPECT_TRUE(recvMapMessage->itemExists("String1"));
  EXPECT_TRUE(recvMapMessage->getString("String1").empty());
  EXPECT_TRUE(recvMapMessage->itemExists("String2"));
  EXPECT_FALSE(recvMapMessage->itemExists("String3"));
  EXPECT_TRUE(recvMapMessage->getString("String2") == std::string("value"));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testMapSetEmptyBytesVector) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  std::unique_ptr<cms::MapMessage> mapMessage(session->createMapMessage());

  std::vector<signed char> bytes;

  mapMessage->setBytes("BYTES", bytes);

  // Send some text messages
  producer->send(mapMessage.get());

  std::unique_ptr<cms::Message> message(consumer->receive(3000));
  EXPECT_TRUE(message != nullptr);

  auto *recvMapMessage = dynamic_cast<MapMessage *>(message.get());
  EXPECT_TRUE(recvMapMessage != nullptr);
  EXPECT_TRUE(recvMapMessage->itemExists("BYTES"));
  EXPECT_TRUE(recvMapMessage->getBytes("BYTES").empty());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testBytesConversion) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<MapMessage> msg(session->createMapMessage());

  std::vector<signed char> buffer(1);

  msg->setBoolean("boolean", true);
  msg->setByte("byte", (unsigned char)1);
  msg->setBytes("bytes", buffer);
  msg->setChar("char", (unsigned short)'a');
  msg->setDouble("double", 1.5);
  msg->setFloat("float", 1.5F);
  msg->setInt("int", 1);
  msg->setLong("long", 1);
  msg->setShort("short", (short)1);
  msg->setString("string", "string");

  // Test with a 1Meg String
  std::string bigString;

  bigString.reserve(1024 * 1024);
  for (int i = 0; i < 1024 * 1024; i++) {
    bigString += (char)((int)'a' + i % 26);
  }

  msg->setWritable();

  msg->setString("bigString", bigString);

  msg->setReadable();

  std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

  EXPECT_EQ(msg2->getBoolean("boolean"), true);
  EXPECT_EQ(msg2->getByte("byte"), (unsigned char)1);
  EXPECT_EQ(msg2->getBytes("bytes").size(), (std::size_t)1);
  EXPECT_TRUE(msg2->getChar("char") == (unsigned short)'a');
  EXPECT_DOUBLE_EQ(msg2->getDouble("double"), 1.5);
  EXPECT_FLOAT_EQ(msg2->getFloat("float"), 1.5F);
  EXPECT_EQ(msg2->getInt("int"), 1);
  EXPECT_EQ(msg2->getLong("long"), 1LL);
  EXPECT_EQ(msg2->getShort("short"), (short)1);
  EXPECT_EQ(msg2->getString("string"), std::string("string"));
  EXPECT_EQ(msg2->getString("bigString"), bigString);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetBoolean) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  msg->setBoolean(name, true);

  msg->setReadable();

  std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

  // EXPECT_TRUE(msg2->getBoolean(name));
  EXPECT_EQ(msg2->getBoolean(name), true);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetByte) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  msg->setByte(name, (unsigned char)1);

  msg->setReadable();

  std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

  EXPECT_TRUE(msg2->getByte(name) == (unsigned char)1);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetShort) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    msg->setShort(name, (short)1);

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getShort(name) == (short)1);

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetChar) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    msg->setChar(name, (unsigned short)'a');

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getChar(name) == (unsigned short)'a');

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetInt) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    msg->setInt(name, 1);

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getInt(name) == 1);

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetLong) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    msg->setLong(name, 1);

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getLong(name) == 1);

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetFloat) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    msg->setFloat(name, 1.5F);

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getFloat(name) == 1.5F);

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetDouble) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    msg->setDouble(name, 1.5);

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getDouble(name) == 1.5);

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetString) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    std::string str = "test";
    msg->setString(name, str);

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getString(name) == str);

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetBytes) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  try {
    std::vector<signed char> bytes1(3, 'a');
    std::vector<signed char> bytes2(2, 'b');

    msg->setBytes(name, bytes1);
    msg->setBytes(name + "2", bytes2);

    msg->setReadable();

    std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

    EXPECT_TRUE(msg2->getBytes(name) == bytes1);
    EXPECT_EQ(msg2->getBytes(name + "2").size(), bytes2.size());

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }

  MapMessage *msg3 = session->createMapMessage();
  msg3->setBytes("empty", std::vector<signed char>());
  msg3->setReadable();
  EXPECT_NO_THROW(msg3->getBytes("empty"));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testGetMapNames) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  std::vector<signed char> bytes1(3, 'a');
  std::vector<signed char> bytes2(2, 'b');

  msg->setBoolean("boolean", true);
  msg->setByte("byte", (unsigned char)1);
  msg->setBytes("bytes1", bytes1);
  msg->setBytes("bytes2", bytes2);
  msg->setChar("char", (unsigned short)'a');
  msg->setDouble("double", 1.5);
  msg->setFloat("float", 1.5F);
  msg->setInt("int", 1);
  msg->setLong("long", 1);
  msg->setShort("short", (short)1);
  msg->setString("string", "string");

  std::unique_ptr<cms::MapMessage> msg2((cms::MapMessage *)msg->clone());

  std::vector<std::string> mapNamesList = msg2->getMapNames();

  EXPECT_EQ((std::size_t)11, mapNamesList.size());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "boolean") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "byte") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "bytes1") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "bytes2") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "char") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "double") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "float") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "int") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "long") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "short") != mapNamesList.end());
  EXPECT_TRUE(std::find(mapNamesList.begin(), mapNamesList.end(), "string") != mapNamesList.end());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testItemExists) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> mapMessage(session->createMapMessage());

  mapMessage->setString("exists", "test");

  std::unique_ptr<cms::MapMessage> mapMessage2((cms::MapMessage *)mapMessage->clone());

  EXPECT_TRUE(mapMessage2->itemExists("exists"));
  EXPECT_TRUE(!mapMessage2->itemExists("doesntExist"));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testClearBody) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<cms::MapMessage> mapMessage(session->createMapMessage());

  mapMessage->setString("String", "String");
  mapMessage->clearBody();

  EXPECT_FALSE(mapMessage->itemExists("String"));
  mapMessage->clearBody();
  mapMessage->setString("String", "String");

  std::unique_ptr<cms::MapMessage> mapMessage2((cms::MapMessage *)mapMessage->clone());

  EXPECT_TRUE(mapMessage2->itemExists("String"));
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(MapMessageTest, testReadOnlyBody) {
  Session *session = cmsProvider->getSession();
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  std::unique_ptr<cms::MapMessage> msg(session->createMapMessage());

  std::vector<signed char> buffer(2);

  msg->setBoolean("boolean", true);
  msg->setByte("byte", (unsigned char)1);
  msg->setBytes("bytes", buffer);
  msg->setChar("char", (unsigned short)'a');
  msg->setDouble("double", 1.5);
  msg->setFloat("float", 1.5F);
  msg->setInt("int", 1);
  msg->setLong("long", 1);
  msg->setShort("short", (short)1);
  msg->setString("string", "string");

  producer->send(msg.get());

  msg.reset((cms::MapMessage *)consumer->receive(3000));
  EXPECT_TRUE(msg != nullptr);

  try {
    msg->getBoolean("boolean");
    msg->getByte("byte");
    msg->getBytes("bytes");
    msg->getChar("char");
    msg->getDouble("double");
    msg->getFloat("float");
    msg->getInt("int");
    msg->getLong("long");
    msg->getShort("short");
    msg->getString("string");
  } catch (MessageNotReadableException &) {
    EXPECT_TRUE(false) << ("should be readable");
  }

  EXPECT_THROW(msg->setBoolean("boolean", true), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setByte("byte", (unsigned char)1), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setBytes("bytes", buffer), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setChar("char", (unsigned short)'a'), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setDouble("double", 1.5), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setFloat("float", 1.5F), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setInt("int", 1), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setLong("long", 1), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setShort("short", (short)1), cms::MessageNotWriteableException) << ("should throw exception");
  EXPECT_THROW(msg->setString("string", "string"), cms::MessageNotWriteableException) << ("should throw exception");
}

void MapMessageTest::TearDown() {}
