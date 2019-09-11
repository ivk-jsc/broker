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

#ifndef __DestinationImpl_CPP__
#define __DestinationImpl_CPP__

#include "DestinationImpl.h"
#include "Util.h"
#include "ConnectionImpl.h"
#include "SessionImpl.h"

#include <sstream>
#include <stdexcept>

const std::string DestinationImpl::queue_prefix = "queue://";
const std::string DestinationImpl::topic_prefix = "topic://";
const std::string DestinationImpl::temporary_queue_prefix = "temp-queue://";
const std::string DestinationImpl::temporary_topic_prefix = "temp-topic://";

DestinationImpl::DestinationImpl(SessionImpl *session, const string &destUri)
    : _session(session), _destroyed(false), _destType(URIPrefixToType(destUri)), _destName(destNameFromURI(destUri)), _destUri(destUri) {}

DestinationImpl::DestinationImpl(SessionImpl *session, const string &destName, cms::Destination::DestinationType destType)
    : _session(session), _destroyed(false), _destType(destType), _destName(destName), _destUri() {
  std::stringstream tmp_uri;
  switch (destType) {
    case cms::Destination::QUEUE:
      tmp_uri << queue_prefix;
      break;
    case cms::Destination::TOPIC:
      tmp_uri << topic_prefix;
      break;
    case cms::Destination::TEMPORARY_QUEUE:
      tmp_uri << temporary_queue_prefix;
      break;
    case cms::Destination::TEMPORARY_TOPIC:
      tmp_uri << temporary_topic_prefix;
      break;
    default:
      throw cms::CMSException("error type destination");
  }
  trim(_destName);
  tmp_uri << _destName;
  _destUri = tmp_uri.str();
}

DestinationImpl::~DestinationImpl() {
  switch (_destType) {
    case cms::Destination::TEMPORARY_QUEUE:
    case cms::Destination::TEMPORARY_TOPIC:
      try {
        destroy();
      }
      CATCH_ALL_NOTHROW
    default:;
  }
}

const string &DestinationImpl::getName() const { return _destName; }

const string &DestinationImpl::getUri() const { return _destUri; }

cms::Destination::DestinationType DestinationImpl::getType() const { return _destType; }

std::string cms::Destination::getDestinationTypeName(cms::Destination::DestinationType destType) {
  switch (destType) {
    case cms::Destination::QUEUE:
      return MakeStringify(QUEUE);
    case cms::Destination::TOPIC:
      return MakeStringify(TOPIC);
    case cms::Destination::TEMPORARY_QUEUE:
      return MakeStringify(TEMPORARY_QUEUE);
    case cms::Destination::TEMPORARY_TOPIC:
      return MakeStringify(TEMPORARY_TOPIC);
    default:
      return "";
  }
}

std::string DestinationImpl::getTopicName() const { return this->getName(); }

std::string DestinationImpl::getQueueName() const { return this->getName(); }

void DestinationImpl::destroy() {
  if (isDestroyed()) {
    return;
  }

  try {
    if (_session != nullptr) {
      if (_session->_connection != nullptr) {
        if (!_session->_connection->isClosed()) {
          for (auto &it : _session->_consumersMap) {
            if (it.second->_destination->getName() == this->getName() && it.second->isAlive()) {
              throw cms::CMSException("destination in use");
            }
          }
          _session->removeDesination(this);
          _session->undestination(_destUri);
          _destroyed = true;
        }
      }
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Destination *DestinationImpl::clone() const {
  DestinationImpl *ret = new DestinationImpl(*const_cast<DestinationImpl *>(this));
  return ret;
}

void DestinationImpl::copy(const cms::Destination &source) {
  const DestinationImpl *dest = dynamic_cast<const DestinationImpl *>(&source);
  if (dest != nullptr) {
    _destName = dest->_destName;
    _destType = dest->_destType;
    _destUri = dest->_destUri;
  }
}

bool DestinationImpl::equals(const cms::Destination &other) const {
  const DestinationImpl *dest = dynamic_cast<const DestinationImpl *>(&other);
  if (dest != nullptr) {
    if (_destName.compare(dest->_destName) == 0 && _destType == dest->_destType) {
      return true;
    }
  }
  return false;
}

cms::Destination::DestinationType DestinationImpl::URIPrefixToType(const std::string &uri) {
  if (uri.substr(0, queue_prefix.length()) == queue_prefix) {
    return cms::Destination::QUEUE;
  }
  if (uri.substr(0, topic_prefix.length()) == topic_prefix) {
    return cms::Destination::TOPIC;
  }
  if (uri.substr(0, temporary_queue_prefix.length()) == temporary_queue_prefix) {
    return cms::Destination::TEMPORARY_QUEUE;
  }
  if (uri.substr(0, temporary_topic_prefix.length()) == temporary_topic_prefix) {
    return cms::Destination::TEMPORARY_TOPIC;
  }
  return cms::Destination::QUEUE;
}

std::string DestinationImpl::destNameFromURI(const std::string &uri) {
  std::string::size_type count = 0;
  if (uri.substr(0, queue_prefix.length()) == queue_prefix) {
    count = queue_prefix.length();
  } else if (uri.substr(0, topic_prefix.length()) == topic_prefix) {
    count = topic_prefix.length();
  } else if (uri.substr(0, temporary_queue_prefix.length()) == temporary_queue_prefix) {
    count = temporary_queue_prefix.length();
  } else if (uri.substr(0, temporary_topic_prefix.length()) == temporary_topic_prefix) {
    count = temporary_topic_prefix.length();
  }
  return uri.substr(count);
}

bool DestinationImpl::isDestroyed() { return _destroyed; }
#endif  //__DestinationImpl_CPP__
