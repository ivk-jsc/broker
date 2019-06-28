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

#include "DestinationFactory.h"
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include "Defines.h"
#include "MiscDefines.h"
#include "fake_cpp14.h"
#include <Poco/URI.h>
namespace upmq {
namespace broker {

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}
std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

std::unique_ptr<Destination> DestinationFactory::createDestination(const Exchange &exchange, const std::string &uri) {
  switch (destinationType(uri)) {
    case Destination::Type::NONE:
      throw EXCEPTION("invalid destination uri", uri, ERROR_DESTINATION);
    case Destination::Type::QUEUE:
      return std::make_unique<QueueDestination>(exchange, uri);
    case Destination::Type::TOPIC:
      return std::make_unique<TopicDestination>(exchange, uri);
    case Destination::Type::TEMPORARY_QUEUE:
      return std::make_unique<TemporaryQueueDestination>(exchange, uri);
    case Destination::Type::TEMPORARY_TOPIC:
      return std::make_unique<TemporaryTopicDestination>(exchange, uri);
  }
  return nullptr;
}

Destination::Type DestinationFactory::destinationType(const std::string &uri) {
  if (!uri.empty()) {
    Poco::StringTokenizer URI(uri, ":", Poco::StringTokenizer::TOK_TRIM);
    std::string destinationSType = Poco::toLower(URI[0]);
    if (destinationSType == QUEUE_PREFIX) {
      return Destination::Type::QUEUE;
    }
    if (destinationSType == TEMP_QUEUE_PREFIX) {
      return Destination::Type::TEMPORARY_QUEUE;
    }
    if (destinationSType == TOPIC_PREFIX) {
      return Destination::Type::TOPIC;
    }
    if (destinationSType == TEMP_TOPIC_PREFIX) {
      return Destination::Type::TEMPORARY_TOPIC;
    }
  }
  return Destination::Type::NONE;
}
std::string DestinationFactory::destinationTypePrefix(const std::string &uri) {
  Poco::URI tURI(uri);
  return Poco::toLower(tURI.getScheme()) + "/";
}
std::string DestinationFactory::destinationName(const std::string &uri) {
  Poco::StringTokenizer URI(uri, ":", Poco::StringTokenizer::TOK_TRIM);
  std::string destName = split(Poco::trim(Poco::replace(URI[1], "//", " ")), '/')[0];
  removeParamsAndFragmentFromURI(destName);
  return destName;
}
void DestinationFactory::removeParamsAndFragmentFromURI(std::string &uri) {
  std::string::size_type pos = uri.find('?');
  if (pos != std::string::npos) {
    uri.erase(pos);
  }
  pos = uri.find('#');
  if (pos != std::string::npos) {
    uri.erase(pos);
  }
}
}  // namespace broker
}  // namespace upmq
