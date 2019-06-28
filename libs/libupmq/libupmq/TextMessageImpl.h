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

#ifndef __TextMessageImpl_H__
#define __TextMessageImpl_H__

#include <cms/TextMessage.h>

#include "MessageTemplate.h"
#include "ProtoHeader.h"

using namespace std;

class TextMessageImpl : public MessageTemplate<cms::TextMessage> {
 public:
  TextMessageImpl();
  TextMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr);
  TextMessageImpl(const UPMQCommand &command);
  TextMessageImpl(TextMessageImpl &other);

  virtual ~TextMessageImpl() throw();

  string getText() const override;

  void setText(const char *text) override;
  void setText(const string &text) override;

  cms::TextMessage *clone() const override;
};

#endif /*__TextMessageImpl_H__*/
