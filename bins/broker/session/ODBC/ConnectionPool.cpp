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

#include <ProtoBuf.h>
#include "Configuration.h"
#include "ConnectionPool.h"
#include "MiscDefines.h"

#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/ODBC/Connector.h>
#else
#include <Poco/Data/ODBC/Connector.h>
#endif

static constexpr char ODBC_CONNECTOR_STR[] = "ODBC";

namespace upmq {
namespace broker {
namespace storage {
namespace odbc {
ConnectionPool::ConnectionPool() : _count(STORAGE_CONFIG.connection.props.connectionPool), _dbmsString(STORAGE_CONFIG.connection.value.get()) {
  TRY_POCO_DATA_EXCEPTION {
    Poco::Data::ODBC::Connector::registerConnector();

    for (int i = 0; i < _count; i++) {
      std::shared_ptr<Poco::Data::Session> session = makeSession();
      _sessions.enqueue(session);
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("create dbms connect", _dbmsString, Proto::ERROR_STORAGE);
}
ConnectionPool::~ConnectionPool() {
  std::shared_ptr<Poco::Data::Session> session;
  for (int i = 0; i < _count; i++) {
    _sessions.try_dequeue(session);
    session.reset();
  }
}
std::shared_ptr<Poco::Data::Session> ConnectionPool::makeSession() const {
  return std::make_shared<Poco::Data::Session>(ODBC_CONNECTOR_STR, _dbmsString);
}
std::shared_ptr<Poco::Data::Session> ConnectionPool::dbmsConnection() const {
  std::shared_ptr<Poco::Data::Session> session;
  do {
    _sessions.try_dequeue(session);
  } while (session == nullptr);
  return session;
}
void ConnectionPool::pushBack(std::shared_ptr<Poco::Data::Session> session) {
  if (session) {
    _sessions.enqueue(std::move(session));
  }
}
void ConnectionPool::beginTX(Poco::Data::Session &dbSession, const std::string &txName, storage::DBMSSession::TransactionMode mode) {
  dbSession.begin();
}
void ConnectionPool::commitTX(Poco::Data::Session &dbSession, const std::string &txName) { dbSession.commit(); }
void ConnectionPool::rollbackTX(Poco::Data::Session &dbSession, const std::string &txName) { dbSession.rollback() }

}  // namespace odbc
}  // namespace storage
}  // namespace broker
}  // namespace upmq