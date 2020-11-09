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

#ifndef __UPMQ_EXCEPTION_H__
#define __UPMQ_EXCEPTION_H__

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <functional>
#include <Poco/Error.h>
#include <Poco/Nullable.h>
#include <fake_cpp14.h>
#include <unordered_map>
#include "Defines.h"

#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/SQLite/SQLiteException.h>
namespace PDSQLITE = Poco::SQL::SQLite;
#else
#include <Poco/Data/SQLite/SQLiteException.h>
namespace PDSQLITE = Poco::Data::SQLite;
#endif

namespace upmq {
namespace broker {

class Exception : public std::exception {
 public:
  ///
  /// \param info - exception information
  /// \param errDescription - error description, for example, strerror(errno)
  /// \param err - error number, for example, errno
  /// \param file - file name, use __FILE__ macro
  /// \param line - line number, use __LINE__ macro
  Exception(const std::string &info, const std::string &errDescription, int err, const std::string &file, int line);
  Exception(const Exception &);
  Exception(Exception &&) noexcept;
  Exception &operator=(const Exception &);
  Exception &operator=(Exception &&) noexcept;
  const char *what() const noexcept override;
  std::string message() const;
  int error() const;
  ~Exception() noexcept override;

 private:
  std::string _message{};
  int _error{0};
};

#define EXCEPTION(_info, _errDescription, _err) \
  upmq::broker::Exception(                      \
      _info, std::string(_errDescription).append(" native(").append(std::to_string(Poco::Error::last())).append(")"), _err, __FILE__, __LINE__)

class OnError {
  std::string _info;
  std::string _sql;
  std::string _errorDescription;
  int _error{0};
  Poco::Nullable<std::function<void()>> _expression{};
  std::string _file;
  int _line{0};

 public:
  enum Mode { WITH_THROW, NO_THROW };
  OnError() = default;
  OnError &setInfo(std::string info);
  OnError &setError(int error);
  OnError &setSql(std::string sql);
  OnError &setErrorDescription(std::string description);
  OnError &setExpression(const std::function<void()> &expression);
  OnError &setLine(int line);
  OnError &setFile(std::string file);

  void make(OnError::Mode mode) const;
};

void tryExecute(const std::function<void()> &call, OnError &onError, std::string file, int line, OnError::Mode mode = OnError::WITH_THROW);
}  // namespace broker
}  // namespace upmq

#define TRY_EXECUTE(__call__, __onError__) upmq::broker::tryExecute((__call__), (__onError__), __FILE__, __LINE__)
#define TRY_EXECUTE_NOEXCEPT(__call__, __onError__) upmq::broker::tryExecute((__call__), (__onError__), __FILE__, __LINE__, OnError::NO_THROW)

#endif  // __UPMQ_EXCEPTION_H__
