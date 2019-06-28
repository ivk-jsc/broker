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

#ifndef __BytesMessageImpl_H__
#define __BytesMessageImpl_H__

#include <cms/BytesMessage.h>
#include <memory>
#include "MessageTemplate.h"
#include "ProtoHeader.h"

using namespace std;

class BytesMessageImpl : public MessageTemplate<cms::BytesMessage> {
 public:
  BytesMessageImpl();
  BytesMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr);
  BytesMessageImpl(const UPMQCommand &command);
  BytesMessageImpl(BytesMessageImpl &other);

  virtual ~BytesMessageImpl() throw();

  void setBodyBytes(const unsigned char *buffer, int numBytes) override;

  unsigned char *getBodyBytes() const override;

  void setBodyBytesFromStream(std::istream &input) override;

  void getBodyBytesToStream(std::ostream &output) const override;

  int getBodyLength() const override;

  void reset() override;

  bool readBoolean() const override;

  void writeBoolean(bool value) override;

  unsigned char readByte() const override;

  void writeByte(unsigned char value) override;

  int readBytes(std::vector<unsigned char> &value) const override;

  void writeBytes(const std::vector<unsigned char> &value) override;

  int readBytes(unsigned char *buffer, int length) const override;

  void writeBytes(const unsigned char *value, int offset, int length) override;

  char readChar() const override;

  void writeChar(char value) override;

  float readFloat() const override;

  void writeFloat(float value) override;

  double readDouble() const override;

  void writeDouble(double value) override;

  short readShort() const override;

  void writeShort(short value) override;

  unsigned short readUnsignedShort() const override;

  void writeUnsignedShort(unsigned short value) override;

  int readInt() const override;

  void writeInt(int value) override;

  long long readLong() const override;

  void writeLong(long long value) override;

  std::string readString() const override;

  void writeString(const std::string &value) override;

  std::string readUTF() const override;

  void writeUTF(const std::string &value) override;

  cms::BytesMessage *clone() const override;

 private:
  void appendValue(char *value, size_t count);
};

#endif /*__BytesMessageImpl_H__*/
