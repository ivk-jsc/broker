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

#ifndef BROKER_MESSAGEINFO_H
#define BROKER_MESSAGEINFO_H

#include <Poco/Nullable.h>
#include <Poco/Tuple.h>
#include <Poco/DateTime.h>
#include <string>
#include "MessageDefines.h"

namespace upmq {
namespace broker {

class MessageInfo {
 public:
  enum Field {
    NUM = 0,
    MESSAGE_ID,
    TYPE,
    BODY_TYPE,
    PRIORITY,
    PERSISTENT,
    CORRELATION_ID,
    REPLY_TO,
    TIMESTAMP,
    EXPIRATION,
    TTL,
    CREATED_TIME,
    DELIVERY_COUNT,
    DELIVERY_STATUS,
    CLIENT_ID,
    CONSUMER_ID,
    GROUP_ID,
    GROUP_SEQ,
    LAST_IN_GROUP,
    TRANSACTION_ID
  };

  template <MessageInfo::Field field>
  struct FieldInfo {
    static constexpr int POSITION = static_cast<int>(field);
  };

  using MsgTuple = Poco::Tuple<Poco::Int64,                  // num;
                               std::string,                  // messageID;
                               std::string,                  // type;
                               int,                          // bodyType;
                               int,                          // priority;
                               bool,                         // persistent;
                               Poco::Nullable<std::string>,  // correlationID;
                               Poco::Nullable<std::string>,  // replyTo;
                               Poco::Int64,                  // timestamp;
                               Poco::Int64,                  // expiration;
                               Poco::Int64,                  // timetolive;
                               Poco::DateTime,               // createdTime;
                               int,                          // deliveryCount;
                               int,                          // deliveryStatus;
                               Poco::Nullable<std::string>,  // clientID;
                               Poco::Nullable<std::string>,  // consumerID;
                               Poco::Nullable<std::string>,  // groupID;
                               int,                          // groupSeq;
                               bool,                         // lastInGroup;
                               Poco::Nullable<std::string>   // transactionID;
                               >;

  MsgTuple tuple;

  MessageInfo() = default;
  explicit MessageInfo(MsgTuple tuple);
  explicit MessageInfo(const std::string &messageID);
  explicit MessageInfo(Poco::Int64 num,
                       const std::string &messageID,
                       const std::string &type,
                       int bodyType,
                       int priority,
                       bool persistent,
                       const std::string &correlationID,
                       const std::string &replyTo,
                       Poco::Int64 timestamp,
                       Poco::Int64 expiration,
                       Poco::Int64 timetolive,
                       const Poco::DateTime &createdTime,
                       int deliveryCount,
                       int deliveryStatus,
                       const std::string &clientID,
                       const std::string &consumerID,
                       const std::string &groupID,
                       int groupSeq,
                       bool lastInGroup,
                       const std::string &transactionID);

  const std::string &messageId() const;
  Poco::Int64 num() const;
  const std::string &type() const;
  int bodyType() const;
  int priority() const;
  bool persistent() const;
  const Poco::Nullable<std::string> &correlationID() const;
  const Poco::Nullable<std::string> &replyTo() const;
  Poco::Int64 timestamp() const;
  Poco::Int64 expiration() const;
  Poco::Int64 timetolive() const;
  const Poco::DateTime &createdTime() const;
  int deliveryCount() const;
  int deliveryStatus() const;
  const Poco::Nullable<std::string> &clientID() const;
  const Poco::Nullable<std::string> &consumerID() const;
  const Poco::Nullable<std::string> &groupID() const;
  int groupSeq() const;
  bool lastInGroup() const;
  const Poco::Nullable<std::string> &transactionID() const;
  void clear();
};

}  // namespace broker
}  // namespace upmq

namespace message {
namespace field {
using Num = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::NUM>;
using MessageId = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::MESSAGE_ID>;
using Type = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::TYPE>;
using BodyType = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::BODY_TYPE>;
using Priority = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::PRIORITY>;
using Persistent = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::PERSISTENT>;
using CorrelationId = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::CORRELATION_ID>;
using ReplyTo = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::REPLY_TO>;
using Timestamp = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::TIMESTAMP>;
using Expiration = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::EXPIRATION>;
using TimeToLive = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::TTL>;
using CreatedTime = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::CREATED_TIME>;
using DeliveryCount = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::DELIVERY_COUNT>;
using DeliveryStatus = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::DELIVERY_STATUS>;
using ClientId = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::CLIENT_ID>;
using ConsumerId = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::CONSUMER_ID>;
using GroupId = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::GROUP_ID>;
using GroupSeq = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::GROUP_SEQ>;
using LastInGroup = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::LAST_IN_GROUP>;
using TransactionId = upmq::broker::MessageInfo::FieldInfo<upmq::broker::MessageInfo::TRANSACTION_ID>;
}  // namespace field
}  // namespace message

#endif  // BROKER_MESSAGEINFO_H
