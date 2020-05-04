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

#include "DBMSConnectionPool.h"
#include <Poco/File.h>
#if POCO_VERSION_MAJOR > 1
#include "Poco/SQL/SQLite/Connector.h"
#include "Poco/SQL/SQLite/SQLiteException.h"
#include "Poco/SQL/SQLite/Utility.h"
#include <Poco/SQL/ODBC/Connector.h>
#ifdef HAS_POSTGRESQL
#include "Poco/SQL/PostgreSQL/Connector.h"
#include "Poco/SQL/PostgreSQL/PostgreSQLException.h"
#include "Poco/SQL/PostgreSQL/Utility.h"
#endif
#else
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/SQLiteException.h"
#include "Poco/Data/SQLite/Utility.h"
#include "sqlite3.h"
#ifdef HAS_ODBC
#include <Poco/Data/ODBC/Connector.h>
#endif
#ifdef HAS_POSTGRESQL
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/PostgreSQL/PostgreSQLException.h"
#include "Poco/Data/PostgreSQL/Utility.h"
#endif
#endif
static constexpr char SQLITE_CONNECTOR_STR[] = "SQLite";

#include <Poco/UUIDGenerator.h>
#include <fake_cpp14.h>
#ifdef SQLITE_TRACE
#include <sqlite3.h>
#endif  // SQLITE_TRACE
#include "MiscDefines.h"
#include "ProtoBuf.h"
#include "AsyncLogger.h"
#include "Poco/RWLock.h"

