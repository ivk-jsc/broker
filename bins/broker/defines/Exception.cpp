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

#include "Exception.h"
#include <Poco/Thread.h>
#include "AsyncLogger.h"
#include "Configuration.h"

#ifdef _DEBUG
#define LOG_LOCKS(x)                                                    \
  auto* log = &Poco::Logger::get(CONFIGURATION::Instance().log().name); \
  log->warning(#x " file : %s, line : %d, sql : %s ", file, line, onError.sql())
#else
#define LOG_LOCKS(x)
#endif

namespace upmq {
namespace broker {

Exception::Exception(const std::string& info, const std::string& errDescription, int err, const std::string& file, int line)
    : _message(std::string("exception : ")
                   .append(file)
                   .append(" : ")
                   .append(std::to_string(line))
                   .append(" => ")
                   .append(errDescription)
                   .append("(")
                   .append(std::to_string(err))
                   .append(")")
                   .append(" : ")
                   .append(info)),
      _error(err) {}

Exception::Exception(const Exception&) = default;

Exception::Exception(Exception&&) noexcept = default;

Exception& Exception::operator=(const Exception&) = default;

Exception& Exception::operator=(Exception&&) noexcept = default;

const char* Exception::what() const noexcept { return _message.c_str(); }

std::string Exception::message() const { return _message; }

int Exception::error() const { return _error; }

Exception::~Exception() noexcept = default;
// ----------------------------------------
OnError& OnError::setInfo(std::string info) {
  _info = std::move(info);
  return *this;
}
OnError& OnError::setError(int error) {
  _error = error;
  return *this;
}
OnError& OnError::setSql(std::string sql) {
  _sql = std::move(sql);
  return *this;
}
OnError& OnError::setErrorDescription(std::string description) {
  _errorDescription = std::move(description);
  return *this;
}
OnError& OnError::setExpression(const std::function<void()>& expression) {
  _expression.assign(expression);
  return *this;
}
OnError& OnError::setLine(int line) {
  _line = line;
  return *this;
}
OnError& OnError::setFile(std::string file) {
  _file = std::move(file);
  return *this;
}
void OnError::make(OnError::Mode mode) const {
  if (!_expression.isNull()) {
    (_expression.value())();
  }
  switch (mode) {
    case WITH_THROW:
      throw upmq::broker::Exception(_info + " : " + _sql,
                                    std::string(_errorDescription).append(" native(").append(std::to_string(Poco::Error::last())).append(")"),
                                    _error,
                                    _file,
                                    _line);
      break;
    case NO_THROW:
      ASYNCLOGGER::Instance().get(LOG_CONFIG.name).error("- ! => %s : %s : %s : %d", _info, _sql, _errorDescription, _error);
      break;
  }
}
const std::string& OnError::info() const { return _info; }
const std::string& OnError::sql() const { return _sql; }
const std::string& OnError::errorDescription() const { return _errorDescription; }
int OnError::error() const { return _error; }
const Poco::Nullable<std::function<void()>>& OnError::expression() const { return _expression; }
const std::string& OnError::file() const { return _file; }
int OnError::line() const { return _line; }

void tryExecute(const std::function<void()>& call, OnError& onError, std::string file, int line, OnError::Mode mode) {
  bool locked;
  do {
    locked = false;
    try {
      call();
    } catch (PDSQLITE::DBLockedException& dblex) {
      UNUSED_VAR(dblex);
      locked = true;
      LOG_LOCKS(dbLocked);
      Poco::Thread::yield();
    } catch (PDSQLITE::TableLockedException& tblex) {
      UNUSED_VAR(tblex);
      locked = true;
      LOG_LOCKS(tableLocked);
      Poco::Thread::yield();
    } catch (Poco::InvalidAccessException& invaccex) {
      UNUSED_VAR(invaccex);
      locked = true;
      LOG_LOCKS(invalidAccessLocked);
      Poco::Thread::yield();
    } catch (Poco::Exception& pex) {
      onError.setErrorDescription(pex.message()).setFile(std::move(file)).setLine(line);
      onError.make(mode);
    } catch (const std::exception& e) {
      onError.setErrorDescription(e.what()).setFile(std::move(file)).setLine(line);
      onError.make(mode);
    } catch (...) {
      onError.setErrorDescription("unknown exception").setFile(std::move(file)).setLine(line);
      onError.make(mode);
    }
  } while (locked);
}
}  // namespace broker
}  // namespace upmq
