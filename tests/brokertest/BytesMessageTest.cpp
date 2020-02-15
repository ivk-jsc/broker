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

#include "BytesMessageTest.h"
#include <cstring>
#include <fake_cpp14.h>

////////////////////////////////////////////////////////////////////////////////
void BytesMessageTest::SetUp() {
  cmsProvider = std::make_unique<CMSProvider>(getBrokerURL());
  cmsProvider->cleanUpDestination();
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testSendRecvCloneBytesMessage) {
  // Create CMS Object for Comms
  cms::Session *session(cmsProvider->getSession());
  cms::MessageConsumer *consumer = cmsProvider->getConsumer();
  cms::MessageProducer *producer = cmsProvider->getProducer();
  producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

  unsigned char test[4];
  memcpy(test, "test", 4);

  cms::BytesMessage *message = session->createBytesMessage(test, 4);
  message->writeBoolean(false);
  message->writeBoolean(true);
  message->writeString("test2");
  message->writeLong(0);
  message->writeInt(1);

  producer->send(message);

  delete message;

  message = nullptr;
  message = (cms::BytesMessage *)consumer->receive(3000);
  EXPECT_TRUE(message != nullptr);

  unsigned char data[4];

  memset(&data[0], 0, 4);
  message->readBytes(&data[0], 4);
  EXPECT_TRUE(memcmp(data, test, 4) == 0);
  EXPECT_TRUE(message->readBoolean() == false);
  EXPECT_TRUE(message->readBoolean() == true);
  EXPECT_TRUE(message->readString() == "test2");
  EXPECT_TRUE(message->readLong() == 0);
  EXPECT_TRUE(message->readInt() == 1);

  auto *clonedMessage = (cms::BytesMessage *)message->clone();
  EXPECT_TRUE(clonedMessage != nullptr);

  memset(&data[0], 0, 4);
  clonedMessage->readBytes(&data[0], 4);
  EXPECT_TRUE(memcmp(data, test, 4) == 0);
  EXPECT_TRUE(clonedMessage->readBoolean() == false);
  EXPECT_TRUE(clonedMessage->readBoolean() == true);
  EXPECT_TRUE(clonedMessage->readString() == "test2");
  EXPECT_TRUE(clonedMessage->readLong() == 0);
  EXPECT_TRUE(clonedMessage->readInt() == 1);

  delete message;
  delete clonedMessage;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testGetBodyLength) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  int len = 10;

  try {
    for (int i = 0; i < len; i++) {
      msg->writeLong(9223372036854775807LL);
    }

  } catch (CMSException &ex) {
    ex.printStackTrace();
  }

  try {
    msg->reset();
    int resLen = msg->getBodyLength();
    EXPECT_TRUE(resLen == (len * static_cast<int>(sizeof(long long))));

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadBoolean) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeBoolean(true);
    msg->reset();
    EXPECT_TRUE(msg->readBoolean());
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadByte) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeByte((unsigned char)2);
    msg->reset();
    EXPECT_TRUE(msg->readByte() == 2);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadShort) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeShort((short)3000);
    msg->reset();
    EXPECT_TRUE(msg->readShort() == 3000);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadUnsignedShort) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeShort((short)3000);
    msg->reset();
    EXPECT_TRUE(msg->readUnsignedShort() == 3000);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadChar) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeChar('a');
    msg->reset();
    EXPECT_TRUE(msg->readChar() == 'a');
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadInt) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeInt(3000);
    msg->reset();
    EXPECT_TRUE(msg->readInt() == 3000);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadLong) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeLong(3000);
    msg->reset();
    EXPECT_TRUE(msg->readLong() == 3000);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadFloat) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeFloat(3.3F);
    msg->reset();
    EXPECT_TRUE(msg->readFloat() == 3.3F);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadDouble) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    msg->writeDouble(3.3);
    msg->reset();
    EXPECT_TRUE(msg->readDouble() == 3.3);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadString) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    std::string str = "this is a test";
    msg->writeUTF(str);
    msg->reset();
    EXPECT_TRUE(msg->readUTF() == str);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadUTF) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    std::string str = "this is a test";
    msg->writeUTF(str);
    msg->reset();
    EXPECT_TRUE(msg->readUTF() == str);
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadBytesbyteArray) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    unsigned char data[50];
    for (int i = 0; i < 50; i++) {
      data[i] = (unsigned char)i;
    }
    msg->writeBytes(&data[0], 0, 50);
    msg->reset();
    unsigned char test[50];
    msg->readBytes(test, 50);
    for (int i = 0; i < 50; i++) {
      EXPECT_TRUE(test[i] == i);
    }

    EXPECT_THROW(msg->readBytes(test, -1), cms::CMSException) << "Should have thrown a CMSException";

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadBytesbyteArray2) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    std::vector<unsigned char> data(50);
    for (int i = 0; i < 50; i++) {
      data.at(i) = (unsigned char)i;
    }
    msg->writeBytes(data);
    msg->reset();
    std::vector<unsigned char> test(50);
    msg->readBytes(test);
    for (int i = 0; i < 50; i++) {
      EXPECT_TRUE(test[i] == i);
    }

    EXPECT_THROW(msg->readBytes(test), cms::CMSException) << "Should have thrown a CMSException";

  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testGetBodyBytes) {
  Session *session = cmsProvider->getSession();
  BytesMessage *msg = session->createBytesMessage();

  try {
    unsigned char data[50];
    for (int i = 0; i < 50; i++) {
      data[i] = (unsigned char)i;
    }
    msg->setBodyBytes(data, 50);
    msg->reset();

    unsigned char *data2;
    data2 = msg->getBodyBytes();
    for (int i = 0; i < 50; i++) {
      EXPECT_TRUE(data[i] == data2[i]);
    }
  } catch (CMSException &ex) {
    ex.printStackTrace();
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete msg;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testClearBody) {
  Session *session = cmsProvider->getSession();
  BytesMessage *bytesMessage = session->createBytesMessage();

  try {
    bytesMessage->writeInt(1);
    bytesMessage->readInt();
    bytesMessage->clearBody();
    bytesMessage->writeInt(1);
    bytesMessage->readInt();
  } catch (const MessageNotReadableException &) {
  } catch (const MessageNotWriteableException &ex) {
    EXPECT_TRUE(false) << ex.getMessage();
  }
  delete bytesMessage;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReset) {
  Session *session = cmsProvider->getSession();
  BytesMessage *message = session->createBytesMessage();

  try {
    message->writeDouble(24.5);
    message->writeLong(311);
  } catch (const MessageNotWriteableException &) {
    EXPECT_TRUE(false) << ("should be writeable");
  }
  message->reset();
  try {
    EXPECT_DOUBLE_EQ(message->readDouble(), 24.5);
    EXPECT_EQ(message->readLong(), 311LL);
  } catch (const MessageNotReadableException &) {
    EXPECT_TRUE(false) << ("should be readable");
  }

  EXPECT_THROW(message->writeInt(33), cms::MessageNotWriteableException) << ("should throw exception");

  delete message;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testReadOnlyBody) {
  Session *session = cmsProvider->getSession();
  BytesMessage *message = session->createBytesMessage();

  std::vector<unsigned char> buffer(3);
  unsigned char array[2] = {0, 0};

  try {
    message->writeBoolean(true);
    message->writeByte((unsigned char)1);
    message->writeByte((unsigned char)1);
    message->writeBytes(buffer);
    message->writeBytes(&array[0], 0, 2);
    message->writeChar('a');
    message->writeDouble(1.5);
    message->writeFloat((float)1.5);
    message->writeInt(1);
    message->writeLong(1);
    message->writeString("stringobj");
    message->writeShort((short)1);
    message->writeShort((short)1);
    message->writeUTF("utfstring");
  } catch (const MessageNotWriteableException &) {
    EXPECT_TRUE(false) << ("should be writeable");
  }
  message->reset();
  try {
    message->readBoolean();
    message->readByte();
    message->readByte();
    message->readBytes(buffer);
    message->readBytes(&array[0], 2);
    message->readChar();
    message->readDouble();
    message->readFloat();
    message->readInt();
    message->readLong();
    message->readString();
    message->readShort();
    message->readUnsignedShort();
    message->readUTF();
  } catch (const MessageNotReadableException &) {
    EXPECT_TRUE(false) << ("should be readable");
  }

  EXPECT_THROW(message->writeBoolean(true), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeByte((unsigned char)1), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeBytes(buffer), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeBytes(&array[0], 0, 2), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeChar('a'), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeDouble(1.5), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeFloat(1.5F), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeInt(1), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeLong(1L), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeString("StringObject"), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeShort((short)1), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeUTF("UTFString"), cms::MessageNotWriteableException) << ("Should have thrown exception");
  EXPECT_THROW(message->writeBoolean(true), cms::MessageNotWriteableException) << ("Should have thrown exception");

  delete message;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BytesMessageTest, testWriteOnlyBody) {
  Session *session = cmsProvider->getSession();
  BytesMessage *message = session->createBytesMessage();

  message->clearBody();
  std::vector<unsigned char> buffer(3);
  unsigned char array[2];
  try {
    message->writeBoolean(true);
    message->writeByte((unsigned char)1);
    message->writeByte((unsigned char)1);
    message->writeBytes(buffer);
    message->writeBytes(&array[0], 0, 2);
    message->writeChar('a');
    message->writeDouble(1.5);
    message->writeFloat((float)1.5);
    message->writeInt(1);
    message->writeLong(1LL);
    message->writeString("stringobj");
    message->writeShort((short)1);
    message->writeShort((short)1);
    message->writeUTF("utfstring");
  } catch (const MessageNotWriteableException &) {
    EXPECT_TRUE(false) << ("should be writeable");
  }

  EXPECT_THROW(message->readBoolean(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readByte(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readBytes(buffer), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readBytes(&array[0], 2), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readChar(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readDouble(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readFloat(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readInt(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readLong(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readString(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readShort(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readUnsignedShort(), cms::MessageNotReadableException) << ("Should have thrown exception");
  EXPECT_THROW(message->readUTF(), cms::MessageNotReadableException) << ("Should have thrown exception");
  delete message;
}

void BytesMessageTest::TearDown() {}
