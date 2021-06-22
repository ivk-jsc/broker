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

#ifndef UPMQ_BINS_BROKER_SESSION_POSTGRESQL_CONNECTIONPOOL_H_
#define UPMQ_BINS_BROKER_SESSION_POSTGRESQL_CONNECTIONPOOL_H_

#include "IConnectionPool.h"

namespace upmq {
namespace broker {
namespace storage {
namespace postgresql {

class ConnectionPool : public IConnectionPool {
  std::shared_ptr<Poco::Data::Session> makeSession() const;
  void runTXCommand(Poco::Data::Session &dbSession, const char *command);

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
}  // namespace postgresql
}  // namespace storage
}  // namespace broker
}  // namespace upmq

#endif  // UPMQ_BINS_BROKER_SESSION_POSTGRESQL_CONNECTIONPOOL_H_
