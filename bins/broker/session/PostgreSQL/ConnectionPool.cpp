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
#ifdef HAS_POSTGRESQL
#include "Poco/SQL/PostgreSQL/Connector.h"
#include "Poco/SQL/PostgreSQL/PostgreSQLException.h"
#include "Poco/SQL/PostgreSQL/Utility.h"
#endif
#else
#ifdef HAS_POSTGRESQL
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/PostgreSQL/PostgreSQLException.h"
#include "Poco/Data/PostgreSQL/Utility.h"
#endif
#endif

static constexpr char POSTGRES_CONNECTOR_STR[] = "Postgresql";

namespace upmq {
namespace broker {
namespace storage {
namespace postgresql {
ConnectionPool::ConnectionPool() : IConnectionPool(STORAGE_CONFIG.connection.props.connectionPool, STORAGE_CONFIG.connection.value.get()) {
  Poco::Data::PostgreSQL::Connector::registerConnector();

  for (int i = 0; i < count; i++) {
    std::shared_ptr<Poco::Data::Session> session = makeSession();
    sessions.enqueue(session);
  }
}
ConnectionPool::~ConnectionPool() {
  std::shared_ptr<Poco::Data::Session> session;
  for (int i = 0; i < count; i++) {
    sessions.try_dequeue(session);
    session.reset();
  }
}
std::shared_ptr<Poco::Data::Session> ConnectionPool::makeSession() const {
  return std::make_shared<Poco::Data::Session>(POSTGRES_CONNECTOR_STR, dbmsString);
}
std::shared_ptr<Poco::Data::Session> ConnectionPool::dbmsConnection() const {
  std::shared_ptr<Poco::Data::Session> session;
  do {
    sessions.try_dequeue(session);
  } while (session == nullptr);
  return session;
}
void ConnectionPool::pushBack(std::shared_ptr<Poco::Data::Session> session) {
  if (session) {
    sessions.enqueue(std::move(session));
  }
}
void ConnectionPool::beginTX(Poco::Data::Session &dbSession, const std::string &txName, storage::DBMSSession::TransactionMode mode) {
  std::stringstream sql;
  sql << "BEGIN TRANSACTION";
  if (mode == storage::DBMSSession::TransactionMode::READ) {
    sql << " READ ONLY";
  } else {
    sql << " READ WRITE";
  }
  dbSession << sql.str(), Poco::Data::Keywords::now;
}
void ConnectionPool::commitTX(Poco::Data::Session &dbSession, const std::string &txName) { dbSession << "COMMIT;", Poco::Data::Keywords::now; }
void ConnectionPool::rollbackTX(Poco::Data::Session &dbSession, const std::string &txName) { dbSession << "ROLLBACK;", Poco::Data::Keywords::now; }
}  // namespace postgresql
}  // namespace storage
}  // namespace broker
}  // namespace upmq