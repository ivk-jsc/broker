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

#ifndef BROKER_CLIENTSROWPAGEREPLACER_H
#define BROKER_CLIENTSROWPAGEREPLACER_H

#include "TemplateParamReplacer.h"

class ClientsRowPageReplacer : public TemplateParamReplacer {
 public:
  enum RowParam { id = 0, type, msgCount, subscription, status, selector };

  ClientsRowPageReplacer(std::string pageName, std::string id, int type, int msgCount, std::string subscription, int status, std::string selector);

  std::string typeReplacer();

  std::string idReplacer();

  std::string msgCountReplacer();

  std::string subscriptionReplacer();

  std::string statusReplacer();

  std::string selectorReplacer();

 private:
  std::string _id;
  int _type;
  int _msgCount;
  std::string _subscription;
  int _status;
  std::string _selector;
};

#endif  // BROKER_CLIENTSROWPAGEREPLACER_H
