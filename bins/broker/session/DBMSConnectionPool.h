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

#include "Singleton.h"
#include "IConnectionPool.h"

namespace upmq {
namespace broker {
namespace storage {
class DBMSConnectionPool {
 public:
  enum class TX : int { NOT_USE = 0, USE };
  DBMSConnectionPool();
  DBMSConnectionPool(const DBMSConnectionPool &) = delete;
  DBMSConnectionPool(DBMSConnectionPool &&) = delete;
  virtual ~DBMSConnectionPool();

  std::shared_ptr<Poco::Data::Session> dbmsConnection() const;

  void pushBack(std::shared_ptr<Poco::Data::Session> session);
  void doNow(const std::string &sql, DBMSConnectionPool::TX tx = TX::USE);

  void beginTX(Poco::Data::Session &dbSession,
               const std::string &txName,
               storage::DBMSSession::TransactionMode mode = storage::DBMSSession::TransactionMode::WRITE);
  void commitTX(Poco::Data::Session &dbSession, const std::string &txName);
  void rollbackTX(Poco::Data::Session &dbSession, const std::string &txName);

  DBMSSession dbmsSession() const;
  std::unique_ptr<DBMSSession> dbmsSessionPtr() const;

 private:
  std::unique_ptr<IConnectionPool> _impl;
};
}  // namespace storage
}  // namespace broker
}  // namespace upmq

using dbms = Singleton<upmq::broker::storage::DBMSConnectionPool>;
using DBMSConnection = std::shared_ptr<Poco::Data::Session>;

#endif  // BROKER_DBMSCONNECTIONPOOL_H
