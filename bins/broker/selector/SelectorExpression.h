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

#ifndef UPMQ_BROKER_SELECTOREXPRESSION_H
#define UPMQ_BROKER_SELECTOREXPRESSION_H

#include <iosfwd>
#include <string>

namespace upmq {
namespace broker {
namespace storage {

class SelectorEnv;

class TopExpression {
 public:
  virtual ~TopExpression(){};
  virtual void repr(std::ostream &) const = 0;
  virtual bool eval(const SelectorEnv &) const = 0;

  static TopExpression *parse(const std::string &exp);
};
}  // namespace storage
}  // namespace broker
}  // namespace upmq

#endif
