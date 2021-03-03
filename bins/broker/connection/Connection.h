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

#ifndef BROKER_CONNECTION_H
#define BROKER_CONNECTION_H

#include <Poco/RWLock.h>
#include <Poco/Logger.h>
#include <atomic>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "AsyncTCPHandler.h"
#include "Selector.h"
#include "Session.h"
#include "FixedSizeUnorderedMap.h"

namespace upmq {
namespace broker {

class Connection {
 public:
  using TCPConnectionsList = std::set<size_t>;
  // SessionsList - map<session_id, session>
  using SessionsList = FSUnorderedMap<std::string, std::unique_ptr<upmq::broker::Session>>;

 private:
  std::string _clientID;
  mutable TCPConnectionsList _tcpConnections;
  mutable upmq::MRWLock _tcpLock;
  SessionsList _sessions;

  const std::string _sessionsT;
  const std::string _tcpT;
  mutable Poco::Logger *log{nullptr};

 public:
  explicit Connection(const std::string &clientID);
  virtual ~Connection();

  void addSession(const std::string &sessionID, Proto::Acknowledge acknowledgeType);
  void removeSession(const std::string &sessionID, size_t tcpNum);
  void beginTX(const std::string &sessionID);
  void commitTX(const std::string &sessionID);
  void abortTX(const std::string &sessionID);
  void saveMessage(const MessageDataContainer &sMessage);
  void processAcknowledge(const MessageDataContainer &sMessage);

  void addSender(const MessageDataContainer &sMessage);
  void removeSender(const MessageDataContainer &sMessage);

  void addSubscription(const MessageDataContainer &sMessage);
  void removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum);
  void removeConsumers(const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum);

  const std::string &clientID() const;
  void setClientID(const std::string &clientID);
  void addTcpConnection(size_t tcpConnectionNum);
  void removeTcpConnection(size_t tcpConnectionNum);
  bool isTcpConnectionExists(size_t tcpConnectionNum) const;
  size_t tcpConnectionsCount() const;

  const std::string &sessionsT() const;
  const std::string &tcpT() const;

  int maxNotAcknowledgedMessages(size_t tcpConnectionNum) const;
  std::string transactionID(const std::string &sessionID) const;
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_CONNECTION_H
