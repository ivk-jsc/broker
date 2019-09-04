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

#ifndef __MessageImpl_CPP__
#define __MessageImpl_CPP__

#include "MessageImpl.h"

MessageImpl::MessageImpl() : MessageTemplate<cms::Message>() {
  _header->mutable_message()->set_body_type(Proto::Body::BODYTYPE_NOT_SET);
  _body->mutable_unknown_fields();
}

MessageImpl::MessageImpl(const MessageImpl &other) : MessageTemplate<cms::Message>(other) {}

MessageImpl::MessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr)
    : MessageTemplate<cms::Message>(header, body_buff, body_size) {
  DECAF_UNUSED_VAR(pr);
}

MessageImpl::MessageImpl(const UPMQCommand &command) : MessageTemplate<cms::Message>(command) {}

MessageImpl::~MessageImpl() throw() {}

cms::Message *MessageImpl::clone() const { return new MessageImpl(*const_cast<MessageImpl *>(this)); }

#endif  //__MessageImpl_CPP__
