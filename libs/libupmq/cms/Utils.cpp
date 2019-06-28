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

#include "Utils.h"

#include "cms/BytesMessage.h"
#include "cms/MapMessage.h"
#include "cms/ObjectMessage.h"
#include "cms/StreamMessage.h"
#include "cms/TextMessage.h"
#include "libupmq/MessageFactoryImpl.h"
#include "libupmq/MessageTemplate.h"
#include "transport/UPMQCommand.h"

using namespace cms;

bool Utils::isMessageValid(const cms::Message *message) {
  if (message == nullptr) {
    return false;
  }

  const UPMQCommand *upmqCommand = dynamic_cast<const UPMQCommand *>(message);
  if (upmqCommand == nullptr) {
    return false;
  }

  return upmqCommand->getMessage().IsInitialized();
}

std::vector<char> Utils::serialize(const cms::Message *m) {
  if (!isMessageValid(m)) {
    return std::vector<char>{};
  }

  return dynamic_cast<const UPMQCommand *>(m)->serialize();  //-V522
}
void Utils::serialize(const cms::Message *message, std::ostream &outStream) {
  if (!isMessageValid(message)) {
    return;
  }

  dynamic_cast<const UPMQCommand *>(message)->serializeToOstream(outStream);  //-V522
}
template <typename T>
T *Utils::deserialize(const char *data, long long size) {
  if (size < static_cast<long long>(sizeof(int) + sizeof(long long))) {
    return nullptr;
  }
  T *message = nullptr;

  int headerSize = *reinterpret_cast<const int *>(data);
  long long bodySize = *reinterpret_cast<const long long *>(&data[sizeof(headerSize)]);
  long long preHeaderSize = static_cast<long long>(sizeof(headerSize) + sizeof(bodySize));

  if (size < preHeaderSize + headerSize + bodySize) {
    return nullptr;
  }

  const char *headerBuff = &data[sizeof(headerSize) + sizeof(bodySize)];
  char *bodyBuff = nullptr;
  if (bodySize) {
    bodyBuff = new char[static_cast<size_t>(bodySize + 1)];
    memcpy(bodyBuff, &data[preHeaderSize + headerSize], static_cast<size_t>(bodySize));
  }

  Proto::ProtoMessage *header = new Proto::ProtoMessage();
  if (!header->ParseFromArray(headerBuff, headerSize)) {
    header->Clear();
    delete header;
    delete[] bodyBuff;
    return nullptr;
  }

  message = (MessageTemplate<T> *)MessageFactoryImpl::getProperMessage(header, reinterpret_cast<unsigned char *>(bodyBuff), bodySize);
  return message;
}
template <typename T>
T *Utils::deserialize(std::istream &inputStream) {
  T *message = nullptr;
  int headerSize = 0;
  long long bodySize = 0;
  long long preHeaderSize = static_cast<long long>(sizeof(headerSize) + sizeof(bodySize));

  inputStream.seekg(0, inputStream.beg);
  inputStream.read((char *)&headerSize, sizeof(headerSize));
  inputStream.seekg(sizeof(headerSize), inputStream.beg);
  inputStream.read((char *)&bodySize, sizeof(bodySize));

  std::vector<char> headerBuff;
  headerBuff.resize(static_cast<size_t>(headerSize));
  inputStream.seekg(preHeaderSize, inputStream.beg);
  inputStream.read(&headerBuff[0], headerSize);

  char *bodyBuff = nullptr;
  if (bodySize > 0) {
    bodyBuff = new char[static_cast<size_t>(bodySize)];
    inputStream.seekg(preHeaderSize + headerSize, inputStream.beg);
    inputStream.read(&bodyBuff[0], bodySize);
  }

  Proto::ProtoMessage *header = new Proto::ProtoMessage();
  if (!header->ParseFromArray(&headerBuff[0], headerSize)) {
    header->Clear();
    delete header;
    delete[] bodyBuff;
    return nullptr;
  }

  message = (MessageTemplate<T> *)MessageFactoryImpl::getProperMessage(header, reinterpret_cast<unsigned char *>(&bodyBuff[0]), bodySize);
  return message;
}
std::string Utils::toJsonString(const cms::Message *message) {
  if (!isMessageValid(message)) {
    return "";
  }

  return dynamic_cast<const UPMQCommand *>(message)->toJsonString();  //-V522
}

std::string Utils::toPrettyJsonString(const cms::Message *message) {
  if (!isMessageValid(message)) {
    return "";
  }

  return dynamic_cast<const UPMQCommand *>(message)->toPrettyJsonString();  //-V522
}

template CMS_API cms::Message *Utils::deserialize<cms::Message>(const char *data, long long size);
template CMS_API cms::BytesMessage *Utils::deserialize<cms::BytesMessage>(const char *data, long long size);
template CMS_API cms::MapMessage *Utils::deserialize<cms::MapMessage>(const char *data, long long size);
template CMS_API cms::StreamMessage *Utils::deserialize<cms::StreamMessage>(const char *data, long long size);
template CMS_API cms::ObjectMessage *Utils::deserialize<cms::ObjectMessage>(const char *data, long long size);
template CMS_API cms::TextMessage *Utils::deserialize<cms::TextMessage>(const char *data, long long size);

template CMS_API cms::Message *Utils::deserialize<cms::Message>(std::istream &inputStream);
template CMS_API cms::BytesMessage *Utils::deserialize<cms::BytesMessage>(std::istream &inputStream);
template CMS_API cms::MapMessage *Utils::deserialize<cms::MapMessage>(std::istream &inputStream);
template CMS_API cms::StreamMessage *Utils::deserialize<cms::StreamMessage>(std::istream &inputStream);
template CMS_API cms::ObjectMessage *Utils::deserialize<cms::ObjectMessage>(std::istream &inputStream);
template CMS_API cms::TextMessage *Utils::deserialize<cms::TextMessage>(std::istream &inputStream);
