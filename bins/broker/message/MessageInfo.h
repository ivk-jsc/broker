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
#include <string>
#include "MessageDefines.h"

namespace upmq {
namespace broker {

class MessageInfo {
 public:
  enum class Field : int {
    num = 0,
    message_id,
    type,
    body_type,
    priority,
    persistent,
    correlation_id,
    reply_to,
    timestamp,
    expiration,
    ttl,
    created_time,
    delivery_count,
    delivery_status,
    client_id,
    consumer_id,
    group_id,
    group_seq,
    last_in_group,
    transaction_id
  };
  struct FieldInfo {
    constexpr explicit FieldInfo(Field ec_) : ec(ec_), position(static_cast<int>(ec)) {}
    const Field ec;
    const int position;
  };

  typedef Poco::Tuple<Poco::Int64,                  // num;
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
                      Poco::Int64,                  // createdTime;
                      int,                          // deliveryCount;
                      int,                          // deliveryStatus;
                      Poco::Nullable<std::string>,  // clientID;
                      Poco::Nullable<std::string>,  // consumerID;
                      Poco::Nullable<std::string>,  // groupID;
                      int,                          // groupSeq;
                      bool,                         // lastInGroup;
                      Poco::Nullable<std::string>   // transactionID;
                      >
      MsgTuple;

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
                       Poco::Int64 createdTime,
                       int deliveryCount,
                       int deliveryStatus,
                       const std::string &clientID,
                       const std::string &consumerID,
                       const std::string &groupID,
                       int groupSeq,
                       bool lastInGroup,
                       const std::string &transactionID);
};
}  // namespace broker
}  // namespace upmq

namespace message {
static constexpr upmq::broker::MessageInfo::FieldInfo field_num = upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::num);
static constexpr upmq::broker::MessageInfo::FieldInfo field_message_id =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::message_id);
static constexpr upmq::broker::MessageInfo::FieldInfo field_type = upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::type);
static constexpr upmq::broker::MessageInfo::FieldInfo field_body_type =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::body_type);
static constexpr upmq::broker::MessageInfo::FieldInfo field_priority =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::priority);
static constexpr upmq::broker::MessageInfo::FieldInfo field_persistent =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::persistent);
static constexpr upmq::broker::MessageInfo::FieldInfo field_correlation_id =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::correlation_id);
static constexpr upmq::broker::MessageInfo::FieldInfo field_reply_to =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::reply_to);
static constexpr upmq::broker::MessageInfo::FieldInfo field_timestamp =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::timestamp);
static constexpr upmq::broker::MessageInfo::FieldInfo field_expiration =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::expiration);
static constexpr upmq::broker::MessageInfo::FieldInfo field_ttl = upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::ttl);
static constexpr upmq::broker::MessageInfo::FieldInfo field_created_time =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::created_time);
static constexpr upmq::broker::MessageInfo::FieldInfo field_delivery_count =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::delivery_count);
static constexpr upmq::broker::MessageInfo::FieldInfo field_delivery_status =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::delivery_status);
static constexpr upmq::broker::MessageInfo::FieldInfo field_client_id =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::client_id);
static constexpr upmq::broker::MessageInfo::FieldInfo field_consumer_id =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::consumer_id);
static constexpr upmq::broker::MessageInfo::FieldInfo field_group_id =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::group_id);
static constexpr upmq::broker::MessageInfo::FieldInfo field_group_seq =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::group_seq);
static constexpr upmq::broker::MessageInfo::FieldInfo field_last_in_group =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::last_in_group);
static constexpr upmq::broker::MessageInfo::FieldInfo field_transaction_id =
    upmq::broker::MessageInfo::FieldInfo(upmq::broker::MessageInfo::Field::transaction_id);
}  // namespace message

#endif  // BROKER_MESSAGEINFO_H
