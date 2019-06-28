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

#ifndef __DestinationImpl_H__
#define __DestinationImpl_H__

#include <cms/Destination.h>
#include <cms/Session.h>

#include "ProtoHeader.h"

#define MakeStringify(name) #name

using namespace std;

class ConnectionImpl;
class SessionImpl;

class DestinationImpl : public cms::TemporaryTopic, public cms::TemporaryQueue {
 public:
  DestinationImpl(SessionImpl *session, const string &destUri);
  DestinationImpl(SessionImpl *session, const string &destName, cms::Destination::DestinationType destType);
  ~DestinationImpl() override;

  cms::Destination::DestinationType getType() const override;
  const string &getName() const override;
  const string &getUri() const;

  std::string getTopicName() const override;
  std::string getQueueName() const override;
  void destroy() override;

  cms::Destination *clone() const override;
  void copy(const cms::Destination &source) override;
  bool equals(const cms::Destination &other) const override;

  static cms::Destination::DestinationType URIPrefixToType(const std::string &uri);
  static std::string destNameFromURI(const std::string &uri);
  bool isDestroyed();

  SessionImpl *_session;
  bool _destroyed;

  cms::Destination::DestinationType _destType;
  string _destName;
  string _destUri;

  static const std::string queue_prefix;
  static const std::string topic_prefix;
  static const std::string temporary_queue_prefix;
  static const std::string temporary_topic_prefix;
};

#endif  //__DestinationImpl_H__
