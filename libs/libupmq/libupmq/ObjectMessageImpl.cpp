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

#ifndef __ObjectMessageImpl_CPP__
#define __ObjectMessageImpl_CPP__

#include "ObjectMessageImpl.h"

ObjectMessageImpl::ObjectMessageImpl() : MessageTemplate<cms::ObjectMessage>() {
  Proto::Message* mutableMessage = _header->mutable_message();
  mutableMessage->set_body_type(Proto::Body::kObjectBody);
  mutableMessage->set_sender_id("");
  mutableMessage->set_session_id("");
  mutableMessage->set_timestamp(0);
  _header->mutable_object_id()->assign("");
  _header->set_request_reply_id(-1);
  _body->mutable_object_body();  //-V773
}

ObjectMessageImpl::ObjectMessageImpl(const ObjectMessageImpl &other) : MessageTemplate<cms::ObjectMessage>(other) {}

ObjectMessageImpl::~ObjectMessageImpl() throw() {}

ObjectMessageImpl::ObjectMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr)
    : MessageTemplate<cms::ObjectMessage>(header, body_buff, body_size) {
  DECAF_UNUSED_VAR(pr);
}

ObjectMessageImpl::ObjectMessageImpl(const UPMQCommand &command) : MessageTemplate<cms::ObjectMessage>(command) {}

string ObjectMessageImpl::getObjectBytes() const {
  failIfWriteOnlyBody();
  ObjectMessageImpl *foo = const_cast<ObjectMessageImpl *>(this);
  foo->unpackBody();

  return _body->object_body().value();
}

void ObjectMessageImpl::setObjectBytes(const string &text) {
  failIfReadOnlyBody();

  _body->mutable_object_body()->set_value(text);
}

cms::ObjectMessage *ObjectMessageImpl::clone() const { return new ObjectMessageImpl(*const_cast<ObjectMessageImpl *>(this)); }

#endif  //__ObjectMessageImpl_CPP__
