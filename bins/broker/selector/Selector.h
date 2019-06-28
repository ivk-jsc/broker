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

#ifndef UPMQ_BROKER_SELECTOR_H
#define UPMQ_BROKER_SELECTOR_H

#include <memory>
#include <string>
#include "PropertyHandler.h"

namespace upmq {
namespace broker {
namespace storage {

class MappedDBMessage;
class Value;
class TopExpression;

/**
 * Interface to provide values to a Selector evaluation
 */
class SelectorEnv {
 public:
  virtual ~SelectorEnv(){};

  virtual const Value &value(const std::string &) const = 0;
};

class Selector {
  std::unique_ptr<TopExpression> _parse;
  const std::string _expression;

 public:
  explicit Selector(const std::string &);
  ~Selector();

  const std::string &expression() const;

  /**
   * Evaluate parsed expression with a given environment
   */
  bool eval(const SelectorEnv &env);

  /**
   * Apply selector to message
   * @param msg message to filter against selector
   * @return true if msg meets the selector specification
   */
  bool filter(const MappedDBMessage &msg);
};

/**
 * Return a Selector as specified by the string:
 * - Structured like this so that we can move to caching Selectors with the same
 *   specifications and just returning an existing one
 */
std::shared_ptr<Selector> returnSelector(const std::string &);
}  // namespace storage
}  // namespace broker
}  // namespace upmq
#endif
