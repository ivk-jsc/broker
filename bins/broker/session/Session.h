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

#ifndef BROKER_SESSION_H
#define BROKER_SESSION_H

#include <MoveableRWLock.h>
#include <atomic>
#include <string>
#include <unordered_set>
#include "CircularQueue.h"
#include "DBMSConnectionPool.h"
#include "MessageDataContainer.h"
#include "ProtoBuf.h"

namespace upmq {
namespace broker {

class Connection;

class Session {
 public:
  // DestinationsList - set<destination_id>
  using DestinationsList = std::unordered_set<std::string>;
  enum class State : int {
    BEGIN = 11,
    COMMIT = 12,
    ABORT = 13,
  };
  class CurrentDBSession {
    mutable upmq::MRWLock _rwLock;
    mutable std::unordered_map<Poco::Thread::TID, std::unique_ptr<storage::DBMSSession>> _currentSessions;

   public:
    explicit CurrentDBSession(std::unique_ptr<storage::DBMSSession> currentDBSession)
        : _currentSessions(THREADS_CONFIG.writers + THREADS_CONFIG.readers + THREADS_CONFIG.subscribers) {
      _currentSessions.emplace(Poco::Thread::currentTid(), std::move(currentDBSession));
    }
    CurrentDBSession() = default;

    storage::DBMSSession &get() const {
      upmq::ScopedReadRWLock readRWLock(_rwLock);
      return *_currentSessions.at(Poco::Thread::currentTid());
    }
    bool exists() const {
      upmq::ScopedReadRWLock readRWLock(_rwLock);
      auto item = _currentSessions.find(Poco::Thread::currentTid());
      return (item != _currentSessions.end()) && (item->second != nullptr);
    }
    void set(std::unique_ptr<storage::DBMSSession> currDBSession) noexcept {
      upmq::ScopedWriteRWLock writeRWLock(_rwLock);
      _currentSessions.erase(Poco::Thread::currentTid());
      _currentSessions.emplace(Poco::Thread::currentTid(), std::move(currDBSession));
    }
    CurrentDBSession &operator=(std::unique_ptr<storage::DBMSSession> currDBSession) noexcept {
      this->set(std::move(currDBSession));
      return *this;
    }
    storage::DBMSSession *operator->() {
      upmq::ScopedReadRWLock readRWLock(_rwLock);
      return _currentSessions.at(Poco::Thread::currentTid()).get();
    }
    bool operator==(storage::DBMSSession *rhs) const {
      upmq::ScopedReadRWLock readRWLock(_rwLock);
      auto it = _currentSessions.find(Poco::Thread::currentTid());
      if (it == _currentSessions.end()) {
        return false;
      }
      if (rhs == nullptr) {
        return it->second.get() == rhs;
      }
      return *it->second == *rhs;
    }
    bool operator!=(storage::DBMSSession *rhs) const { return !(operator==(rhs)); }
    storage::DBMSSession &operator*() { return get(); }
    void reset(storage::DBMSSession *currDBSession) {
      if (currDBSession == nullptr) {
        upmq::ScopedWriteRWLock writeRWLock(_rwLock);
        _currentSessions.erase(Poco::Thread::currentTid());
      } else {
        set(std::unique_ptr<storage::DBMSSession>(currDBSession));
      }
    }
    std::unique_ptr<storage::DBMSSession> move() {
      upmq::ScopedWriteRWLock writeRWLock(_rwLock);
      auto tid = Poco::Thread::currentTid();
      auto pointer = std::move(_currentSessions.at(tid));
      _currentSessions.erase(tid);
      return pointer;
    }
  };

 private:
  std::string _id;
  DestinationsList _destinations;
  upmq::MRWLock _destinationsLock;
  Proto::Acknowledge _acknowledgeType;
  const Connection &_connection;
  mutable std::atomic_int _txCounter;
  mutable CircularQueue<State> _stateStack;
  mutable std::recursive_mutex _rebeginMutex;
  void rebegin() const;

 public:
  Session(const Connection &connection, std::string id, Proto::Acknowledge acknowledgeType);
  virtual ~Session();

  mutable CurrentDBSession currentDBSession;
  void begin() const;
  void commit() const;
  void abort(bool destruct = false) const;
  std::string txName() const;
  void saveMessage(const MessageDataContainer &sMessage);
  void addSender(const MessageDataContainer &sMessage);
  void removeSender(const MessageDataContainer &sMessage);
  void addSubscription(const MessageDataContainer &sMessage);

  void removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum);
  void removeConsumers(const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum);
  void processAcknowledge(const MessageDataContainer &sMessage);

  static std::string acknowlegeName(Proto::Acknowledge acknowledgeType);
  void addToUsed(const std::string &uri);
  void removeFromUsed(const std::string &destinationID);
  bool isAutoAcknowledge() const;
  bool isDupsOkAcknowledge() const;
  bool isClientAcknowledge() const;
  bool isTransactAcknowledge() const;
  const CircularQueue<Session::State> &stateStack() const;
  const Connection &connection() const;
  const std::string &id() const;
  Proto::Acknowledge type() const;
  void closeSubscriptions(size_t tcpNum);
  void removeSenders() const;
  bool canResetCurrentDBSession() const;

 private:
  void deleteFromConnectionTable() const;
  void dropTemporaryDestination() const;
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_SESSION_H
