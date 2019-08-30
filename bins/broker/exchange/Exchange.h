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

#ifndef BROKER_EXCHANGE_H
#define BROKER_EXCHANGE_H

#include <Poco/RWLock.h>
#include <Poco/ThreadPool.h>
#include <unordered_map>
#include <Poco/RunnableAdapter.h>
#include "DestinationFactory.h"
#include "Singleton.h"

namespace upmq {
namespace broker {

class Session;

class Exchange {
 public:
  enum class DestinationCreationMode { NO_CREATE = 0, CREATE };
  // DestinationsList - map<mainDestinationPath, Destination>
  using DestinationsList = FSUnorderedMap<std::string, std::unique_ptr<Destination>>;

 private:
  mutable DestinationsList _destinations;
  const std::string _destinationsT;
  std::atomic_bool _isRunning{false};
  mutable std::vector<Poco::FastMutex> _mutexDestinations;
  mutable std::vector<Poco::Condition> _conditionDestinations;
  Poco::ThreadPool _threadPool;
  std::unique_ptr<Poco::RunnableAdapter<Exchange>> _threadAdapter;
  std::atomic_size_t _thrNum{0};
  using BQ = moodycamel::ConcurrentQueue<std::string>;
  mutable BQ _destinationEvents;

 public:
  Exchange();
  virtual ~Exchange();
  Destination &destination(const std::string &uri, Exchange::DestinationCreationMode creationMode = Exchange::DestinationCreationMode::CREATE) const;
  void deleteDestination(const std::string &uri);
  static std::string mainDestinationPath(const std::string &uri);
  void saveMessage(const Session &session, const MessageDataContainer &sMessage);
  const std::string &destinationsT() const;
  void removeConsumer(const std::string &sessionID, const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum);
  void removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum);
  void begin(const upmq::broker::Session &session, const std::string &destinationID);
  void commit(const upmq::broker::Session &session, const std::string &destinationID);
  void abort(const upmq::broker::Session &session, const std::string &destinationID);
  bool isDestinationTemporary(const std::string &id);
  void dropDestination(const std::string &id, DestinationOwner *owner = nullptr);
  void dropOwnedDestination(const std::string &clientId);
  void addSubscription(const upmq::broker::Session &session, const MessageDataContainer &sMessage);
  void addSender(const upmq::broker::Session &session, const MessageDataContainer &sMessage);
  void removeSender(const upmq::broker::Session &session, const MessageDataContainer &sMessage);
  void removeSenders(const upmq::broker::Session &session);
  void start();
  void stop();
  void postNewMessageEvent(const std::string &name) const;
  void addNewMessageEvent(const std::string &name) const;
  std::vector<Destination::Info> info() const;

 private:
  Destination &getDestination(const std::string &id) const;
  void removeSenderFromAnyDest(const upmq::broker::Session &session, const std::string &senderID);
  void run();
};
}  // namespace broker
}  // namespace upmq

typedef Singleton<upmq::broker::Exchange> EXCHANGE;

#endif  // BROKER_EXCHANGE_H
