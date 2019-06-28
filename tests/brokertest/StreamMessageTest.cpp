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
#include "StreamMessageTest.h"

////////////////////////////////////////////////////////////////////////////////
void StreamMessageTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testSetAndGet) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> myMessage(session->createStreamMessage());

  std::vector<unsigned char> data;
  data.push_back(2);
  data.push_back(4);
  data.push_back(8);
  data.push_back(16);
  data.push_back(32);
  std::vector<unsigned char> readData(data.size());

  myMessage->writeBoolean(false);
  myMessage->writeByte(127);
  myMessage->writeChar('a');
  myMessage->writeShort(32000);
  myMessage->writeInt(6789999);
  myMessage->writeLong(0xFFFAAA33345LL);
  myMessage->writeFloat(0.000012F);
  myMessage->writeDouble(64.54654);
  myMessage->writeBytes(data);

  myMessage->reset();

  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::BOOLEAN_TYPE);
  EXPECT_FALSE(myMessage->readBoolean());
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::BYTE_TYPE);
  EXPECT_TRUE(myMessage->readByte() == 127);
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::CHAR_TYPE);
  EXPECT_TRUE(myMessage->readChar() == 'a');
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::SHORT_TYPE);
  EXPECT_TRUE(myMessage->readShort() == 32000);
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::INTEGER_TYPE);
  EXPECT_TRUE(myMessage->readInt() == 6789999);
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::LONG_TYPE);
  EXPECT_TRUE(myMessage->readLong() == 0xFFFAAA33345LL);
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::FLOAT_TYPE);
  EXPECT_TRUE(myMessage->readFloat() == 0.000012F);
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::DOUBLE_TYPE);
  EXPECT_TRUE(myMessage->readDouble() == 64.54654);
  EXPECT_TRUE(myMessage->getNextValueType() == cms::Message::BYTE_ARRAY_TYPE);
  EXPECT_TRUE(myMessage->readBytes(readData) == (int)data.size());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadBoolean) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    msg->writeBoolean(true);
    msg->reset();
    EXPECT_TRUE(msg->readBoolean());
    msg->reset();
    EXPECT_TRUE(msg->readString() == "true");
    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readShort(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readInt(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readLong(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readDouble(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadByte) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    unsigned char test = 4;
    msg->writeByte(test);
    msg->reset();
    EXPECT_TRUE(msg->readByte() == test);
    msg->reset();
    EXPECT_TRUE(msg->readShort() == test);
    msg->reset();
    EXPECT_TRUE(msg->readInt() == test);
    msg->reset();
    EXPECT_TRUE(msg->readLong() == test);

    msg->reset();
    EXPECT_THROW(msg->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readDouble(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadShort) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    short test = (short)4;
    msg->writeShort(test);
    msg->reset();
    EXPECT_TRUE(msg->readShort() == test);
    msg->reset();
    EXPECT_TRUE(msg->readInt() == test);
    msg->reset();
    EXPECT_TRUE(msg->readLong() == test);

    msg->reset();
    EXPECT_THROW(msg->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readDouble(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadChar) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    char test = 'z';
    msg->writeChar(test);
    msg->reset();
    EXPECT_TRUE(msg->readChar() == test);

    msg->reset();
    EXPECT_THROW(msg->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readShort(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readInt(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readLong(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readDouble(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadInt) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    int test = 4;
    msg->writeInt(test);
    msg->reset();
    EXPECT_TRUE(msg->readInt() == test);
    msg->reset();
    EXPECT_TRUE(msg->readLong() == test);

    msg->reset();
    EXPECT_THROW(msg->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readShort(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readDouble(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadLong) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    long test = 4L;
    msg->writeLong(test);
    msg->reset();
    EXPECT_TRUE(msg->readLong() == test);
    msg->reset();

    msg->reset();
    EXPECT_THROW(msg->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readShort(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readInt(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readDouble(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadFloat) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    float test = 4.4F;
    msg->writeFloat(test);
    msg->reset();
    EXPECT_TRUE(msg->readFloat() == test);
    msg->reset();
    EXPECT_TRUE(msg->readDouble() == test);
    msg->reset();
    EXPECT_THROW(msg->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readShort(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readInt(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readLong(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadDouble) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    double test = 4.4;
    msg->writeDouble(test);
    msg->reset();
    EXPECT_TRUE(msg->readDouble() == test);
    msg->reset();
    EXPECT_THROW(msg->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readShort(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readInt(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readLong(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadString) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    unsigned char testByte = 2;
    msg->reset();
    EXPECT_TRUE(msg->readByte() == testByte);
    msg->clearBody();
    short testShort = 3;
    msg->reset();
    EXPECT_TRUE(msg->readShort() == testShort);
    msg->clearBody();
    int testInt = 4;
    msg->reset();
    EXPECT_TRUE(msg->readInt() == testInt);
    msg->clearBody();
    long testLong = 6L;
    msg->reset();
    EXPECT_TRUE(msg->readLong() == testLong);
    msg->clearBody();
    float testFloat = 6.6F;
    msg->reset();
    EXPECT_TRUE(msg->readFloat() == testFloat);
    msg->clearBody();
    double testDouble = 7.7;
    msg->reset();
    EXPECT_DOUBLE_EQ(testDouble, msg->readDouble());
    msg->clearBody();
    msg->writeString("true");
    msg->reset();
    EXPECT_TRUE(msg->readBoolean());
    msg->clearBody();
    msg->writeString("a");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->clearBody();
    msg->writeString("777");
    msg->reset();
    EXPECT_THROW(msg->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadBigString) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    // Test with a 1Meg String
    std::string bigString;
    bigString.reserve(1024 * 1024);
    for (int i = 0; i < 1024 * 1024; i++) {
      bigString.append(1, (char)'a' + i % 26);
    }

    msg->writeString(bigString);
    msg->reset();
    EXPECT_EQ(bigString, msg->readString());

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadBytes) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> msg(session->createStreamMessage());

  try {
    unsigned char test[50];
    for (int i = 0; i < 50; i++) {
      test[i] = (unsigned char)i;
    }
    msg->writeBytes(test, 0, 50);
    msg->reset();

    unsigned char valid[50];
    msg->readBytes(valid, 50);
    for (int i = 0; i < 50; i++) {
      EXPECT_TRUE(valid[i] == test[i]);
    }

    msg->reset();
    EXPECT_THROW(msg->readByte(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readShort(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readInt(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readLong(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readChar(), cms::MessageFormatException) << ("should have thrown exception");
    msg->reset();
    EXPECT_THROW(msg->readString(), cms::MessageFormatException) << ("should have thrown exception");

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testClearBody) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> streamMessage(session->createStreamMessage());

  try {
    streamMessage->writeLong(2LL);
    streamMessage->clearBody();
    streamMessage->writeLong(2LL);
    streamMessage->readLong();
    EXPECT_TRUE(false) << ("should throw exception");

  } catch (MessageNotReadableException &) {
  } catch (MessageNotWriteableException &) {
    EXPECT_TRUE(false) << ("should be writeable");
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReset) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> streamMessage(session->createStreamMessage());

  try {
    streamMessage->writeDouble(24.5);
    streamMessage->writeLong(311LL);
  } catch (MessageNotWriteableException &) {
    EXPECT_TRUE(false) << ("should be writeable");
  }

  streamMessage->reset();

  try {
    // debug EXPECT_TRUE(streamMessage->isReadOnlyBody());
    EXPECT_DOUBLE_EQ(streamMessage->readDouble(), 24.5);
    EXPECT_EQ(streamMessage->readLong(), 311LL);
  } catch (MessageNotReadableException &) {
    EXPECT_TRUE(false) << ("should be readable");
  }

  EXPECT_THROW(streamMessage->writeInt(33), cms::MessageNotWriteableException) << ("should throw exception");
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testReadOnlyBody) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> message(session->createStreamMessage());

  try {
    message->writeBoolean(true);
    message->writeByte((unsigned char)1);
    message->writeChar('a');
    message->writeDouble(121.5);
    message->writeFloat(1.5F);
    message->writeInt(1);
    message->writeLong(1);
    message->writeShort((short)1);
    message->writeString("string");
  } catch (MessageNotWriteableException &) {
    EXPECT_TRUE(false) << ("should be writeable");
  }
  message->reset();
  try {
    message->readBoolean();
    message->readByte();
    message->readChar();
    message->readDouble();
    message->readFloat();
    message->readInt();
    message->readLong();
    message->readShort();
    message->readString();
  } catch (MessageNotReadableException &) {
    EXPECT_TRUE(false) << ("should be readable");
  }
  EXPECT_THROW(message->writeBoolean(true), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeByte((unsigned char)1), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeBytes(buffer), cms::MessageNotWriteableException) << ("should have thrown exception");
  unsigned char test[3];
  EXPECT_THROW(message->writeBytes(test, 0, 2), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeChar('a'), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeDouble(1.5), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeFloat(1.5F), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeInt(1), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeLong(1), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeShort((short)1), cms::MessageNotWriteableException) << ("should have thrown exception");
  EXPECT_THROW(message->writeString("string"), cms::MessageNotWriteableException) << ("should have thrown exception");
}

//////////////////////////////////////////////////////////////////////////////////
TEST_F(StreamMessageTest, DISABLED_testWriteOnlyBody) {
  Session *session = cmsProvider->getSession();
  std::unique_ptr<StreamMessage> message(session->createStreamMessage());

  message->clearBody();
  try {
    message->writeBoolean(true);
    message->writeByte((unsigned char)1);
    message->writeBytes(buffer);
    message->writeChar('a');
    message->writeDouble(1.5);
    message->writeFloat((float)1.5);
    message->writeInt(1);
    message->writeLong(1);
    message->writeShort((short)1);
    message->writeString("string");
  } catch (MessageNotWriteableException &) {
    EXPECT_TRUE(false) << ("should be writeable");
  }
  EXPECT_THROW(message->getNextValueType(), cms::MessageNotReadableException) << ("should have thrown exception");

  EXPECT_THROW(message->readBoolean(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readByte(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readBytes(buffer), cms::MessageFormatException) << ("should have thrown exception");
  unsigned char test[50];
  EXPECT_THROW(message->readBytes(test, 50), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readChar(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readFloat(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readDouble(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readInt(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readLong(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readString(), cms::MessageFormatException) << ("should have thrown exception");
  EXPECT_THROW(message->readShort(), cms::MessageFormatException) << ("should have thrown exception");
}

void StreamMessageTest::TearDown() {}
