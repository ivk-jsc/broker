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

#include <fake_cpp14.h>
#include "Connection.h"
#include "AsyncHandlerRegestry.h"
#include "Broker.h"
#include "DBMSSession.h"
#include "Exception.h"
#include "MiscDefines.h"
#include "Exchange.h"

namespace upmq {
namespace broker {

Connection::Connection(const std::string &clientID)
    : _clientID(clientID),
      _clientIDWasSet(!clientID.empty()),
      _sessions(SESSIONS_CONFIG.maxCount),
      _sessionsT("\"" + clientID + "_sessions\""),
      _tcpT("\"" + clientID + "_tcp_connections\"") {
  std::stringstream sql;
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();

  sql << "create table if not exists " << _sessionsT << " ("
      << " id text not null primary key"
      << ",ack_type int not null"
      << ",create_time timestamp not null default current_timestamp"
      << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION("can't create connection", sql.str(), dbSession.rollbackTX(), ERROR_CONNECTION);
  sql.str("");
  sql << "create table if not exists " << _tcpT << " ("
      << " client_id text not null primary key"
      << ",tcp_id int not null"
      << ",create_time timestamp not null default current_timestamp"
      << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION("can't create connection", sql.str(), dbSession.rollbackTX(), ERROR_CONNECTION);

  sql.str("");
  sql << "insert into \"" << BROKER::Instance().id() << "\" (client_id) values "
      << "("
      << "\'" << _clientID << "\'"
      << ");";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION("can't create connection", sql.str(), dbSession.rollbackTX(), ERROR_CONNECTION);
}
Connection::~Connection() {
  try {
    _sessions.clear();
  } catch (...) {
  }
  try {
    std::stringstream sql;
    sql << "delete from \"" << BROKER::Instance().id() << "\" where client_id = \'" << _clientID << "\';";
    TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
    CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't delete client_id", sql.str(), ERROR_CONNECTION)

    sql.str("");
    sql << "drop table if exists " << _sessionsT << ";";
    TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
    CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't drop sessions", sql.str(), ERROR_CONNECTION)
    sql.str("");
    sql << "drop table if exists " << _tcpT << ";";
    TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
    CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't drop tcp connections", sql.str(), ERROR_CONNECTION)
  } catch (...) {
  }
}
const std::string &Connection::clientID() const { return _clientID; }
void Connection::setClientID(const std::string &clientID) {
  if (_clientID.empty()) {
    _clientID = clientID;

    for (const auto &tcpConnection : _tcpConnections) {
      auto handler = AHRegestry::Instance().aHandler(tcpConnection);
      if (handler) {
        handler->setClientID(_clientID);
      }
    }
  } else {
    throw EXCEPTION("connection id can be set only once", clientID, ERROR_CLIENT_ID_EXISTS);
  }
}
void Connection::addTcpConnection(size_t tcpConnectionNum) {
  upmq::ScopedWriteRWLockWithUnlock writeRWLock(_tcpLock);
  auto it = _tcpConnections.find(tcpConnectionNum);
  if (it == _tcpConnections.end()) {
    _tcpConnections.insert(tcpConnectionNum);
    writeRWLock.unlock();
    std::stringstream sql;
    sql << "insert into " << _tcpT << "("
        << "client_id"
        << ",tcp_id"
        << ")"
        << " values "
        << "("
        << " \'" << _clientID << "\'"
        << "," << tcpConnectionNum << ")"
        << ";";
    TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
    CATCH_POCO_DATA_EXCEPTION_PURE("can't add tcp connection", sql.str(), ERROR_CLIENT_ID_EXISTS);
    return;
  }
  throw EXCEPTION("connection already exists", _clientID + " : " + std::to_string(tcpConnectionNum), ERROR_CLIENT_ID_EXISTS);
}
void Connection::removeTcpConnection(size_t tcpConnectionNum) {
  upmq::ScopedWriteRWLockWithUnlock writeRWLock(_tcpLock);
  auto it = _tcpConnections.find(tcpConnectionNum);
  if (it != _tcpConnections.end()) {
    _tcpConnections.erase(it);
    writeRWLock.unlock();
    std::stringstream sql;
    sql << "delete from " << _tcpT << " where tcp_id = " << tcpConnectionNum << ";";
    TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
    CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT("can't remove tcp connection", sql.str(), ERROR_CONNECTION)
  }
}
bool Connection::isTcpConnectionExists(size_t tcpConnectionNum) const {
  upmq::ScopedReadRWLock readRWLock(_tcpLock);
  auto it = _tcpConnections.find(tcpConnectionNum);
  return (it != _tcpConnections.end());
}
void Connection::addSession(const std::string &sessionID, Proto::Acknowledge acknowledgeType) {
  auto it = _sessions.find(sessionID);
  if (it.hasValue()) {
    throw EXCEPTION("session already exists", sessionID, ERROR_ON_SESSION);
  }
  _sessions.insert(std::make_pair(sessionID, std::make_unique<upmq::broker::Session>(*this, sessionID, acknowledgeType)));
}
void Connection::removeSession(const std::string &sessionID, size_t tcpNum) {
  {
    auto it = _sessions.find(sessionID);
    if (it.hasValue()) {
      try {
        (*it)->removeSenders();
        (*it)->closeSubscriptions(tcpNum);
      } catch (Exception &ex) {
        if (ex.error() != ERROR_UNKNOWN) {
          throw Exception(ex);
        }
      }
    }
  }

  _sessions.erase(sessionID);
}
void Connection::beginTX(const std::string &sessionID) {
  auto it = _sessions.find(sessionID);
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_BEGIN);
  }
  (*it)->begin();
}
void Connection::commitTX(const std::string &sessionID) {
  auto it = _sessions.find(sessionID);
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_COMMIT);
  }
  (*it)->commit();
}
void Connection::abortTX(const std::string &sessionID) {
  auto it = _sessions.find(sessionID);
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_ABORT);
  }
  (*it)->abort();
}
void Connection::saveMessage(const MessageDataContainer &sMessage) {
  auto it = _sessions.find(sMessage.message().session_id());
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sMessage.message().session_id(), ERROR_ON_SAVE_MESSAGE);
  }
  (*it)->saveMessage(sMessage);
}
const std::string &Connection::sessionsT() const { return _sessionsT; }
const std::string &Connection::tcpT() const { return _tcpT; }
void Connection::addSender(const MessageDataContainer &sMessage) {
  const Proto::Sender &sender = sMessage.sender();
  auto it = _sessions.find(sender.session_id());
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sender.session_id(), ERROR_ON_SENDER);
  }
  (*it)->addSender(sMessage);
}
void Connection::removeSender(const MessageDataContainer &sMessage) {
  auto it = _sessions.find(sMessage.unsender().session_id());
  if (it.hasValue()) {
    (*it)->removeSender(sMessage);
  }
}
void Connection::addSubscription(const MessageDataContainer &sMessage) {
  auto it = _sessions.find(sMessage.subscription().session_id());
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sMessage.subscription().session_id(), ERROR_ON_SUBSCRIPTION);
  }

  (*it)->addSubscription(sMessage);
}
void Connection::removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum) {
  {
    auto it = _sessions.find(sMessage.unsubscription().session_id());
    if (!it.hasValue()) {
      throw EXCEPTION("session not found", sMessage.unsubscription().session_id(), ERROR_ON_UNSUBSCRIPTION);
    }
  }

  EXCHANGE::Instance().removeConsumer(sMessage, tcpNum);
}
void Connection::removeConsumers(const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum) {
  std::vector<std::string> ids;
  {
    ids.reserve(_sessions.size());
    _sessions.applyForEach([&ids](const SessionsList::ItemType::KVPair &pair) { ids.emplace_back(pair.second->id()); });
  }
  std::for_each(ids.begin(), ids.end(), [&destinationID, &subscriptionID, &tcpNum](const std::string &sessionId) {
    EXCHANGE::Instance().removeConsumer(sessionId, destinationID, subscriptionID, tcpNum);
  });
}
void Connection::processAcknowledge(const MessageDataContainer &sMessage) {
  auto it = _sessions.find(sMessage.ack().session_id());
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sMessage.ack().session_id(), ERROR_ON_ACK_MESSAGE);
  }
  (*it)->processAcknowledge(sMessage);
}
int Connection::maxNotAcknowledgedMessages(size_t tcpConnectionNum) const {
  auto handler = AHRegestry::Instance().aHandler(tcpConnectionNum);
  if (handler) {
    return handler->maxNotAcknowledgedMessages();
  }
  return 100;
}
std::string Connection::transactionID(const std::string &sessionID) const {
  auto it = _sessions.find(sessionID);
  if (!it.hasValue()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_SESSION);
  }
  return (*it)->txName();
}
size_t Connection::tcpConnectionsCount() const {
  upmq::ScopedReadRWLock readRWLock(_tcpLock);
  return _tcpConnections.size();
}
}  // namespace broker
}  // namespace upmq
