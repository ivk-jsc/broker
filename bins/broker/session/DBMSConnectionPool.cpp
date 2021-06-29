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

#include <Exception.h>
#include "DBMSConnectionPool.h"
#include "Configuration.h"
#include "AsyncLogger.h"
#include "Poco/RWLock.h"
#include "ProtoBuf.h"

#include "SQlite/ConnectionPool.h"

#ifdef HAS_POSTGRESQL
#include "PostgreSQL/ConnectionPool.h"
#endif

#ifdef HAS_ODBC
#include "ODBC/ConnectionPool.h"
#endif

namespace upmq {
namespace broker {
namespace storage {

DBMSConnectionPool::DBMSConnectionPool() {
  log = &Poco::Logger::get(CONFIGURATION::Instance().log().name);
  TRACE(log);
  OnError onError;
  onError.setError(Proto::ERROR_STORAGE).setInfo("can't create dbms connect");
  switch (STORAGE_CONFIG.connection.props.dbmsType) {
    case NO_TYPE:
      throw EXCEPTION("invalid DBMS", Configuration::Storage::typeName(STORAGE_CONFIG.connection.props.dbmsType), Proto::ERROR_STORAGE);
    case Postgresql:
#ifdef HAS_POSTGRESQL
      TRY_EXECUTE(([this]() { _impl.reset(new postgresql::ConnectionPool()); }), onError);
#endif
      break;
    case SQLite:
      break;
    case SQLiteNative:
      TRY_EXECUTE(([this]() { _impl.reset(new sqlite::ConnectionPool()); }), onError);
      break;
  }
  if (_impl == nullptr) {
    throw EXCEPTION("invalid DBMS", Configuration::Storage::typeName(STORAGE_CONFIG.connection.props.dbmsType), Proto::ERROR_STORAGE);
  }
}
DBMSConnectionPool::~DBMSConnectionPool() {
  TRACE(log);
  _impl.reset();
}

std::shared_ptr<Poco::Data::Session> DBMSConnectionPool::dbmsConnection() const {
  TRACE(log);
  return _impl->dbmsConnection();
}
void DBMSConnectionPool::pushBack(std::shared_ptr<Poco::Data::Session> session) {
  TRACE(log);
  _impl->pushBack(std::move(session));
}

void DBMSConnectionPool::beginTX(Poco::Data::Session &dbSession, const std::string &txName, storage::DBMSSession::TransactionMode mode) {
  TRACE(log);
  _impl->beginTX(dbSession, txName, mode);
}
void DBMSConnectionPool::commitTX(Poco::Data::Session &dbSession, const std::string &txName) {
  TRACE(log);
  _impl->commitTX(dbSession, txName);
}
void DBMSConnectionPool::rollbackTX(Poco::Data::Session &dbSession, const std::string &txName) {
  TRACE(log);
  _impl->rollbackTX(dbSession, txName);
}
void DBMSConnectionPool::doNow(const DBMSSession &dbmsSession, const std::string &sql, DBMSConnectionPool::TX tx) {
  TRACE(log);
  std::string txName;
  if (tx == TX::USE) {
    txName = std::to_string((size_t)(Poco::Thread::currentTid()));
  }
  Poco::Data::Session &dbSession = dbmsSession();
  try {
    if (tx == TX::USE) {
      beginTX(dbSession, txName);
    }
    _impl->runSimple(dbSession, sql);
    if (tx == TX::USE) {
      commitTX(dbSession, txName);
    }
  } catch (Poco::Exception &pex) {
    if (tx == TX::USE) {
      rollbackTX(dbSession, txName);
    }
    pex.rethrow();
  } catch (...) {
    if (tx == TX::USE) {
      rollbackTX(dbSession, txName);
    }
    throw;
  }
}

DBMSSession DBMSConnectionPool::dbmsSession() const {
  TRACE(log);
  return DBMSSession(dbmsConnection(), const_cast<DBMSConnectionPool &>(*this));
}
std::unique_ptr<DBMSSession> DBMSConnectionPool::dbmsSessionPtr() const {
  TRACE(log);
  return std::make_unique<DBMSSession>(dbmsConnection(), const_cast<DBMSConnectionPool &>(*this));
}
}  // namespace storage
}  // namespace broker
}  // namespace upmq
