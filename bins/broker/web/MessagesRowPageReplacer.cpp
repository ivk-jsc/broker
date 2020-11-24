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

#include "MessagesRowPageReplacer.h"
#include <MessageDefines.h>
#include <utility>

MessagesRowPageReplacer::MessagesRowPageReplacer(std::string pageName,
                                                 std::string messageId_,
                                                 int deliveryMode_,
                                                 int64_t expiration_,
                                                 int priority_,
                                                 std::string created_,
                                                 std::string correlationId_,
                                                 int type_,
                                                 std::string clientId_,
                                                 std::string consumerId_,
                                                 int error_,
                                                 int num_,
                                                 std::string delivered_)
    : TemplateParamReplacer(std::move(pageName)),
      _messageId(std::move(messageId_)),
      _deliveryMode(deliveryMode_),
      _expiration(expiration_),
      _priority(priority_),
      _created(std::move(created_)),
      _correlationId(std::move(correlationId_)),
      _type(type_),
      _clientId(std::move(clientId_)),
      _consumerId(std::move(consumerId_)),
      _error(error_),
      _num(num_),
      _delivered(std::move(delivered_)),
      _processMode(processMode) {
  addReplacer(MakeStringify(messageId), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::messageIdReplacer);
  addReplacer(MakeStringify(deliveryMode), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::deliveryModeReplacer);
  addReplacer(MakeStringify(expiration), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::expirationReplacer);
  addReplacer(MakeStringify(priority), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::priorityReplacer);
  addReplacer(MakeStringify(created), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::createdReplacer);
  addReplacer(MakeStringify(correlationId), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::correlationIdReplacer);
  addReplacer(MakeStringify(type), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::typeReplacer);
  addReplacer(MakeStringify(clientId), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::clientIdReplacer);
  addReplacer(MakeStringify(consumerId), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::consumerIdReplacer);
  addReplacer(MakeStringify(error), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::errorReplacer);
  addReplacer(MakeStringify(num), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::numReplacer);
  addReplacer(MakeStringify(delivered), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::deliveredReplacer);
  addReplacer(MakeStringify(processMode), (TemplateParamReplacer::Callback)&MessagesRowPageReplacer::processModeReplacer);
}

std::string MessagesRowPageReplacer::messageIdReplacer() { return _messageId; }

std::string MessagesRowPageReplacer::deliveryModeReplacer() {
  switch (_deliveryMode) {
    case 1:
      return MakeStringify(NON_PERSISTENT);
    case 2:
      return MakeStringify(PERSISTENT);
    default:
    case 0:
      break;
  }
  return MakeStringify(DELIVERYMODE_NOT_SET);
}

std::string MessagesRowPageReplacer::expirationReplacer() { return std::to_string(_expiration); }

std::string MessagesRowPageReplacer::priorityReplacer() { return std::to_string((long long)_priority); }

std::string MessagesRowPageReplacer::createdReplacer() { return _created; }

std::string MessagesRowPageReplacer::correlationIdReplacer() { return _correlationId; }

std::string MessagesRowPageReplacer::typeReplacer() {
  UNUSED_VAR(_type);
  return "DATA";
}

std::string MessagesRowPageReplacer::clientIdReplacer() { return _clientId; }

std::string MessagesRowPageReplacer::consumerIdReplacer() { return _consumerId; }

std::string MessagesRowPageReplacer::errorReplacer() { return std::to_string((long long)_error); }

std::string MessagesRowPageReplacer::numReplacer() { return std::to_string((long long)_num); }

std::string MessagesRowPageReplacer::deliveredReplacer() { return _delivered; }

std::string MessagesRowPageReplacer::processModeReplacer() {
  switch (static_cast<message::_DeliveryStatus>(_processMode)) {
    case message::NOT_SENT:
      return MakeStringify(NOT_SENT);
    case message::WAS_SENT:
      return MakeStringify(WAS_SENT);
    case message::DELIVERED:
      return MakeStringify(DELIVERED);
  }
  return MakeStringify(NOT_SENT);
}

MessagesRowPageReplacer::~MessagesRowPageReplacer() = default;
