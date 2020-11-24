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

#include "MessageInfo.h"

namespace upmq {
namespace broker {

MessageInfo::MessageInfo(Poco::Int64 num,
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
                         const std::string &transactionID)
    : tuple(num,
            messageID,
            type,
            bodyType,
            priority,
            persistent,
            correlationID,
            replyTo,
            timestamp,
            expiration,
            timetolive,
            createdTime,
            deliveryCount,
            deliveryStatus,
            clientID,
            consumerID,
            groupID,
            groupSeq,
            lastInGroup,
            transactionID) {}

MessageInfo::MessageInfo(const std::string &messageID) {
  tuple.set<message::field::Num::POSITION>(-1);
  tuple.set<message::field::MessageId::POSITION>(messageID);
}
MessageInfo::MessageInfo(MessageInfo::MsgTuple tuple_) : tuple(std::move(tuple_)) {}
const std::string &MessageInfo::messageId() const { return tuple.get<message::field::MessageId::POSITION>(); }
Poco::Int64 MessageInfo::num() const { return tuple.get<message::field::Num::POSITION>(); }
const std::string &MessageInfo::type() const { return tuple.get<message::field::Type::POSITION>(); }
int MessageInfo::bodyType() const { return tuple.get<message::field::BodyType::POSITION>(); }
int MessageInfo::priority() const { return tuple.get<message::field::Priority::POSITION>(); }
bool MessageInfo::persistent() const { return tuple.get<message::field::Persistent::POSITION>(); }
const Poco::Nullable<std::string> &MessageInfo::correlationID() const { return tuple.get<message::field::CorrelationId::POSITION>(); }
const Poco::Nullable<std::string> &MessageInfo::replyTo() const { return tuple.get<message::field::ReplyTo::POSITION>(); }
Poco::Int64 MessageInfo::timestamp() const { return tuple.get<message::field::Timestamp::POSITION>(); }
Poco::Int64 MessageInfo::expiration() const { return tuple.get<message::field::Expiration::POSITION>(); }
Poco::Int64 MessageInfo::timetolive() const { return tuple.get<message::field::TimeToLive::POSITION>(); }
const Poco::DateTime &MessageInfo::createdTime() const { return tuple.get<message::field::CreatedTime::POSITION>(); }
int MessageInfo::deliveryCount() const { return tuple.get<message::field::DeliveryCount::POSITION>(); }
int MessageInfo::deliveryStatus() const { return tuple.get<message::field::DeliveryStatus::POSITION>(); }
const Poco::Nullable<std::string> &MessageInfo::clientID() const { return tuple.get<message::field::ClientId::POSITION>(); }
const Poco::Nullable<std::string> &MessageInfo::consumerID() const { return tuple.get<message::field::ConsumerId::POSITION>(); }
const Poco::Nullable<std::string> &MessageInfo::groupID() const { return tuple.get<message::field::GroupId::POSITION>(); }
int MessageInfo::groupSeq() const { return tuple.get<message::field::GroupSeq::POSITION>(); }
bool MessageInfo::lastInGroup() const { return tuple.get<message::field::LastInGroup::POSITION>(); }
const Poco::Nullable<std::string> &MessageInfo::transactionID() const { return tuple.get<message::field::TransactionId::POSITION>(); }
void MessageInfo::clear() { tuple = MessageInfo::MsgTuple{}; }
}  // namespace broker
}  // namespace upmq
