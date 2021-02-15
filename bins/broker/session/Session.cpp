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
#include "Exchange.h"
#include "Exception.h"
#include "AsyncLogger.h"

namespace upmq {
namespace broker {

Session::Session(const Connection &connection, std::string id, Proto::Acknowledge acknowledgeType)
    : _id(std::move(id)), _acknowledgeType(acknowledgeType), _connection(connection), _txCounter(0), _stateStack(2) {
  log = &Poco::Logger::get(CONFIGURATION::Instance().log().name);
  TRACE(log);

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
  OnError onError;
  onError.setError(Proto::ERROR_ON_SESSION).setInfo("can't create session").setSql(sql.str());

  TRY_EXECUTE(([&sql]() { dbms::Instance().doNow(sql.str()); }), onError);
}
Session::~Session() {
  TRACE(log);
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
  TRACE(log);
  try {
    EXCHANGE::Instance().removeSenders(*this);
  } catch (...) {  // -V565
  }
}
void Session::dropTemporaryDestination() const {
  TRACE(log);
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
  TRACE(log);
  OnError onError;
  std::stringstream sql;
  sql << "delete from " << _connection.sessionsT() << " where id = \'" << _id << "\';";
  onError.setError(Proto::ERROR_ON_UNSESSION).setInfo("can't delete from session table").setSql(sql.str());
  TRY_EXECUTE_NOEXCEPT(([&sql]() { dbms::Instance().doNow(sql.str()); }), onError);
}
std::string Session::acknowlegeName(Proto::Acknowledge acknowledgeType) {
  switch (acknowledgeType) {
    case Proto::SESSION_TRANSACTED:
      return MakeStringify(SESSION_TRANSACTED);
    case Proto::AUTO_ACKNOWLEDGE:
      return MakeStringify(AUTO_ACKNOWLEDGE);
    case Proto::CLIENT_ACKNOWLEDGE:
      return MakeStringify(CLIENT_ACKNOWLEDGE);
    case Proto::DUPS_OK_ACKNOWLEDGE:
      return MakeStringify(DUPS_OK_ACKNOWLEDGE);
  }
  return emptyString;
}
bool Session::isAutoAcknowledge() const { return _acknowledgeType == Proto::AUTO_ACKNOWLEDGE; }
bool Session::isDupsOkAcknowledge() const { return _acknowledgeType == Proto::DUPS_OK_ACKNOWLEDGE; }
bool Session::isClientAcknowledge() const { return _acknowledgeType == Proto::CLIENT_ACKNOWLEDGE; }
bool Session::isTransactAcknowledge() const { return _acknowledgeType == Proto::SESSION_TRANSACTED; }
void Session::addToUsed(const std::string &uri) {
  TRACE(log);
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
  TRACE(log);
  upmq::ScopedWriteRWLock writeRWLock(_destinationsLock);
  auto it = _destinations.find(destinationID);
  if (it != _destinations.end()) {
    _destinations.erase(it);
  }
}
void Session::begin() const {
  TRACE(log);
  if (!_stateStack.empty() && (_stateStack.last() == State::BEGIN)) {
    return;
  }
  ++_txCounter;

  _stateStack.push(State::BEGIN);
}
void Session::rebegin() const {
  TRACE(log);
  begin();
  for (const auto &item : _destinations) {
    EXCHANGE::Instance().begin(*this, item);
  }
  _rebeginMutex.unlock();
}
void Session::commit() const {
  TRACE(log);
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
  TRACE(log);
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
  TRACE(log);
  addToUsed(sMessage.message().destination_uri());
  EXCHANGE::Instance().saveMessage(*this, sMessage);
}
std::string Session::txName() const {
  TRACE(log);
  return _id + "_" + std::to_string(_txCounter);
}
void Session::addSender(const MessageDataContainer &sMessage) {
  TRACE(log);
  addToUsed(sMessage.sender().destination_uri());
  EXCHANGE::Instance().addSender(*this, sMessage);
}
void Session::removeSender(const MessageDataContainer &sMessage) {
  TRACE(log);
  EXCHANGE::Instance().removeSender(*this, sMessage);
}
void Session::addSubscription(const MessageDataContainer &sMessage) {
  TRACE(log);
  const Proto::Subscription &subscription = sMessage.subscription();
  EXCHANGE::Instance().addSubscription(*this, sMessage);
  addToUsed(subscription.destination_uri());
}
void Session::removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum) {
  TRACE(log);
  EXCHANGE::Instance().removeConsumer(sMessage, tcpNum);
}
void Session::removeConsumers(const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum) {
  TRACE(log);
  EXCHANGE::Instance().removeConsumer(_id, destinationID, subscriptionID, tcpNum);
}
const CircularQueue<Session::State> &Session::stateStack() const { return _stateStack; }
const Connection &Session::connection() const { return _connection; }
const std::string &Session::id() const { return _id; }
Proto::Acknowledge Session::type() const { return _acknowledgeType; }
void Session::processAcknowledge(const MessageDataContainer &sMessage) {
  TRACE(log);
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
          if (ex.error() == Proto::ERROR_UNKNOWN) {
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
  TRACE(log);
  upmq::ScopedReadRWLock readRWLock(_destinationsLock);
  for (const auto &destination : _destinations) {
    try {
      EXCHANGE::Instance().destination(destination, Exchange::DestinationCreationMode::NO_CREATE).closeAllSubscriptions(*this, tcpNum);
    } catch (Exception &ex) {
      if ((ex.error() != Proto::ERROR_UNKNOWN) || (ex.message().find("destination not found") == std::string::npos)) {
        throw Exception(ex);
      }
    }
  }
}
bool Session::canResetCurrentDBSession() const {
  TRACE(log);
  if (currentDBSession == nullptr) {
    return true;
  }
  return !currentDBSession->inTransaction();
}
}  // namespace broker
}  // namespace upmq
