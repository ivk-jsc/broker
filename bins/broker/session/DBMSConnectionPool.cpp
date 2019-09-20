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
#include <Poco/Data/ODBC/Connector.h>
#ifdef HAS_POSTGRESQL
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/PostgreSQL/PostgreSQLException.h"
#include "Poco/Data/PostgreSQL/Utility.h"
#endif
#endif
static constexpr char SQLITE_CONNECTOR_STR[] = "SQLite";

#include <Poco/UUIDGenerator.h>
#include <fake_cpp14.h>
#include "MiscDefines.h"
#include "ProtoBuf.h"

namespace upmq {
namespace broker {
namespace storage {

DBMSConnectionPool::DBMSConnectionPool()
    : _count(STORAGE_CONFIG.connection.props.connectionPool), _dbmsString(STORAGE_CONFIG.connection.value.get()) {
  TRY_POCO_DATA_EXCEPTION {
    std::string connector;
    DBMSType dbmsType = STORAGE_CONFIG.connection.props.dbmsType;
    if (dbmsType == storage::SQLiteNative) {
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
      connector = SQLITE_CONNECTOR_STR;
      PDSQLITE::Connector::enableSharedCache();
    } else if (dbmsType == storage::Postgresql) {
#ifdef HAS_POSTGRESQL
      Poco::Data::PostgreSQL::Connector::registerConnector();
      connector = "postgresql";
#endif  // HAS_POSTGRESQL
    } else {
      Poco::Data::ODBC::Connector::registerConnector();
      connector = "ODBC";
    }
    if (_dbmsString == ":memory:") {
      _inMemory = IN_MEMORY::M_YES;
      _count = 1;
      _memorySession = makeSession(dbmsType, connector);
      initDB(*_memorySession);
    } else {
      for (int i = 0; i < _count; i++) {
        std::shared_ptr<Poco::Data::Session> session = makeSession(dbmsType, connector);
        if (i == 0) {
          initDB(*session);
        }
        _sessions.enqueue(session);
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("create dbms connect", _dbmsString, Proto::ERROR_STORAGE);
}
DBMSConnectionPool::~DBMSConnectionPool() = default;

std::shared_ptr<Poco::Data::Session> DBMSConnectionPool::dbmsConnection() const {
  switch (_inMemory) {
    case IN_MEMORY::M_YES:
      return _memorySession;
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

void DBMSConnectionPool::beginTX(Poco::Data::Session &dbSession, const std::string &txName) {
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
    //    dbSession.setFeature("autoCommit", false);
    dbSession << "BEGIN TRANSACTION;", Poco::Data::Keywords::now;
  }
#endif  // HAS_POSTGRESQL
  else {

    std::string transactionType = (_inMemory == IN_MEMORY::M_YES ? "immediate" : "exclusive");

    bool locked;
    std::stringstream sql;
    sql << "begin " << transactionType << " transaction \"" << txName << Poco::Thread::currentTid() << "\";";  // immediate
    do {
      locked = false;
      try {
        if (STORAGE_CONFIG.connection.props.dbmsType == storage::SQLite) {
          dbSession.setFeature("autoCommit", false);
        }
        dbSession << sql.str(), Poco::Data::Keywords::now;
      } catch (PDSQLITE::DBLockedException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (PDSQLITE::TableLockedException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (PDSQLITE::InvalidSQLStatementException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (Poco::Exception &pex) {
        std::cout << " ! ERROR : " << pex.message() << non_std_endl;
      }
    } while (locked);
  }
}
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
    //    dbSession.setFeature("autoCommit", true);
  }
#endif  // HAS_POSTGRESQL
  else {
    bool locked;
    std::stringstream sql;
    sql << "commit transaction \"" << txName << Poco::Thread::currentTid() << "\";";
    do {
      locked = false;
      try {
        dbSession << sql.str(), Poco::Data::Keywords::now;
        if (STORAGE_CONFIG.connection.props.dbmsType == storage::SQLite) {
          dbSession.setFeature("autoCommit", true);
        }
      } catch (PDSQLITE::DBLockedException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (PDSQLITE::TableLockedException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (PDSQLITE::InvalidSQLStatementException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (Poco::Exception &pex) {
        std::cout << " ! ERROR : " << pex.message() << non_std_endl;
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
    //    dbSession.setFeature("autoCommit", true);
  }
#endif  // HAS_POSTGRESQL
  else {
    bool locked;
    std::stringstream sql;
    sql << "rollback transaction \"" << txName << Poco::Thread::currentTid() << "\";";
    do {
      locked = false;
      try {
        dbSession << sql.str(), Poco::Data::Keywords::now;
        if (STORAGE_CONFIG.connection.props.dbmsType == storage::SQLite) {
          dbSession.setFeature("autoCommit", true);
        }
      } catch (PDSQLITE::DBLockedException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (PDSQLITE::TableLockedException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (PDSQLITE::InvalidSQLStatementException &) {
        locked = true;
        Poco::Thread::yield();
      } catch (Poco::Exception &pex) {
        std::cout << " ! ERROR : " << pex.message() << non_std_endl;
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
