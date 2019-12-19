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

#include "DBMSSession.h"
#include <Exception.h>
#include "DBMSConnectionPool.h"
upmq::broker::storage::DBMSSession::DBMSSession(std::shared_ptr<Poco::Data::Session> &&session, upmq::broker::storage::DBMSConnectionPool &dbmsPool)
    : _session(std::move(session)), _dbmsPool(dbmsPool) {}
upmq::broker::storage::DBMSSession::~DBMSSession() {
  try {
    close();
  } catch (...) {
  }
}
void upmq::broker::storage::DBMSSession::beginTX(const std::string &txName, TransactionMode mode) {
  if (!_session) {
    throw EXCEPTION("dbms session was closed", txName, ERROR_WORKER);
  }
  _lastTXName = txName;
  if (_lastTXName.find_first_of('\"') == 0) {
    Poco::removeInPlace(_lastTXName, '\"');
  }
  dbms::Instance().beginTX(*_session, _lastTXName, mode);
  _inTransaction = true;
}
void upmq::broker::storage::DBMSSession::commitTX() {
  if (!_session) {
    throw EXCEPTION("dbms session was closed", _lastTXName, ERROR_WORKER);
  }
  if (_inTransaction) {
    DBMSConnectionPool::commitTX(*_session, _lastTXName);
  }
  _inTransaction = false;
}
void upmq::broker::storage::DBMSSession::rollbackTX() {
  if (!_session) {
    throw EXCEPTION("dbms session was closed", _lastTXName, ERROR_WORKER);
  }
  if (_inTransaction) {
    DBMSConnectionPool::rollbackTX(*_session, _lastTXName);
  }
  _inTransaction = false;
}
void upmq::broker::storage::DBMSSession::close() {
  if (_session) {
    if (_inTransaction) {
      DBMSConnectionPool::rollbackTX(*_session, _lastTXName);
    }
    _dbmsPool.pushBack(_session);
  }
}
Poco::Data::Session &upmq::broker::storage::DBMSSession::operator()() const {
  if (!_session) {
    throw EXCEPTION("dbms session was closed", _lastTXName, ERROR_WORKER);
  }
  return *_session;
}
bool upmq::broker::storage::DBMSSession::isValid() const { return _session != nullptr; }
const std::shared_ptr<Poco::Data::Session> &upmq::broker::storage::DBMSSession::dbmsConnnectionRef() const {
  if (!_session) {
    throw EXCEPTION("dbms session was closed", _lastTXName, ERROR_WORKER);
  }
  return _session;
}
bool upmq::broker::storage::DBMSSession::inTransaction() const { return _inTransaction; }
