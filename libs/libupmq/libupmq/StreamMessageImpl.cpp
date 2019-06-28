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

#ifndef __StreamMessageImpl_CPP__
#define __StreamMessageImpl_CPP__

#include "StreamMessageImpl.h"

StreamMessageImpl::StreamMessageImpl() : MessageTemplate<cms::StreamMessage>() {
  _header->mutable_message()->set_body_type(Proto::Body::kStreamBody);
  _header->mutable_message()->set_sender_id("");
  _header->mutable_message()->set_session_id("");
  _header->mutable_message()->set_timestamp(0);
  _header->mutable_object_id()->assign("");
  _header->set_request_reply_id(-1);
  _body->mutable_stream_body();  //-V773
}

StreamMessageImpl::StreamMessageImpl(StreamMessageImpl &other) : MessageTemplate<cms::StreamMessage>(other) {}

StreamMessageImpl::StreamMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr) : MessageTemplate<cms::StreamMessage>(header, body_buff, body_size) {
  DECAF_UNUSED_VAR(pr);
}

StreamMessageImpl::StreamMessageImpl(const UPMQCommand &command) : MessageTemplate<cms::StreamMessage>(command) {}

StreamMessageImpl::~StreamMessageImpl() throw() {  // throw()
}

cms::Message::ValueType StreamMessageImpl::getNextValueType() const { return cms::Message::NULL_TYPE; }

bool StreamMessageImpl::readBoolean() const { return false; }

void StreamMessageImpl::writeBoolean(bool value) { (void)value; }

unsigned char StreamMessageImpl::readByte() const { return 0; }

void StreamMessageImpl::writeByte(unsigned char value) { (void)value; }

int StreamMessageImpl::readBytes(std::vector<unsigned char> &value) const {
  (void)value;
  return 0;
}

void StreamMessageImpl::writeBytes(const std::vector<unsigned char> &value) { (void)value; }

int StreamMessageImpl::readBytes(unsigned char *buffer, int length) const {
  (void)buffer;
  (void)length;
  return 0;
}

void StreamMessageImpl::writeBytes(const unsigned char *value, int offset, int length) {
  (void)value;
  (void)offset;
  (void)length;
}

char StreamMessageImpl::readChar() const { return 0; }

void StreamMessageImpl::writeChar(char value) { (void)value; }

float StreamMessageImpl::readFloat() const { return (float)0.; }

void StreamMessageImpl::writeFloat(float value) { (void)value; }

double StreamMessageImpl::readDouble() const { return 0.; }

void StreamMessageImpl::writeDouble(double value) { (void)value; }

short StreamMessageImpl::readShort() const { return 0; }

void StreamMessageImpl::writeShort(short value) { (void)value; }

unsigned short StreamMessageImpl::readUnsignedShort() const { return 0; }

void StreamMessageImpl::writeUnsignedShort(unsigned short value) { (void)value; }

int StreamMessageImpl::readInt() const { return 0; }

void StreamMessageImpl::writeInt(int value) { (void)value; }

long long StreamMessageImpl::readLong() const { return 0; }

void StreamMessageImpl::writeLong(long long value) { (void)value; }

std::string StreamMessageImpl::readString() const { return emptyString; }

void StreamMessageImpl::writeString(const std::string &value) { (void)value; }

void StreamMessageImpl::reset() {}

cms::StreamMessage *StreamMessageImpl::clone() const { return new StreamMessageImpl(*const_cast<StreamMessageImpl *>(this)); }

#endif  //__StreamMessageImpl_CPP__
