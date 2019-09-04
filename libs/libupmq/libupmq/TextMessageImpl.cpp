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

#ifndef __TextMessageImpl_CPP__
#define __TextMessageImpl_CPP__

#include "TextMessageImpl.h"

TextMessageImpl::TextMessageImpl() : MessageTemplate<cms::TextMessage>() {
  _header->mutable_message()->set_body_type(Proto::Body::kTextBody);
  _header->mutable_message()->set_sender_id("");
  _header->mutable_message()->set_session_id("");
  _header->mutable_message()->set_timestamp(0);
  _header->mutable_object_id()->assign("");
  _header->set_request_reply_id(-1);
  _body->mutable_text_body();  //-V773
}

TextMessageImpl::TextMessageImpl(TextMessageImpl &other) : MessageTemplate<cms::TextMessage>(other) {}

TextMessageImpl::TextMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr)
    : MessageTemplate<cms::TextMessage>(header, body_buff, body_size) {
  DECAF_UNUSED_VAR(pr);
}

TextMessageImpl::TextMessageImpl(const UPMQCommand &command) : MessageTemplate<cms::TextMessage>(command) {}

TextMessageImpl::~TextMessageImpl() throw() {}

string TextMessageImpl::getText() const {
  failIfWriteOnlyBody();

  TextMessageImpl *foo = const_cast<TextMessageImpl *>(this);
  foo->unpackBody();

  return std::string(_body->text_body().value());
}

void TextMessageImpl::setText(const char *text) {
  failIfReadOnlyBody();

  _body->mutable_text_body()->set_value(std::string(text));
}

void TextMessageImpl::setText(const string &text) {
  failIfReadOnlyBody();

  _body->mutable_text_body()->set_value(text);
}

cms::TextMessage *TextMessageImpl::clone() const { return new TextMessageImpl(*const_cast<TextMessageImpl *>(this)); }

#endif  //__TextMessageImpl_CPP__
