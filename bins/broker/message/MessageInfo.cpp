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
                         Poco::Int64 createdTime,
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
  tuple.set<message::field_num.position>(-1);
  tuple.set<message::field_message_id.position>(messageID);
}
MessageInfo::MessageInfo(const MessageInfo::MsgTuple &tuple_) : tuple(tuple_) {}
MessageInfo::MessageInfo(MessageInfo::MsgTuple &&tuple_) : tuple(std::move(tuple_)) {}
}  // namespace broker
}  // namespace upmq
