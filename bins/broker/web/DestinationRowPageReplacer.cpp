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

#include <Destination.h>
#include "Defines.h"
#include "DestinationRowPageReplacer.h"

DestinationRowPageReplacer::DestinationRowPageReplacer(
    std::string pageName, std::string destination, int type, int subscriptions, std::string creationTime, std::string dataPath, std::string connectionString, uint64_t messages)
    : TemplateParamReplacer(std::move(pageName)),
      _destination(std::move(destination)),
      _type(type),
      _subscriptions(subscriptions),
      _creationTime(std::move(creationTime)),
      _dataPath(std::move(dataPath)),
      _connectionString(std::move(connectionString)),
      _messages(messages) {
  addReplacer(MakeStringify(destination), (TemplateParamReplacer::Callback)&DestinationRowPageReplacer::destinationReplacer);
  addReplacer(MakeStringify(type), (TemplateParamReplacer::Callback)&DestinationRowPageReplacer::typeReplacer);
  addReplacer(MakeStringify(subscriptions), (TemplateParamReplacer::Callback)&DestinationRowPageReplacer::subscriptionsReplacer);
  addReplacer(MakeStringify(creationTime), (TemplateParamReplacer::Callback)&DestinationRowPageReplacer::creationTimeReplacer);
  addReplacer(MakeStringify(dataPath), (TemplateParamReplacer::Callback)&DestinationRowPageReplacer::dataPathReplacer);
  addReplacer(MakeStringify(connectionString), (TemplateParamReplacer::Callback)&DestinationRowPageReplacer::connectionStringReplacer);
  addReplacer(MakeStringify(messages), (TemplateParamReplacer::Callback)&DestinationRowPageReplacer::messagesReplacer);
}

std::string DestinationRowPageReplacer::destinationReplacer() { return _destination; }

std::string DestinationRowPageReplacer::typeReplacer() { return upmq::broker::Destination::typeName(static_cast<upmq::broker::Destination::Type>(_type)); }

std::string DestinationRowPageReplacer::subscriptionsReplacer() { return std::to_string(_subscriptions); }

std::string DestinationRowPageReplacer::creationTimeReplacer() { return _creationTime; }

std::string DestinationRowPageReplacer::connectionStringReplacer() { return _connectionString; }

std::string DestinationRowPageReplacer::dataPathReplacer() { return STORAGE_CONFIG.data.get().toString() + "/" + _dataPath; }

std::string DestinationRowPageReplacer::messagesReplacer() { return std::to_string(_messages); }
