//
// Created by Alexander Bychuk on 29.06.2020.
//

#ifndef UPMQ_BINS_BROKER_SESSION_ICONNECTIONPOOL_H_
#define UPMQ_BINS_BROKER_SESSION_ICONNECTIONPOOL_H_

#include "ConcurrentQueueHeader.h"
#include <Poco/Data/Session.h>
#include "DBMSSession.h"

namespace upmq {
namespace broker {
namespace storage {
class IConnectionPool {
 protected:
  using SessionsQueueType = moodycamel::ConcurrentQueue<std::shared_ptr<Poco::Data::Session>>;

  int count;
  mutable SessionsQueueType sessions;
  std::string dbmsString;

 public:
  IConnectionPool(int cnt, std::string connectionString) : count(cnt), dbmsString(std::move(connectionString)) {}
  IConnectionPool(const IConnectionPool &) = delete;
  IConnectionPool(IConnectionPool &&) = delete;
  IConnectionPool &operator=(const IConnectionPool &) = delete;
  IConnectionPool &operator=(IConnectionPool &&) = delete;
  virtual ~IConnectionPool() = default;

  virtual std::shared_ptr<Poco::Data::Session> dbmsConnection() const = 0;
  virtual void pushBack(std::shared_ptr<Poco::Data::Session> session) = 0;

  virtual void beginTX(Poco::Data::Session &dbSession, const std::string &txName, storage::DBMSSession::TransactionMode mode) = 0;
  virtual void commitTX(Poco::Data::Session &dbSession, const std::string &txName) = 0;
  virtual void rollbackTX(Poco::Data::Session &dbSession, const std::string &txName) = 0;
};
}  // namespace storage
}  // namespace broker
}  // namespace upmq

#endif  // UPMQ_BINS_BROKER_SESSION_ICONNECTIONPOOL_H_
