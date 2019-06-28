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

#ifndef BROKER_SENDER_H
#define BROKER_SENDER_H

#include <atomic>
#include <string>
#include <tuple>

namespace upmq {
namespace broker {

class Session;
class Storage;
class MessageDataContainer;

class Sender {
 protected:
  std::string _id;
  const Session &_session;
  mutable std::string _groupID;
  /// @brief group-info -  tuple<group_id, message_id, seq_counter>
  mutable std::tuple<std::string, std::string, std::atomic_int> _groupInfo;
  void fixMessageInGroup(const Session &session, Storage &storage, const MessageDataContainer &sMessage) const;
  void closeGroup(const Session &session, Storage &storage) const;

 public:
  explicit Sender(std::string id, const Session &session);
  virtual ~Sender();
  virtual void fixMessageInGroup(const Session &session, const MessageDataContainer &sMessage) const = 0;
  virtual void closeGroup(const Session &session) const = 0;
  const std::string &id() const;
  const Session &session() const;

 private:
  void setToLast(const Session &session, Storage &storage) const;
};
}  // namespace broker
}  // namespace upmq
#endif  // BROKER_SENDER_H
