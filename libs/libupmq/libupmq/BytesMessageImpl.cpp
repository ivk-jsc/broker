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

#ifndef __BytesMessageImpl_CPP__
#define __BytesMessageImpl_CPP__

#include "BytesMessageImpl.h"
#include <cstdint>

BytesMessageImpl::BytesMessageImpl() : MessageTemplate<cms::BytesMessage>() {
  Proto::Message* mutableMessage = _header->mutable_message();
  mutableMessage->set_body_type(Proto::Body::kBytesBody);
  mutableMessage->set_sender_id("");
  mutableMessage->set_session_id("");
  mutableMessage->set_timestamp(0);
  _header->mutable_object_id()->assign("");
  _header->set_request_reply_id(-1);
  _body->mutable_bytes_body();  //-V773
}

BytesMessageImpl::BytesMessageImpl(BytesMessageImpl &other) : MessageTemplate<cms::BytesMessage>(other) {}

BytesMessageImpl::~BytesMessageImpl() throw() {}

BytesMessageImpl::BytesMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr)
    : MessageTemplate<cms::BytesMessage>(header, body_buff, body_size) {
  DECAF_UNUSED_VAR(pr);
}

BytesMessageImpl::BytesMessageImpl(const UPMQCommand &command) : MessageTemplate<cms::BytesMessage>(command) {}

void BytesMessageImpl::setBodyBytes(const unsigned char *buffer, int numBytes) {
  failIfReadOnlyBody();
  appendValue((char *)buffer, static_cast<size_t>(numBytes));
}

unsigned char *BytesMessageImpl::getBodyBytes() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }
  return (unsigned char *)_body->bytes_body().value().data();
}

void BytesMessageImpl::setBodyBytesFromStream(std::istream &input) {
  failIfReadOnlyBody();
  std::string *s = _body->mutable_bytes_body()->mutable_value();
  std::istream::pos_type position = input.tellg();
  input.seekg(0, std::ios_base::end);
  std::istream::pos_type size = input.tellg() - position;
  input.seekg(position);

  s->resize(static_cast<size_t>(size));
  input.read((char *)&s[0], size);
}

void BytesMessageImpl::getBodyBytesToStream(std::ostream &output) const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= static_cast<string::size_type>(getBodyLength())) {
    throw cms::MessageEOFException();
  }
  output.write(_body->bytes_body().value().data(), _body->bytes_body().value().size());
}

int BytesMessageImpl::getBodyLength() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();

  return static_cast<int>(_body->bytes_body().value().size());
}

void BytesMessageImpl::reset() {
  this->_messageOffset = 0;
  this->_bodyWritable = false;
  this->_bodyReadable = true;
}

bool BytesMessageImpl::readBoolean() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  uint32_t value = *(uint32_t *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(uint32_t);

  return static_cast<bool>(!!value);
}

void BytesMessageImpl::writeBoolean(bool value) {
  failIfReadOnlyBody();
  uint32_t bvalue = static_cast<uint32_t>(value);
  appendValue((char *)&bvalue, sizeof(bvalue));
}

unsigned char BytesMessageImpl::readByte() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  unsigned char value = *(unsigned char *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(unsigned char);

  return value;
}

void BytesMessageImpl::writeByte(unsigned char value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

int BytesMessageImpl::readBytes(std::vector<unsigned char> &value) const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  string::size_type i;
  if (value.capacity() <= size_t(getBodyLength())) {
    for (i = 0; i < value.capacity(); i++) {
      value.at(i) = (unsigned char)_body->bytes_body().value().data()[i + foo->_messageOffset];
    }
    foo->_messageOffset = foo->_messageOffset + value.capacity();
    return static_cast<int>(value.capacity());
  } else {
    for (i = 0; i < size_t(getBodyLength()); i++) {
      value.at(i) = (unsigned char)_body->bytes_body().value().data()[i + foo->_messageOffset];
    }
    foo->_messageOffset = foo->_messageOffset + getBodyLength();
    return static_cast<int>(getBodyLength());
  }
}

