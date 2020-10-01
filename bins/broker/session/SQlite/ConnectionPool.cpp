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

#include "SQlite/ConnectionPool.h"

#if POCO_VERSION_MAJOR > 1
#include "Poco/SQL/SQLite/Connector.h"
#include "Poco/SQL/SQLite/SQLiteException.h"
#include "Poco/SQL/SQLite/Utility.h"
#include <Poco/SQL/ODBC/Connector.h>
#else
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/SQLiteException.h"
#include "Poco/Data/SQLite/Utility.h"
#include "fake_sqlite.h"
#endif
#include "MiscDefines.h"
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/RWLock.h>
#include "Configuration.h"

static constexpr char SQLITE_CONNECTOR_STR[] = "SQLite";

namespace upmq {
namespace broker {
namespace storage {
namespace sqlite {

Poco::Timestamp ConnectionPool::_lastBegin;

ConnectionPool::ConnectionPool()
    : IConnectionPool(STORAGE_CONFIG.connection.props.connectionPool, STORAGE_CONFIG.connection.value.get()),
      _memorySession(THREADS_CONFIG.all() + 1) {
  if (STORAGE_CONFIG.connection.value.usePath && dbmsString.find(":memory:") == std::string::npos) {
    Poco::Path dbmsFilePath = STORAGE_CONFIG.connection.path;
    Poco::File dbmsFile(dbmsFilePath);
    if (!dbmsFile.exists()) {
      dbmsFile.createDirectories();
    }
    dbmsFilePath.append(dbmsString);
    dbmsString = dbmsFilePath.toString();
  }
  PDSQLITE::Connector::registerConnector();
  PDSQLITE::Connector::enableSharedCache();

  if (dbmsString.find(":memory:") != std::string::npos) {
    _inMemory = true;
    count = 0;
    auto tempSession = makeSession();
    initDB(*tempSession);
  } else {
    for (int i = 0; i < count; i++) {
      std::shared_ptr<Poco::Data::Session> session = makeSession();
      if (i == 0) {
        initDB(*session);
      }
      sessions.enqueue(session);
    }
  }
}
ConnectionPool::~ConnectionPool() {
  if (dbmsString == ":memory:") {
    _memorySession.clear();
  } else {
    std::shared_ptr<Poco::Data::Session> session;
    for (int i = 0; i < count; i++) {
      sessions.try_dequeue(session);
      session.reset();
    }
  }
}
void ConnectionPool::initDB(Poco::Data::Session &dbSession) {
  std::vector<std::string> drops;
  dbSession << "SELECT 'drop table if exists \"' || tbl_name || '\"' from "
               "sqlite_master where tbl_name like "
               "'%_tcp_connections' or tbl_name like '%sessions' and "
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
}
std::shared_ptr<Poco::Data::Session> ConnectionPool::makeSession() const {
  std::shared_ptr<Poco::Data::Session> session = std::make_shared<Poco::Data::Session>(SQLITE_CONNECTOR_STR, dbmsString);

  *session << "PRAGMA case_sensitive_like = True;", Poco::Data::Keywords::now;
  *session << "PRAGMA synchronous = " << (STORAGE_CONFIG.connection.props.useSync ? "ON" : "OFF") << ";", Poco::Data::Keywords::now;

  *session << "PRAGMA journal_mode = " << STORAGE_CONFIG.connection.props.journalMode << " ;", Poco::Data::Keywords::now;

  *session << "PRAGMA locking_mode = EXCLUSIVE;", Poco::Data::Keywords::now;
  *session << "PRAGMA secure_delete = FALSE;", Poco::Data::Keywords::now;

  return session;
}
std::shared_ptr<Poco::Data::Session> ConnectionPool::dbmsConnection() const {
  switch (static_cast<int>(_inMemory)) {
    case 1: {
      auto tid = reinterpret_cast<Poco::UInt64>(Poco::Thread::currentTid());
      auto sess = _memorySession.find(tid);
      if (!sess.hasValue()) {
        _memorySession.emplace(Poco::UInt64(tid), makeSession());
        sess = _memorySession.find(tid);
      }
      return *sess;
    }
    default: {
      std::shared_ptr<Poco::Data::Session> session;
      do {
        sessions.try_dequeue(session);
      } while (session == nullptr);
      return session;
    }
  }
}
void ConnectionPool::pushBack(std::shared_ptr<Poco::Data::Session> session) {
  if (!_inMemory) {
    if (session) {
      sessions.enqueue(std::move(session));
    }
  }
}
void ConnectionPool::beginTX(Poco::Data::Session &dbSession, const std::string &txName, storage::DBMSSession::TransactionMode mode) {
  bool locked;

  const std::string sql = (mode == storage::DBMSSession::TransactionMode::WRITE) ? "begin concurrent;" : "begin transaction;";

  sqlite3 *pSqlite3 = Poco::Data::SQLite::Utility::dbHandle(dbSession);
  const char *cquery = sql.c_str();
  do {
    int result = sqlite3_exec(pSqlite3, cquery, nullptr, nullptr, nullptr);
    locked = (result == SQLITE_LOCKED || result == SQLITE_BUSY || result == SQLITE_BUSY_SNAPSHOT);
    if (locked) {
      Poco::Thread::yield();
    }
  } while (locked);
}
void ConnectionPool::commitTX(Poco::Data::Session &dbSession, const std::string &txName) {
  bool locked;
  const std::string sql = "commit;";

  sqlite3 *pSqlite3 = Poco::Data::SQLite::Utility::dbHandle(dbSession);
  const char *cquery = sql.c_str();
  do {
    int result = sqlite3_exec(pSqlite3, cquery, nullptr, nullptr, nullptr);
    locked = (result == SQLITE_LOCKED || result == SQLITE_BUSY || result == SQLITE_BUSY_SNAPSHOT);
    if (locked) {
      Poco::Thread::yield();
    }
  } while (locked);
}
void ConnectionPool::rollbackTX(Poco::Data::Session &dbSession, const std::string &txName) {
  bool locked;
  const std::string sql = "rollback;";

  sqlite3 *pSqlite3 = Poco::Data::SQLite::Utility::dbHandle(dbSession);
  const char *cquery = sql.c_str();
  do {
    int result = sqlite3_exec(pSqlite3, cquery, nullptr, nullptr, nullptr);
    locked = (result == SQLITE_LOCKED || result == SQLITE_BUSY || result == SQLITE_BUSY_SNAPSHOT);
    if (locked) {
      Poco::Thread::yield();
    }
  } while (locked);
}
}  // namespace sqlite
}  // namespace storage
}  // namespace broker
}  // namespace upmq