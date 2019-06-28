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

#include "MessageFactoryImpl.h"

#include "BytesMessageImpl.h"
#include "MapMessageImpl.h"
#include "MessageImpl.h"
#include "ObjectMessageImpl.h"
#include "StreamMessageImpl.h"
#include "TextMessageImpl.h"

MessageFactoryImpl::MessageFactoryImpl() {}

MessageFactoryImpl::~MessageFactoryImpl() {}

UPMQCommand *MessageFactoryImpl::getProperMessage(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size) {
  if (header->ProtoMessageType_case() == Proto::ProtoMessage::kMessage) {
    switch (header->message().body_type()) {
      case Proto::Body::BODYTYPE_NOT_SET:
        return dynamic_cast<UPMQCommand *>(new MessageImpl(header, body_buff, body_size, true));  //-V572
      case Proto::Body::kTextBody:
        return dynamic_cast<UPMQCommand *>(new TextMessageImpl(header, body_buff, body_size, true));  //-V572
      case Proto::Body::kMapBody:
        return dynamic_cast<UPMQCommand *>(new MapMessageImpl(header, body_buff, body_size, true));  //-V572
      case Proto::Body::kObjectBody:
        return dynamic_cast<UPMQCommand *>(new ObjectMessageImpl(header, body_buff, body_size, true));  //-V572
      case Proto::Body::kBytesBody:
        return dynamic_cast<UPMQCommand *>(new BytesMessageImpl(header, body_buff, body_size, true));  //-V572
      case Proto::Body::kStreamBody:
        return dynamic_cast<UPMQCommand *>(new StreamMessageImpl(header, body_buff, body_size, true));  //-V572
      default:
        return nullptr;
    }
  }
  return new UPMQCommand(header, body_buff, body_size);
}
