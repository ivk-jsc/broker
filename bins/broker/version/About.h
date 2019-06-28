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

#ifndef BROKERVERSION_H
#define BROKERVERSION_H

#include <iostream>
#include <sstream>
#include <string>

namespace upmq {
namespace broker {

class About {
 public:
  static std::string version();
  static std::string author();
  static std::string commit();
  static std::string vendor();
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKERVERSION_H
