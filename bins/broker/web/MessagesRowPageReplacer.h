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

#ifndef BROKER_MESSAGESROWPAGEREPLACER_H
#define BROKER_MESSAGESROWPAGEREPLACER_H

#include "Defines.h"

#include "TemplateParamReplacer.h"

class MessagesRowPageReplacer : public TemplateParamReplacer {
 public:
  enum RowParam { messageId = 0, deliveryMode, expiration, priority, created, correlationId, type, clientId, consumerId, error, num, delivered, processMode };

  MessagesRowPageReplacer(std::string pageName,
                          std::string messageId,
                          int deliveryMode,
                          int64_t expiration,
                          int priority,
                          std::string created,
                          std::string correlationId,
                          int type,
                          std::string clientId,
                          std::string consumerId,
                          int error,
                          int num,
                          std::string delivered);

  ~MessagesRowPageReplacer() override;

  std::string messageIdReplacer();

  std::string deliveryModeReplacer();

  std::string expirationReplacer();

  std::string priorityReplacer();

  std::string createdReplacer();

  std::string correlationIdReplacer();

  std::string typeReplacer();

  std::string clientIdReplacer();

  std::string consumerIdReplacer();

  std::string errorReplacer();

  std::string numReplacer();

  std::string deliveredReplacer();

  std::string processModeReplacer();

 private:
  std::string _messageId;
  int _deliveryMode;
  int64_t _expiration;
  int _priority;
  std::string _created;
  std::string _correlationId;
  int _type;
  std::string _clientId;
  std::string _consumerId;
  int _error;
  int _num;
  std::string _delivered;
  int _processMode;
};

#endif  // BROKER_MESSAGESROWPAGEREPLACER_H
