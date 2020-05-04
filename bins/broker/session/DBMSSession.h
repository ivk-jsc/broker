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

#ifndef BROKER_DBMSSESSION_H
#define BROKER_DBMSSESSION_H
#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/Session.h>
namespace Poco {
namespace Data = SQL;
}
#else
#include <Poco/Data/Session.h>
#endif
#include <memory>
#include <atomic>

namespace upmq {
namespace broker {
namespace storage {

class DBMSConnectionPool;

class DBMSSession {
  std::shared_ptr<Poco::Data::Session> _session;
  DBMSConnectionPool &_dbmsPool;
  std::string _lastTXName = "";
  bool _inTransaction = false;

 public:
  enum class TransactionMode { READ = 0, WRITE };
  DBMSSession(std::shared_ptr<Poco::Data::Session> &&session, DBMSConnectionPool &dbmsPool);
  DBMSSession(DBMSSession &&) = default;
  DBMSSession(const DBMSSession &) = delete;
  DBMSSession &operator=(const DBMSSession &) = delete;
  virtual ~DBMSSession();
  void beginTX(const std::string &txName, TransactionMode mode = TransactionMode::WRITE);
  void commitTX();
  void rollbackTX();
  template <typename T>
  Poco::Data::Statement operator<<(const T &t) {
    return *_session << t;
  }
  Poco::Data::Session &operator()() const;
  void close();
  bool isValid() const;
  const std::shared_ptr<Poco::Data::Session> &dbmsConnnectionRef() const;
  bool inTransaction() const;
  friend bool operator==(const DBMSSession &lhs, const DBMSSession &rhs) { return lhs._lastTXName == rhs._lastTXName; }
};
}  // namespace storage
}  // namespace broker
}  // namespace upmq
#endif  // BROKER_DBMSSESSION_H
