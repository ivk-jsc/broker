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

#ifndef BROKER_DESTINATIONFACTORY_H
#define BROKER_DESTINATIONFACTORY_H

#include "QueueDestination.h"
#include "TemporaryQueueDestination.h"
#include "TemporaryTopicDestination.h"
#include "TopicDestination.h"

namespace upmq {
namespace broker {
class DestinationFactory {
 public:
  static std::unique_ptr<Destination> createDestination(const Exchange &exchange, const std::string &uri);
  static Destination::Type destinationType(const std::string &uri);
  static std::string destinationTypePrefix(const std::string &uri);
  static std::string destinationName(const std::string &uri);
  static void removeParamsAndFragmentFromURI(std::string &uri);
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_DESTINATIONFACTORY_H