namespace upmq {
namespace broker {
namespace storage {

Poco::Timestamp DBMSConnectionPool::_lastBegin;

DBMSConnectionPool::DBMSConnectionPool()
    : _count(STORAGE_CONFIG.connection.props.connectionPool),
      _dbmsString(STORAGE_CONFIG.connection.value.get()),
      _dbmsType(STORAGE_CONFIG.connection.props.dbmsType),
      _memorySession(THREADS_CONFIG.all() + 1) {
  TRY_POCO_DATA_EXCEPTION {
    if (_dbmsType == storage::SQLiteNative) {
      if (STORAGE_CONFIG.connection.value.usePath && _dbmsString.find(":memory:") == std::string::npos) {
        Poco::Path dbmsFilePath = STORAGE_CONFIG.connection.path;
        Poco::File dbmsFile(dbmsFilePath);
        if (!dbmsFile.exists()) {
          dbmsFile.createDirectories();
        }
        dbmsFilePath.append(_dbmsString);
        _dbmsString = dbmsFilePath.toString();
      }
      PDSQLITE::Connector::registerConnector();
      _connector = SQLITE_CONNECTOR_STR;
      PDSQLITE::Connector::enableSharedCache();
    } else if (_dbmsType == storage::Postgresql) {
#ifdef HAS_POSTGRESQL
      Poco::Data::PostgreSQL::Connector::registerConnector();
      _connector = "postgresql";
#endif  // HAS_POSTGRESQL
    } else {
#ifdef HAS_ODBC
      Poco::Data::ODBC::Connector::registerConnector();
      _connector = "ODBC";
#endif
    }
    if (_dbmsString.find(":memory:") != std::string::npos) {
      _inMemory = IN_MEMORY::M_YES;
      _count = 0;
      auto tempSession = makeSession(_dbmsType, _connector);
      initDB(*tempSession);
    } else {
      for (int i = 0; i < _count; i++) {
        std::shared_ptr<Poco::Data::Session> session = makeSession(_dbmsType, _connector);
        if (i == 0) {
          initDB(*session);
        }
        _sessions.enqueue(session);
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("create dbms connect", _dbmsString, Proto::ERROR_STORAGE);
}
DBMSConnectionPool::~DBMSConnectionPool() {
  if (_dbmsString == ":memory:") {
    _memorySession.clear();
  } else {
    std::shared_ptr<Poco::Data::Session> session;
    for (int i = 0; i < _count; i++) {
      _sessions.try_dequeue(session);
      session.reset();
    }
  }
}

std::shared_ptr<Poco::Data::Session> DBMSConnectionPool::dbmsConnection() const {
  switch (_inMemory) {
    case IN_MEMORY::M_YES: {
      auto tid = reinterpret_cast<Poco::UInt64>(Poco::Thread::currentTid());
      auto sess = _memorySession.find(tid);
      if (!sess.hasValue()) {
        _memorySession.emplace(Poco::UInt64(tid), makeSession(_dbmsType, _connector));
        sess = _memorySession.find(tid);
      }
      return *sess;
    }
      // case IN_MEMORY::M_NO:
    default: {
      std::shared_ptr<Poco::Data::Session> session;
      do {
        _sessions.try_dequeue(session);
      } while (session == nullptr);
      return session;
    }
  }
}
void DBMSConnectionPool::pushBack(std::shared_ptr<Poco::Data::Session> session) {
  switch (_inMemory) {
    case IN_MEMORY::M_YES:
      return;
    case IN_MEMORY::M_NO:
    default:
      if (session) {
        _sessions.enqueue(std::move(session));
      }
  }
}

void DBMSConnectionPool::beginTX(Poco::Data::Session &dbSession, const std::string &txName, storage::DBMSSession::TransactionMode mode) {
  if ((STORAGE_CONFIG.connection.props.dbmsType != storage::SQLiteNative) && (STORAGE_CONFIG.connection.props.dbmsType != storage::SQLite)
#ifdef HAS_POSTGRESQL
      && (STORAGE_CONFIG.connection.props.dbmsType != storage::Postgresql))
#else
  )
#endif  // HAS_POSTGRESQL
  {
    dbSession.begin();
  }
#ifdef HAS_POSTGRESQL
  else if (STORAGE_CONFIG.connection.props.dbmsType == storage::Postgresql) {
    std::stringstream sql;
    sql << "BEGIN TRANSACTION";
    if (mode == storage::DBMSSession::TransactionMode::READ) {
      sql << " READ ONLY";
    } else {
      sql << " READ WRITE";
    }
    dbSession << sql.str(), Poco::Data::Keywords::now;
  }
#endif  // HAS_POSTGRESQL
  else {
    while (dbSession.isTransaction()) {
      Poco::Thread::yield();
    }

    bool locked;

    const std::string sql = (mode == storage::DBMSSession::TransactionMode::WRITE) ? "begin concurrent;" : "begin transaction;";
    int result = 0;
    sqlite3 *pSqlite3 = Poco::Data::SQLite::Utility::dbHandle(dbSession);
    const char *cquery = sql.c_str();
    do {
      result = sqlite3_exec(pSqlite3, cquery, nullptr, nullptr, nullptr);
      locked = (result == SQLITE_LOCKED || result == SQLITE_BUSY || result == SQLITE_BUSY_SNAPSHOT);
      if (locked) {
        Poco::Thread::yield();
      }
    } while (locked);
  }
}  // namespace storage
void DBMSConnectionPool::commitTX(Poco::Data::Session &dbSession, const std::string &txName) {
  if ((STORAGE_CONFIG.connection.props.dbmsType != storage::SQLiteNative) && (STORAGE_CONFIG.connection.props.dbmsType != storage::SQLite)
#ifdef HAS_POSTGRESQL
      && (STORAGE_CONFIG.connection.props.dbmsType != storage::Postgresql))
#else
  )
#endif  // HAS_POSTGRESQL
  {
    dbSession.commit();
  }
#ifdef HAS_POSTGRESQL
  else if (STORAGE_CONFIG.connection.props.dbmsType == storage::Postgresql) {
    dbSession << "COMMIT;", Poco::Data::Keywords::now;
  }
#endif  // HAS_POSTGRESQL
  else {
    bool locked;
    const std::string sql = "commit;";
    int result = 0;
    sqlite3 *pSqlite3 = Poco::Data::SQLite::Utility::dbHandle(dbSession);
    const char *cquery = sql.c_str();
    do {
      result = sqlite3_exec(pSqlite3, cquery, nullptr, nullptr, nullptr);
      locked = (result == SQLITE_LOCKED || result == SQLITE_BUSY || result == SQLITE_BUSY_SNAPSHOT);
      if (locked) {
        Poco::Thread::yield();
      }
    } while (locked);
  }
}
void DBMSConnectionPool::rollbackTX(Poco::Data::Session &dbSession, const std::string &txName) {
  if ((STORAGE_CONFIG.connection.props.dbmsType != storage::SQLiteNative) && (STORAGE_CONFIG.connection.props.dbmsType != storage::SQLite)
#ifdef HAS_POSTGRESQL
      && (STORAGE_CONFIG.connection.props.dbmsType != storage::Postgresql))
#else
  )
#endif  // HAS_POSTGRESQL
  {
    dbSession.rollback();
  }
#ifdef HAS_POSTGRESQL
  else if (STORAGE_CONFIG.connection.props.dbmsType == storage::Postgresql) {
    dbSession << "ROLLBACK;", Poco::Data::Keywords::now;
  }
#endif  // HAS_POSTGRESQL
  else {
    bool locked;
    const std::string sql = "rollback;";
    int result = 0;
    sqlite3 *pSqlite3 = Poco::Data::SQLite::Utility::dbHandle(dbSession);
    const char *cquery = sql.c_str();
    do {
      result = sqlite3_exec(pSqlite3, cquery, nullptr, nullptr, nullptr);
      locked = (result == SQLITE_LOCKED || result == SQLITE_BUSY || result == SQLITE_BUSY_SNAPSHOT);
      if (locked) {
        Poco::Thread::yield();
      }
    } while (locked);
  }
}
void DBMSConnectionPool::doNow(const std::string &sql, DBMSConnectionPool::TX tx) {
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  std::string txName;
  if (tx == TX::USE) {
    txName = std::to_string((size_t)(Poco::Thread::currentTid()));
  }
  try {
    if (tx == TX::USE) {
      dbSession.beginTX(txName);
    }
    dbSession << sql, Poco::Data::Keywords::now;
    if (tx == TX::USE) {
      dbSession.commitTX();
    }
  } catch (Poco::Exception &pex) {
    if (tx == TX::USE) {
      dbSession.rollbackTX();
    }
    pex.rethrow();
  } catch (...) {
    if (tx == TX::USE) {
      dbSession.rollbackTX();
    }
    throw;
  }
}
void DBMSConnectionPool::initDB(Poco::Data::Session &dbSession) {
  switch (STORAGE_CONFIG.connection.props.dbmsType) {
    case NO_TYPE:
    case Postgresql:
      break;
    case SQLite:
    case SQLiteNative: {
      std::vector<std::string> drops;
      dbSession << "SELECT 'drop table if exists \"' || tbl_name || '\"' from "
                   "sqlite_master where tbl_name like "
                   "'%_tcp_connections' or tbl_name like '%_sessions' and "
                   "type='table';",
          Poco::Data::Keywords::into(drops), Poco::Data::Keywords::now;
      for (const auto &drop : drops) {
        dbSession << drop, Poco::Data::Keywords::now;
      }
#ifdef SQLITE_TRACE
      unsigned uMask = SQLITE_TRACE_PROFILE;
      ASYNCLOGGER::Instance().add("sqlite_trace");
      auto callBack = [](unsigned reason, void *ctx, void *p, void *x) -> int {
        if (reason == SQLITE_TRACE_PROFILE) {
          sqlite3_stmt *pStmt = (sqlite3_stmt *)p;
          double dur = *((double *)x);
          double duration = dur / 1000000.0;
          std::string sql = std::string(sqlite3_sql(pStmt));
          Poco::Logger::get("sqlite_trace").trace("[%f msec] -> %s", duration, sql);
          if (sql.find("begin") == 0) {
            ((Poco::Timestamp *)ctx)->update();
          } else if (sql.find("commit") == 0 || sql.find("rollback") == 0) {
            auto diff = ((Poco::Timestamp *)ctx)->elapsed();
            Poco::Logger::get("sqlite_trace").trace("[%ld transaction duration]", diff);
          }
        }
        return 0;
      };
      sqlite3_trace_v2(Poco::Data::SQLite::Utility::dbHandle(dbSession), uMask, callBack, &_lastBegin);
#endif
    } break;
  }
}

std::shared_ptr<Poco::Data::Session> DBMSConnectionPool::makeSession(DBMSType dbmsType, const std::string &connector) const {
  std::shared_ptr<Poco::Data::Session> session = std::make_shared<Poco::Data::Session>(connector, _dbmsString);
  if (dbmsType == storage::SQLiteNative || dbmsType == storage::SQLite) {
    if (dbmsType == storage::SQLite) {
      session->setTransactionIsolation(Poco::Data::Session::TRANSACTION_SERIALIZABLE);
    }
    *session << "PRAGMA case_sensitive_like = True;", Poco::Data::Keywords::now;
    *session << "PRAGMA synchronous = " << (STORAGE_CONFIG.connection.props.useSync ? "ON" : "OFF") << ";", Poco::Data::Keywords::now;

    *session << "PRAGMA journal_mode = " << STORAGE_CONFIG.connection.props.journalMode << " ;", Poco::Data::Keywords::now;

    *session << "PRAGMA locking_mode = EXCLUSIVE;", Poco::Data::Keywords::now;
    *session << "PRAGMA secure_delete = FALSE;", Poco::Data::Keywords::now;
  }
  return session;
}

DBMSSession DBMSConnectionPool::dbmsSession() const { return DBMSSession(dbmsConnection(), const_cast<DBMSConnectionPool &>(*this)); }
std::unique_ptr<DBMSSession> DBMSConnectionPool::dbmsSessionPtr() const {
  return std::make_unique<DBMSSession>(dbmsConnection(), const_cast<DBMSConnectionPool &>(*this));
}
}  // namespace storage
}  // namespace broker
}  // namespace upmq
