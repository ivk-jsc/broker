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

#ifndef BROKER_DBMSCONNECTIONPOOL_H
#define BROKER_DBMSCONNECTIONPOOL_H
#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/AbstractBinder.h>
#include <Poco/SQL/Session.h>
namespace Poco {
namespace Data = SQL;
}
#else
#include <Poco/Data/AbstractBinder.h>
#include <Poco/Data/Session.h>
#endif
#include <memory>
#include <mutex>
#include "ConcurrentQueueHeader.h"
#include "Configuration.h"
#include "DBMSSession.h"
#include "Singleton.h"
#include "FixedSizeUnorderedMap.h"

namespace upmq {
namespace broker {
namespace storage {
class DBMSConnectionPool {
 public:
  enum class TX : int { NOT_USE = 0, USE };
  enum class IN_MEMORY : int { M_YES, M_NO };
  typedef moodycamel::ConcurrentQueue<std::shared_ptr<Poco::Data::Session>> SessionsQueueType;
  DBMSConnectionPool();
  virtual ~DBMSConnectionPool();
  std::shared_ptr<Poco::Data::Session> dbmsConnection() const;
  void pushBack(std::shared_ptr<Poco::Data::Session> session);
  static void doNow(const std::string &sql, DBMSConnectionPool::TX tx = TX::USE);

  void beginTX(Poco::Data::Session &dbSession,
               const std::string &txName,
               storage::DBMSSession::TransactionMode mode = storage::DBMSSession::TransactionMode::WRITE);
  static void commitTX(Poco::Data::Session &dbSession, const std::string &txName);
  static void rollbackTX(Poco::Data::Session &dbSession, const std::string &txName);

  DBMSSession dbmsSession() const;
  std::unique_ptr<DBMSSession> dbmsSessionPtr() const;

 private:
  int _count;
  mutable SessionsQueueType _sessions;
  std::string _dbmsString;
  DBMSType _dbmsType;
  std::string _connector;
  mutable FSUnorderedMap<Poco::UInt64, std::shared_ptr<Poco::Data::Session>> _memorySession;
  static void initDB(Poco::Data::Session &dbSessionstatic);
  std::shared_ptr<Poco::Data::Session> makeSession(DBMSType dbmsType, const std::string &connector) const;
  IN_MEMORY _inMemory{IN_MEMORY::M_NO};
  static Poco::Timestamp _lastBegin;
};
}  // namespace storage
}  // namespace broker
}  // namespace upmq

typedef Singleton<upmq::broker::storage::DBMSConnectionPool> dbms;
typedef std::shared_ptr<Poco::Data::Session> DBMSConnection;

#endif  // BROKER_DBMSCONNECTIONPOOL_H
