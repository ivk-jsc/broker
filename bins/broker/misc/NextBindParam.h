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

#ifndef BROKER_NEXTBINDPARAM_H
#define BROKER_NEXTBINDPARAM_H

#include <string>
#include "Configuration.h"

class NextBindParam {
  mutable int _counter = 0;

 public:
  explicit NextBindParam(int init = 0) : _counter(init){};
  std::string operator()() const {
    if (STORAGE_CONFIG.connection.props.dbmsType == upmq::broker::storage::Postgresql) {
      return "$" + std::to_string(++_counter);
    }
    return "?";
  }
  void reset() { _counter = 0; }
};

#endif  // BROKER_NEXTBINDPARAM_H
