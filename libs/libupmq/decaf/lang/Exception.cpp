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
#include <decaf/lang/Pointer.h>
#include <decaf/util/logging/LoggerDefines.h>
#include <cstdio>
#include <cstdarg>
#include <sstream>
#include <Poco/Format.h>

using namespace std;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util::logging;

namespace decaf {
namespace lang {

class ExceptionData {
 public:
  /**
   * The cause of this exception.
   */
  std::string message;

  /**
   * The Exception that caused this one to be thrown.
   */
  decaf::lang::Pointer<const std::exception> cause;

  /**
   * The stack trace.
   */
  std::vector<std::pair<std::string, int> > stackTrace;

  ExceptionData() : cause(nullptr) {}

  ExceptionData(const ExceptionData &other) = default;

  ExceptionData(ExceptionData &&other) noexcept
      : message(std::move(other.message)), cause(other.cause.release()), stackTrace(std::move(other.stackTrace)) {}

  ExceptionData &operator=(const ExceptionData &other) = default;
  ExceptionData &operator=(ExceptionData &&other) = default;
};
}  // namespace lang
}  // namespace decaf

////////////////////////////////////////////////////////////////////////////////
Exception::Exception() : data(new ExceptionData) {}

////////////////////////////////////////////////////////////////////////////////
Exception::Exception(const Exception &ex) noexcept : Throwable(ex), data(new ExceptionData(*ex.data)) {}

Exception::Exception(Exception &&ex) noexcept : Throwable(std::forward<Throwable>(ex)), data(ex.releaseData()) {}

////////////////////////////////////////////////////////////////////////////////
Exception::Exception(const std::exception *cause) : Throwable(*cause), data(new ExceptionData) { Exception::initCause(cause); }

////////////////////////////////////////////////////////////////////////////////
Exception::Exception(const char *file, const int lineNumber, const char *msg, ...) : data(new ExceptionData) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);

  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
Exception::Exception(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...) : data(new ExceptionData) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);

  // Store the cause
  Exception::initCause(cause);

  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
Exception::~Exception() noexcept { delete this->data; }

////////////////////////////////////////////////////////////////////////////////
void Exception::setMessage(const char *msg, ...) {
  va_list vargs;
  va_start(vargs, msg);
  buildMessage(msg, vargs);
  va_end(vargs);
}

////////////////////////////////////////////////////////////////////////////////
void Exception::buildMessage(const char *format, va_list &vargs) {
  // Allocate a buffer of the specified size.
  std::vector<char> buffer(vsnprintf(nullptr, 0, format, vargs) + 1);
  vsnprintf(&buffer[0], buffer.size() - 1, format, vargs);
  // Guessed size was enough. Assign the string.
  this->data->message.assign(&buffer[0], buffer.size());
}

ExceptionData *Exception::releaseData() {
  ExceptionData *tmp = data;
  data = nullptr;
  return tmp;
}

////////////////////////////////////////////////////////////////////////////////
void Exception::setMark(const char *file, const int lineNumber) {
  // Add this mark to the end of the stack trace.
  this->data->stackTrace.emplace_back(std::string(file), static_cast<int>(lineNumber));
}

////////////////////////////////////////////////////////////////////////////////
Exception *Exception::clone() const { return new Exception(*this); }

////////////////////////////////////////////////////////////////////////////////
std::vector<std::pair<std::string, int> > Exception::getStackTrace() const { return this->data->stackTrace; }

////////////////////////////////////////////////////////////////////////////////
void Exception::setStackTrace(const std::vector<std::pair<std::string, int> > &trace) { this->data->stackTrace = trace; }

////////////////////////////////////////////////////////////////////////////////
void Exception::printStackTrace() const { printStackTrace(std::cerr); }

////////////////////////////////////////////////////////////////////////////////
void Exception::printStackTrace(std::ostream &stream) const { stream << getStackTraceString(); }

////////////////////////////////////////////////////////////////////////////////
std::string Exception::getStackTraceString() const {
  // Create the output stream.
  std::ostringstream stream;

  // Write the message and each stack entry.
  stream << this->data->message << std::endl;
  for (unsigned int ix = 0; ix < this->data->stackTrace.size(); ++ix) {
    stream << "\tFILE: " << this->data->stackTrace[ix].first;
    stream << ", LINE: " << this->data->stackTrace[ix].second;
    stream << std::endl;
  }

  // Return the string from the output stream.
  return stream.str();
}

////////////////////////////////////////////////////////////////////////////////
Exception &Exception::operator=(const Exception &ex) {
  this->data->message = ex.data->message;
  this->data->stackTrace = ex.data->stackTrace;
  this->data->cause = ex.data->cause;
  return *this;
}

Exception &Exception::operator=(Exception &&ex) noexcept {
  this->data->message = std::move(ex.data->message);
  this->data->stackTrace = std::move(ex.data->stackTrace);
  this->data->cause = std::move(ex.data->cause);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void Exception::initCause(const std::exception *cause) {
  if (cause == nullptr || cause == this) {
    return;
  }

  this->data->cause.reset(cause);

  if (this->data->message.empty()) {
    this->data->message = cause->what();
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string Exception::getMessage() const { return this->data->message; }

////////////////////////////////////////////////////////////////////////////////
const std::exception *Exception::getCause() const { return this->data->cause.get(); }

////////////////////////////////////////////////////////////////////////////////
const char *Exception::what() const noexcept { return this->data->message.c_str(); }
