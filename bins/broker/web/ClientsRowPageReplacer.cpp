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

#include <Consumer.h>
#include "Defines.h"
#include "ClientsRowPageReplacer.h"
#include "Subscription.h"

ClientsRowPageReplacer::ClientsRowPageReplacer(
    std::string pageName, std::string id_, int type_, int msgCount_, std::string subscription_, int status_, std::string selector_)
    : TemplateParamReplacer(std::move(pageName)),
      _id(std::move(id_)),
      _type(type_),
      _msgCount(msgCount_),
      _subscription(std::move(subscription_)),
      _status(status_),
      _selector(std::move(selector_)) {
  addReplacer(MakeStringify(type), (TemplateParamReplacer::Callback)&ClientsRowPageReplacer::typeReplacer);
  addReplacer(MakeStringify(id), (TemplateParamReplacer::Callback)&ClientsRowPageReplacer::idReplacer);
  addReplacer(MakeStringify(msgCount), (TemplateParamReplacer::Callback)&ClientsRowPageReplacer::msgCountReplacer);
  addReplacer(MakeStringify(subscription), (TemplateParamReplacer::Callback)&ClientsRowPageReplacer::subscriptionReplacer);
  addReplacer(MakeStringify(status), (TemplateParamReplacer::Callback)&ClientsRowPageReplacer::statusReplacer);
  addReplacer(MakeStringify(selector), (TemplateParamReplacer::Callback)&ClientsRowPageReplacer::selectorReplacer);
}

std::string ClientsRowPageReplacer::typeReplacer() {
  switch (static_cast<upmq::broker::Subscription::Type>(_type)) {
    case upmq::broker::Subscription::Type::SIMPLE:
      return MakeStringify(SIMPLE);
    case upmq::broker::Subscription::Type::DURABLE:
      return MakeStringify(DURABLE);
    case upmq::broker::Subscription::Type::BROWSER:
      return MakeStringify(BROWSER);
  }
  return MakeStringify(SIMPLE);
}

std::string ClientsRowPageReplacer::idReplacer() { return _id; }

std::string ClientsRowPageReplacer::msgCountReplacer() { return std::to_string((long long)_msgCount); }

std::string ClientsRowPageReplacer::subscriptionReplacer() { return _subscription; }

std::string ClientsRowPageReplacer::statusReplacer() { return (_status ? "ONLINE" : "OFFLINE"); }

std::string ClientsRowPageReplacer::selectorReplacer() { return _selector; }
