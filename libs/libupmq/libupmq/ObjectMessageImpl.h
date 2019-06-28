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

#ifndef __ObjectMessageImpl_H__
#define __ObjectMessageImpl_H__

#include <cms/ObjectMessage.h>

#include "MessageTemplate.h"
#include "ProtoHeader.h"

using namespace std;

class ObjectMessageImpl : public MessageTemplate<cms::ObjectMessage> {
 public:
  ObjectMessageImpl();
  ObjectMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr);
  ObjectMessageImpl(const UPMQCommand &command);
  ObjectMessageImpl(const ObjectMessageImpl &other);

  virtual ~ObjectMessageImpl() throw();

  void setObjectBytes(const std::string &text) override;

  std::string getObjectBytes() const override;

  cms::ObjectMessage *clone() const override;
};

#endif /*__ObjectMessageImpl_H__*/
