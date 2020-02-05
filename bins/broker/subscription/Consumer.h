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

#ifndef BROKER_CONSUMER_H
#define BROKER_CONSUMER_H

#include <memory>
#include <string>
#include <utility>
#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/Statement.h>
#else
#include <Poco/Data/Statement.h>
#endif

#include "ProtoBuf.h"
#include "Selector.h"
#include "MessageDataContainer.h"

namespace upmq {
namespace broker {
namespace consumer {
struct Msg {
  Poco::Int64 num = 0;
  std::string messageId;
  int priority = 0;
  int persistent = 0;
  Poco::Nullable<std::string> correlationID;
  Poco::Nullable<std::string> replyTo;
  std::string type;
  Poco::Int64 timestamp = 0;
  Poco::Int64 ttl = 0;
  Poco::Int64 expiration = 0;
  std::string screated;
  int bodyType = 0;
  Poco::Nullable<std::string> groupID;
  int groupSeq = 0;
  int deliveryCount = 0;

  void reset() {
    num = 0;
    messageId.clear();
    correlationID.clear();
    replyTo.clear();
    type.clear();
    timestamp = 0;
    ttl = 0;
    expiration = 0;
    screated.clear();
    bodyType = 0;
    groupID.clear();
    groupSeq = 0;
    deliveryCount = 0;
  }
};
}  // namespace consumer
class Consumer {
 public:
  struct session_info {
    session_info() = default;
    session_info(std::string _id, Proto::Acknowledge _type) : id(std::move(_id)), type(_type) {}
    std::string id;
    Proto::Acknowledge type = Proto::Acknowledge::AUTO_ACKNOWLEDGE;
    mutable std::string txName;
  };
  int num;
  std::unique_ptr<storage::Selector> selector;
  std::string objectID;
  session_info session;
  size_t tcpNum;
  std::string clientID;
  bool noLocal;
  bool browser;
  mutable bool isRunning;
  std::string id;
  int maxNotAckMsg;
  mutable bool abort = false;

  mutable std::shared_ptr<std::deque<std::shared_ptr<MessageDataContainer>>> select;

  Consumer(int _num,
           std::string _clientID,
           size_t _tcpNum,
           std::string _objectID,
           std::string _sessionID,
           Proto::Acknowledge _sessionType,
           const std::string &_selector,
           bool _noLocal,
           bool _browser,
           int _maxNotAxkMsg,
           std::shared_ptr<std::deque<std::shared_ptr<MessageDataContainer>>> selectCache);
  Consumer(const Consumer &) = delete;
  Consumer(Consumer &&) = default;
  Consumer &operator=(Consumer &&) = default;
  virtual ~Consumer();
  void stop() const;
  void start() const;
  static Consumer makeFakeConsumer();
  static std::string genConsumerID(const std::string &_clientID,
                                   const std::string &_tcpID,
                                   const std::string &_sessionID,
                                   const std::string &_selector);
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_CONSUMER_H
