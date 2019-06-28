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
    : _clientID(clientID), _clientIDWasSet(!clientID.empty()), _sessionsT("\"" + clientID + "_sessions\""), _tcpT("\"" + clientID + "_tcp_connections\"") {
  std::stringstream sql;
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(_clientID);
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
  dbSession.commitTX();
}
Connection::~Connection() {
  _sessions.clear();
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
  upmq::ScopedWriteRWLock writeRWLock(_tcpLock);
  auto it = _tcpConnections.find(tcpConnectionNum);
  if (it == _tcpConnections.end()) {
    _tcpConnections.insert(tcpConnectionNum);
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
  upmq::ScopedWriteRWLock writeRWLock(_tcpLock);
  auto it = _tcpConnections.find(tcpConnectionNum);
  if (it != _tcpConnections.end()) {
    _tcpConnections.erase(it);
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
  upmq::ScopedWriteRWLock writeRWLock(_sessionsLock);
  auto it = _sessions.find(sessionID);
  if (it != _sessions.end()) {
    throw EXCEPTION("session already exists", sessionID, ERROR_ON_SESSION);
  }
  _sessions.insert(std::make_pair(sessionID, std::make_unique<upmq::broker::Session>(*this, sessionID, acknowledgeType)));
}
void Connection::removeSession(const std::string &sessionID, size_t tcpNum) {
  {
    upmq::ScopedReadRWLock readRWLock(_sessionsLock);
    auto it = _sessions.find(sessionID);
    if (it != _sessions.end()) {
      try {
        it->second->removeSenders();
        it->second->closeSubscriptions(tcpNum);
      } catch (Exception &ex) {
        if (ex.error() != ERROR_UNKNOWN) {
          throw Exception(ex);
        }
      }
    }
  }
  upmq::ScopedWriteRWLock writeRWLock(_sessionsLock);
  auto it = _sessions.find(sessionID);
  if (it != _sessions.end()) {
    _sessions.erase(it);
  }
}
void Connection::beginTX(const std::string &sessionID) {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  auto it = _sessions.find(sessionID);
  if (it == _sessions.end()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_BEGIN);
  }
  it->second->begin();
}
void Connection::commitTX(const std::string &sessionID) {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  auto it = _sessions.find(sessionID);
  if (it == _sessions.end()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_COMMIT);
  }
  it->second->commit();
}
void Connection::abortTX(const std::string &sessionID) {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  auto it = _sessions.find(sessionID);
  if (it == _sessions.end()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_ABORT);
  }
  it->second->abort();
}
void Connection::saveMessage(const MessageDataContainer &sMessage) {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  auto it = _sessions.find(sMessage.message().session_id());
  if (it == _sessions.end()) {
    throw EXCEPTION("session not found", sMessage.message().session_id(), ERROR_ON_SAVE_MESSAGE);
  }
  it->second->saveMessage(sMessage);
}
const std::string &Connection::sessionsT() const { return _sessionsT; }
const std::string &Connection::tcpT() const { return _tcpT; }
void Connection::addSender(const MessageDataContainer &sMessage) {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  const Proto::Sender &sender = sMessage.sender();
  auto it = _sessions.find(sender.session_id());
  if (it == _sessions.end()) {
    throw EXCEPTION("session not found", sender.session_id(), ERROR_ON_SENDER);
  }
  it->second->addSender(sMessage);
}
void Connection::removeSender(const MessageDataContainer &sMessage) {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  auto it = _sessions.find(sMessage.unsender().session_id());
  if (it != _sessions.end()) {
    it->second->removeSender(sMessage);
  }
}
void Connection::addSubscription(const MessageDataContainer &sMessage) {
  SessionsList::iterator it;
  {
    upmq::ScopedReadRWLock readRWLock(_sessionsLock);
    it = _sessions.find(sMessage.subscription().session_id());
    if (it == _sessions.end()) {
      throw EXCEPTION("session not found", sMessage.subscription().session_id(), ERROR_ON_SUBSCRIPTION);
    }
  }
  it->second->addSubscription(sMessage);
}
void Connection::removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum) {
  {
    upmq::ScopedReadRWLock readRWLock(_sessionsLock);
    auto it = _sessions.find(sMessage.unsubscription().session_id());
    if (it == _sessions.end()) {
      throw EXCEPTION("session not found", sMessage.unsubscription().session_id(), ERROR_ON_UNSUBSCRIPTION);
    }
  }

  EXCHANGE::Instance().removeConsumer(sMessage, tcpNum);
}
void Connection::removeConsumers(const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum) {
  std::vector<std::string> ids;
  {
    upmq::ScopedReadRWLock readRWLock(_sessionsLock);
    ids.reserve(_sessions.size());
    for (const auto &sess : _sessions) {
      ids.emplace_back(sess.second->id());
    }
  }
  std::for_each(
      ids.begin(), ids.end(), [&destinationID, &subscriptionID, &tcpNum](const std::string &sessionId) { EXCHANGE::Instance().removeConsumer(sessionId, destinationID, subscriptionID, tcpNum); });
}
void Connection::processAcknowledge(const MessageDataContainer &sMessage) {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  auto it = _sessions.find(sMessage.ack().session_id());
  if (it == _sessions.end()) {
    throw EXCEPTION("session not found", sMessage.ack().session_id(), ERROR_ON_ACK_MESSAGE);
  }
  it->second->processAcknowledge(sMessage);
}
int Connection::maxNotAcknowledgedMessages(size_t tcpConnectionNum) const {
  auto handler = AHRegestry::Instance().aHandler(tcpConnectionNum);
  if (handler) {
    return handler->maxNotAcknowledgedMessages();
  }
  return 100;
}
std::string Connection::transactionID(const std::string &sessionID) const {
  upmq::ScopedReadRWLock readRWLock(_sessionsLock);
  auto it = _sessions.find(sessionID);
  if (it == _sessions.end()) {
    throw EXCEPTION("session not found", sessionID, ERROR_ON_SESSION);
  }
  return it->second->txName();
}
size_t Connection::tcpConnectionsCount() const {
  upmq::ScopedReadRWLock readRWLock(_tcpLock);
  return _tcpConnections.size();
}
}  // namespace broker
}  // namespace upmq
