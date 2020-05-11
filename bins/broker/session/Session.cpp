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

#include <utility>
#include "Connection.h"
#include "Defines.h"
#include "Exchange.h"
#include "MiscDefines.h"

namespace upmq {
namespace broker {

Session::Session(const Connection &connection, std::string id, Proto::Acknowledge acknowledgeType)
    : _id(std::move(id)), _acknowledgeType(acknowledgeType), _connection(connection), _txCounter(0), _stateStack(2) {
  if (isTransactAcknowledge()) {
    begin();
  }
  std::stringstream sql;
  sql << "insert into " << _connection.sessionsT() << " ("
      << " id"
      << ", ack_type"
      << ")"
      << " values "
      << "("
      << "\'" << _id << "\'"
      << "," << _acknowledgeType << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't create session", sql.str(), ERROR_ON_SESSION);
}
Session::~Session() {
  if (isTransactAcknowledge()) {
    try {
      abort(true);
    } catch (...) {
    }
  }
  try {
    deleteFromConnectionTable();
    removeSenders();
    dropTemporaryDestination();
  } catch (...) {
  }
}
void Session::removeSenders() const {
  try {
    EXCHANGE::Instance().removeSenders(*this);
  } catch (...) {  // -V565
  }
}
void Session::dropTemporaryDestination() const {
  for (const auto &destination : _destinations) {
    try {
      if (EXCHANGE::Instance().isDestinationTemporary(destination)) {
        DestinationOwner destinationOwner(_connection.clientID(), 0);
        EXCHANGE::Instance().dropDestination(destination, &destinationOwner);
      }
    } catch (...) {  // -V565
    }
  }
}
void Session::deleteFromConnectionTable() const {
  std::stringstream sql;
  sql << "delete from " << _connection.sessionsT() << " where id = \'" << _id << "\';";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't delete from session table", sql.str(), ERROR_ON_UNSESSION)
}
std::string Session::acknowlegeName(Proto::Acknowledge acknowledgeType) {
  switch (acknowledgeType) {
    case SESSION_TRANSACTED:
      return MakeStringify(SESSION_TRANSACTED);
    case AUTO_ACKNOWLEDGE:
      return MakeStringify(AUTO_ACKNOWLEDGE);
    case CLIENT_ACKNOWLEDGE:
      return MakeStringify(CLIENT_ACKNOWLEDGE);
    case DUPS_OK_ACKNOWLEDGE:
      return MakeStringify(DUPS_OK_ACKNOWLEDGE);
  }
  return emptyString;
}
bool Session::isAutoAcknowledge() const { return _acknowledgeType == AUTO_ACKNOWLEDGE; }
bool Session::isDupsOkAcknowledge() const { return _acknowledgeType == DUPS_OK_ACKNOWLEDGE; }
bool Session::isClientAcknowledge() const { return _acknowledgeType == CLIENT_ACKNOWLEDGE; }
bool Session::isTransactAcknowledge() const { return _acknowledgeType == SESSION_TRANSACTED; }
void Session::addToUsed(const std::string &uri) {
  DestinationsList::iterator it;
  const std::string destName = Exchange::mainDestinationPath(uri);
  {
    upmq::ScopedReadRWLock readRWLock(_destinationsLock);
    it = _destinations.find(destName);
    if (it != _destinations.end()) {
      return;
    }
  }

  upmq::ScopedWriteRWLock writeRWLock(_destinationsLock);
  it = _destinations.find(destName);
  if (it == _destinations.end()) {
    _destinations.insert(destName);
    if (isTransactAcknowledge()) {
      EXCHANGE::Instance().begin(*this, uri);
    }
  }
}
void Session::removeFromUsed(const std::string &destinationID) {
  upmq::ScopedWriteRWLock writeRWLock(_destinationsLock);
  auto it = _destinations.find(destinationID);
  if (it != _destinations.end()) {
    _destinations.erase(it);
  }
}
void Session::begin() const {
  if (!_stateStack.empty() && (_stateStack.last() == State::BEGIN)) {
    return;
  }
  ++_txCounter;

  _stateStack.push(State::BEGIN);
}
void Session::rebegin() const {
  begin();
  for (const auto &item : _destinations) {
    EXCHANGE::Instance().begin(*this, item);
  }
  _rebeginMutex.unlock();
}
void Session::commit() const {
  if (_stateStack.last() == State::COMMIT) {
    return;
  }
  if (isTransactAcknowledge()) {
    _rebeginMutex.lock();
  }
  for (const auto &item : _destinations) {
    EXCHANGE::Instance().commit(*this, item);
  }
  _stateStack.push(State::COMMIT);
  rebegin();
}
void Session::abort(bool destruct) const {
  if (_stateStack.last() == State::ABORT) {
    return;
  }
  if (isTransactAcknowledge() && !destruct) {
    _rebeginMutex.lock();
  }
  for (const auto &item : _destinations) {
    EXCHANGE::Instance().abort(*this, item);
  }
  _stateStack.push(State::ABORT);
  if (!destruct) {
    rebegin();
  }
}
void Session::saveMessage(const MessageDataContainer &sMessage) {
  addToUsed(sMessage.message().destination_uri());
  EXCHANGE::Instance().saveMessage(*this, sMessage);
}
std::string Session::txName() const {
  if (isTransactAcknowledge()) {
    std::lock_guard<std::recursive_mutex> lock(_rebeginMutex);
    return _id + "_" + std::to_string(_txCounter);
  }
  return _id + "_" + std::to_string(_txCounter);
}
void Session::addSender(const MessageDataContainer &sMessage) {
  addToUsed(sMessage.sender().destination_uri());
  EXCHANGE::Instance().addSender(*this, sMessage);
}
void Session::removeSender(const MessageDataContainer &sMessage) { EXCHANGE::Instance().removeSender(*this, sMessage); }
void Session::addSubscription(const MessageDataContainer &sMessage) {
  const Proto::Subscription &subscription = sMessage.subscription();
  EXCHANGE::Instance().addSubscription(*this, sMessage);
  addToUsed(subscription.destination_uri());
}
void Session::removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum) { EXCHANGE::Instance().removeConsumer(sMessage, tcpNum); }
void Session::removeConsumers(const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum) {
  EXCHANGE::Instance().removeConsumer(_id, destinationID, subscriptionID, tcpNum);
}
const CircularQueue<Session::State> &Session::stateStack() const { return _stateStack; }
const Connection &Session::connection() const { return _connection; }
const std::string &Session::id() const { return _id; }
Proto::Acknowledge Session::type() const { return _acknowledgeType; }
void Session::processAcknowledge(const MessageDataContainer &sMessage) {
  const Proto::Ack &ack = sMessage.ack();
  currentDBSession = dbms::Instance().dbmsSessionPtr();
  currentDBSession->beginTX(ack.message_id() + "ack");
  if (isClientAcknowledge()) {
    std::vector<std::string> toerase;
    {
      upmq::ScopedReadRWLock rlock(_destinationsLock);
      for (const auto &destination : _destinations) {
        try {
          EXCHANGE::Instance().destination(destination, Exchange::DestinationCreationMode::NO_CREATE).ack(*this, sMessage);
        } catch (Exception &ex) {
          if (ex.error() == ERROR_UNKNOWN) {
            toerase.emplace_back(destination);
          } else {
            throw Exception(ex);
          }
        }
      }
    }
    for (const auto &item : toerase) {
      removeFromUsed(item);
    }
  } else {
    EXCHANGE::Instance().destination(ack.destination_uri(), Exchange::DestinationCreationMode::NO_CREATE).ack(*this, sMessage);
  }
  currentDBSession->commitTX();
  currentDBSession.reset(nullptr);
}
void Session::closeSubscriptions(size_t tcpNum) {
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  for (const auto &destination : _destinations) {
    try {
      EXCHANGE::Instance().destination(destination, Exchange::DestinationCreationMode::NO_CREATE).closeAllSubscriptions(*this, tcpNum);
    } catch (Exception &ex) {
      if ((ex.error() != ERROR_UNKNOWN) || (ex.message().find("destination not found") == std::string::npos)) {
        throw Exception(ex);
      }
    }
  }
}
bool Session::canResetCurrentDBSession() const {
  if (currentDBSession == nullptr) {
    return true;
  }
  return !currentDBSession->inTransaction();
}
}  // namespace broker
}  // namespace upmq
