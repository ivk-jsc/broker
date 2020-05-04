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

#ifndef BROKER_SUBSCRIPTION_H
#define BROKER_SUBSCRIPTION_H

#include <Poco/Condition.h>
#include <Poco/RWLock.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <Poco/AtomicCounter.h>
#include <unordered_map>
#include <unordered_set>
#include "AsyncLogger.h"
#include "Consumer.h"
#include "MessageDataContainer.h"
#include "MessageStorage.h"
#include "Selector.h"
#include "SenderList.h"
#include "Session.h"
#include "MoveableRWLock.h"
#include <Poco/Logger.h>

namespace upmq {
namespace broker {

class Destination;

// NOTE: on save-message get group-id and add to the Connection pre-selector
class Subscription {
 public:
  enum class Type : int { SIMPLE, DURABLE, BROWSER };
  enum class LocalMode : int { DEFAULT, IS_NO_LOCAL };
  enum class ConsumerMode : int { EXCLUSIVE, ROUND_ROBIN };
  enum class ProcessMessageResult : int { OK_COMPLETE, CONSUMER_LOCKED, CONSUMER_NOT_RAN, CONSUMER_CANT_SEND, NO_MESSAGE, SOME_ERROR };

  /// @brief ConsumersListType - map<consumer_id, options>
  using ConsumersListType = std::vector<std::pair<std::string, Consumer>>;
  SenderList _senders;

  struct Info {
    Info(std::string id_, std::string name_, Type type_, int consumers_, uint64_t messages_, bool running_)
        : id(std::move(id_)), name(std::move(name_)), type(type_), consumers(consumers_), messages(messages_), running(running_) {}
    Info() = default;
    Info(Info &&) = default;
    Info(const Info &) = default;
    Info &operator=(Info &&) = default;
    Info &operator=(const Info &) = default;
    ~Info() = default;

    std::string id;
    std::string name;
    Type type = Type::SIMPLE;
    int consumers = 0;
    uint64_t messages = 0;
    bool running = false;
  };

 private:
  std::string _id;
  std::string _name;
  Type _type;
  ConsumersListType _consumers;
  mutable MRWLock _consumersLock;
  const std::string _routingKey;
  mutable Storage _storage;
  const Destination &_destination;
  std::unique_ptr<std::atomic_bool> _isRunning;
  mutable size_t _currentConsumerNumber;
  std::string _consumersT;

  unsigned long long _messageCounter;
  Poco::Logger *log;
  mutable bool _isSubsNotify;
  mutable bool _isDestroyed;
  mutable bool _isInited;
  mutable bool _hasSnapshot;
  std::shared_ptr<std::deque<std::shared_ptr<MessageDataContainer>>> _roundRobinCache;

 public:
  Subscription(const upmq::broker::Destination &destination,
               const std::string &id,
               std::string name,
               std::string routingKey,
               Subscription::Type type = Subscription::Type::SIMPLE);
  virtual ~Subscription() noexcept;

  Subscription(Subscription &&) = default;

  void save(const Session &session, const MessageDataContainer &sMessage);
  void commit(const Session &session);
  void abort(const Session &session);

  void addClient(
      const Session &session, size_t tcpConnectionNum, const std::string &objectID, const std::string &selector, Subscription::LocalMode localMode);
  bool removeClient(size_t tcpConnectionNum, const std::string &sessionID);
  void removeClients();
  const std::string &routingKey() const;
  void onEvent(const void *pSender, const MessageDataContainer *&sMessage);

  bool isDurable() const;
  bool isBrowser() const;
  bool isRunning() const;

  //  const Consumer &at(int index) const;
  const Consumer *at(size_t index) const;
  const Consumer &byObjectID(const std::string &objectID);
  const Consumer &byClientAndHandlerAndSessionIDs(const std::string &clientID, size_t handlerNum, const std::string &sessionID);
  Storage &storage() const;

  void start();
  void stop();
  void start(const Consumer &consumer);
  void stop(const Consumer &consumer);
  void recover();
  void recover(const Consumer &consumer);
  void setHasNotify(bool hasNotify);
  void destroy();
  const std::string &id() const;
  void setInited(bool inited);
  bool isInited() const;
  void addSender(const Session &session, const MessageDataContainer &sMessage);
  void removeSender(const Session &session, const MessageDataContainer &sMessage);
  void removeSenders(const Session &session);
  bool hasSnapshot() const;
  void setHasSnapshot(bool hasSnapshot);
  ProcessMessageResult getNextMessage();
  Info info() const;
  void resetConsumersCache();

 private:
  Subscription::ConsumersListType::iterator eraseConsumer(ConsumersListType::iterator it);
  void changeCurrentConsumerNumber() const;
  bool allConsumersStopped();
  bool consumersWithSelectorsOnly() const;
  bool removeConsumer(size_t tcpConnectionNum, const std::string &sessionID);
  void removeConsumers(size_t tcpConnectionNum);
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_SUBSCRIPTION_H
