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

#include "SenderList.h"
#include "DestinationFactory.h"
#include "Exception.h"
#include "QueueSender.h"
#include "Session.h"

namespace upmq {
namespace broker {
SenderList::SenderList() = default;
SenderList::~SenderList() = default;
void SenderList::addSender(std::unique_ptr<Sender> sender) {
  upmq::ScopedWriteRWLock writeRWLock(_sendersLock);
  auto it = _senders.find(sender->id());
  if (it == _senders.end()) {
    _senders.insert(std::make_pair(sender->id(), std::move(sender)));
  }
}
void SenderList::removeSender(const std::string &id) {
  upmq::ScopedWriteRWLock writeRWLock(_sendersLock);
  _senders.erase(id);
}
void SenderList::removeSenders(const Session &session) {
  upmq::ScopedWriteRWLock writeRWLock(_sendersLock);
  for (auto it = _senders.begin(); it != _senders.end();) {
    if (it->second->session().id() == session.id()) {
      _senders.erase(it++);
    } else {
      ++it;
    }
  }
}
void SenderList::closeGroups(const Session &session) {
  upmq::ScopedReadRWLock readRWLock(_sendersLock);
  for (const auto &sender : _senders) {
    if (sender.second->session().id() == session.id()) {
      sender.second->closeGroup(session);
    }
  }
}
void SenderList::fixMessageInGroup(const std::string &senderID, const Session &session, const MessageDataContainer &sMessage) const {
  upmq::ScopedReadRWLock readRWLock(_sendersLock);
  auto it = _senders.find(senderID);
  if (it != _senders.end()) {
    it->second->fixMessageInGroup(session, sMessage);
  }
}
void SenderList::closeGroup(const std::string &senderID, const Session &session) const {
  upmq::ScopedReadRWLock readRWLock(_sendersLock);
  auto it = _senders.find(senderID);
  if (it != _senders.end()) {
    it->second->closeGroup(session);
  }
}
}  // namespace broker
}  // namespace upmq
