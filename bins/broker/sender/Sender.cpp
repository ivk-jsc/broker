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

#include "Sender.h"
#include <Poco/UUIDGenerator.h>
#include "MessageStorage.h"
#include "Session.h"

namespace upmq {
namespace broker {
Sender::Sender(std::string id, const Session &session)
    : _id(std::move(id)), _session(session), _groupID(Poco::UUIDGenerator::defaultGenerator().createRandom().toString()) {}
Sender::~Sender() = default;
void Sender::fixMessageInGroup(const Session &session, Storage &storage, const MessageDataContainer &sMessage) const {
  auto &message = sMessage.message();
  if (message.has_group_id()) {
    std::string groupID = message.group_id();
    groupID.append("+").append(_groupID);

    if (!std::get<0>(_groupInfo).empty() && (std::get<0>(_groupInfo) != groupID)) {
      setToLast(session, storage);
    }
    if (std::get<0>(_groupInfo) != groupID) {
      _groupInfo = std::make_tuple(groupID, message.message_id(), 1);
    } else {
      ++(std::get<2>(_groupInfo));
      std::get<1>(_groupInfo) = message.message_id();
    }
    const_cast<Proto::Message &>(message).set_group_id(std::get<0>(_groupInfo));
    const_cast<Proto::Message &>(message).set_group_seq(std::get<2>(_groupInfo));
  }
}
void Sender::setToLast(const Session &session, Storage &storage) const {
  storage.setMessageToLastInGroup(session, std::get<1>(_groupInfo));
  _groupID = Poco::UUIDGenerator::defaultGenerator().createRandom().toString();
}
void Sender::closeGroup(const Session &session, Storage &storage) const {
  if (!std::get<0>(_groupInfo).empty()) {
    setToLast(session, storage);
  }
  _groupInfo = std::make_tuple(emptyString, emptyString, 0);
}
const std::string &Sender::id() const { return _id; }
const Session &Sender::session() const { return _session; }
}  // namespace broker
}  // namespace upmq
