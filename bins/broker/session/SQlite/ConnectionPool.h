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

#ifndef UPMQ_BINS_BROKER_SESSION_SQLITE_CONNECTIONPOOL_H_
#define UPMQ_BINS_BROKER_SESSION_SQLITE_CONNECTIONPOOL_H_

#include "IConnectionPool.h"
#include "FixedSizeUnorderedMap.h"

namespace upmq {
namespace broker {
namespace storage {
namespace sqlite {
class ConnectionPool : public IConnectionPool {
  bool _inMemory = false;
  static Poco::Timestamp _lastBegin;
  mutable FSUnorderedMap<Poco::UInt64, std::shared_ptr<Poco::Data::Session>> _memorySession;

  static void initDB(Poco::Data::Session &dbSessionstatic);
  std::shared_ptr<Poco::Data::Session> makeSession() const;

 public:
  ConnectionPool();
  ConnectionPool(const ConnectionPool &) = delete;
  ConnectionPool(ConnectionPool &&) = delete;
  ConnectionPool &operator=(const ConnectionPool &) = delete;
  ConnectionPool &operator=(ConnectionPool &&) = delete;
  ~ConnectionPool() override;

  std::shared_ptr<Poco::Data::Session> dbmsConnection() const override;
  void pushBack(std::shared_ptr<Poco::Data::Session> session) override;

  void beginTX(Poco::Data::Session &dbSession, const std::string &txName, storage::DBMSSession::TransactionMode mode) override;
  void commitTX(Poco::Data::Session &dbSession, const std::string &txName) override;
  void rollbackTX(Poco::Data::Session &dbSession, const std::string &txName) override;
  void runSimple(Poco::Data::Session &dbSession, const std::string &sql) override;
};
}  // namespace sqlite
}  // namespace storage
}  // namespace broker
}  // namespace upmq
#endif  // UPMQ_BINS_BROKER_SESSION_SQLITE_CONNECTIONPOOL_H_
