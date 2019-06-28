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
 
#ifndef IVK_UPMQ_ROWPAGEREPLACER_H
#define IVK_UPMQ_ROWPAGEREPLACER_H

#include "TemplateParamReplacer.h"

class DestinationRowPageReplacer : public TemplateParamReplacer {
 public:
  enum RowParam { destination = 0, type, subscriptions, creationTime, dataPath, connectionString, messages };

  DestinationRowPageReplacer(
      std::string pageName, std::string destination, int type, int subscriptions, std::string creationTime, std::string dataPath, std::string connectionString, uint64_t messages);

  std::string destinationReplacer();

  std::string typeReplacer();

  std::string subscriptionsReplacer();

  std::string creationTimeReplacer();

  std::string dataPathReplacer();

  std::string connectionStringReplacer();

  std::string messagesReplacer();

 private:
  std::string _destination;
  int _type;
  int _subscriptions;
  std::string _creationTime;
  std::string _dataPath;
  std::string _connectionString;
  uint64_t _messages;
};

#endif  // IVK_UPMQ_ROWPAGEREPLACER_H