void BytesMessageImpl::writeBytes(const std::vector<unsigned char> &value) {
  failIfReadOnlyBody();
  _body->mutable_bytes_body()->mutable_value()->append(value.begin(), value.end());
}

int BytesMessageImpl::readBytes(unsigned char *buffer, int length) const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  string::size_type i;
  if (length <= getBodyLength()) {
    for (i = 0; i < (size_t)length; i++) {
      buffer[i] = (unsigned char)_body->bytes_body().value().data()[i + foo->_messageOffset];
    }
    foo->_messageOffset = foo->_messageOffset + length;
    return length;
  } else {
    for (i = 0; i < size_t(getBodyLength()); i++) {
      buffer[i] = (unsigned char)_body->bytes_body().value().data()[i + foo->_messageOffset];
    }
    foo->_messageOffset = foo->_messageOffset + getBodyLength();
    return getBodyLength();
  }
}

void BytesMessageImpl::writeBytes(const unsigned char *value, int offset, int length) {
  failIfReadOnlyBody();
  appendValue((char *)&value[offset], length);
}

char BytesMessageImpl::readChar() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  char value = (_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(char);

  return value;
}

void BytesMessageImpl::writeChar(char value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

float BytesMessageImpl::readFloat() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  float value = *(float *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(float);

  return value;
}

void BytesMessageImpl::writeFloat(float value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

double BytesMessageImpl::readDouble() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  double value = *(double *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(double);

  return value;
}

void BytesMessageImpl::writeDouble(double value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

void BytesMessageImpl::appendValue(char *value, size_t count) {
  if (!_body) _body = new Proto::Body();
  _body->mutable_bytes_body()->mutable_value()->append(value, count);
}

short BytesMessageImpl::readShort() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  short value = *(short *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(short);

  return value;
}

void BytesMessageImpl::writeShort(short value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

unsigned short BytesMessageImpl::readUnsignedShort() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  unsigned short value = *(unsigned short *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(unsigned short);

  return value;
}

void BytesMessageImpl::writeUnsignedShort(unsigned short value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

int BytesMessageImpl::readInt() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  int value = *(int *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(int);

  return value;
}

void BytesMessageImpl::writeInt(int value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

long long BytesMessageImpl::readLong() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  long long value = *(long long *)(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + sizeof(long long);

  return value;
}

void BytesMessageImpl::writeLong(long long value) {
  failIfReadOnlyBody();
  appendValue((char *)&value, sizeof(value));
}

std::string BytesMessageImpl::readString() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  string value = reinterpret_cast<const char *>(&_body->bytes_body().value().data()[foo->_messageOffset]);
  foo->_messageOffset = foo->_messageOffset + value.size() + 1;

  return value;
}

void BytesMessageImpl::writeString(const std::string &value) {
  failIfReadOnlyBody();
  _body->mutable_bytes_body()->mutable_value()->append(value.begin(), value.end());
  _body->mutable_bytes_body()->mutable_value()->append(1, '\0');
}

std::string BytesMessageImpl::readUTF() const {
  failIfWriteOnlyBody();
  BytesMessageImpl *foo = const_cast<BytesMessageImpl *>(this);
  foo->unpackBody();
  if (foo->_messageOffset >= string::size_type(getBodyLength())) {
    throw cms::MessageEOFException();
  }

  string value = reinterpret_cast<const char *>(&_body->bytes_body().value().data()[foo->_messageOffset]);
  if (value.length() == 0) {
    throw cms::MessageFormatException();
  }

  foo->_messageOffset = foo->_messageOffset + value.size() + 1;

  return value;
}

void BytesMessageImpl::writeUTF(const std::string &value) {
  failIfReadOnlyBody();
  _body->mutable_bytes_body()->mutable_value()->append(value.begin(), value.end());
  _body->mutable_bytes_body()->mutable_value()->append(1, '\0');
}

cms::BytesMessage *BytesMessageImpl::clone() const { return new BytesMessageImpl(*const_cast<BytesMessageImpl *>(this)); }

#endif  //__BytesMessageImpl_CPP__
