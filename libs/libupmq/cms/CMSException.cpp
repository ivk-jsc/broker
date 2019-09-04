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

#include <cms/CMSException.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

using namespace std;
using namespace cms;

namespace cms {

class CMSExceptionData {
 public:
  std::string message;
  std::unique_ptr<const std::exception> cause;
  std::vector<triplet<std::string, std::string, int>> stackTrace;

  CMSExceptionData() = default;
};
}  // namespace cms

////////////////////////////////////////////////////////////////////////////////
CMSException::CMSException() : data(new CMSExceptionData()) {}

////////////////////////////////////////////////////////////////////////////////
CMSException::CMSException(const CMSException &ex) noexcept : std::exception(ex), data(new CMSExceptionData()) {
  this->data->cause = std::move(ex.data->cause);
  this->data->message = ex.data->message;
  this->data->stackTrace = ex.data->stackTrace;
}

CMSException::CMSException(CMSException &&ex) noexcept : std::exception(ex), data(new CMSExceptionData()) { std::swap(data, ex.data); }

////////////////////////////////////////////////////////////////////////////////
CMSException::CMSException(const std::string &message) : data(new CMSExceptionData()) { this->data->message = message; }

////////////////////////////////////////////////////////////////////////////////
CMSException::CMSException(const std::string &message, const std::exception *cause) : std::exception(), data(new CMSExceptionData()) {
  this->data->cause.reset(cause);
  this->data->message = message;
}

////////////////////////////////////////////////////////////////////////////////
CMSException::CMSException(const std::string &message,
                           const std::exception *cause,
                           const std::vector<triplet<std::string, std::string, int>> &stackTrace)
    : data(new CMSExceptionData()) {
  this->data->cause.reset(cause);
  this->data->message = message;
  this->data->stackTrace = stackTrace;
}

CMSException::CMSException(const std::string &message, const std::exception *cause, const std::vector<std::pair<std::string, int>> &stackTrace)
    : data(new CMSExceptionData()) {
  this->data->cause.reset(cause);
  this->data->message = message;

  for (auto const &value : stackTrace) {
    this->data->stackTrace.push_back(make_triplet(std::string("undefined function"), std::string(value.first), value.second));
  }
}

////////////////////////////////////////////////////////////////////////////////
CMSException::~CMSException() noexcept { delete this->data; }

////////////////////////////////////////////////////////////////////////////////
CMSException *CMSException::clone() { return new CMSException(*this); }

////////////////////////////////////////////////////////////////////////////////
void CMSException::setMark(const char *func, const char *file, const int lineNumber) {
  // Add this mark to the end of the stack trace.
  this->data->stackTrace.push_back(make_triplet(std::string(func), std::string(file), lineNumber));
}

////////////////////////////////////////////////////////////////////////////////
void CMSException::printStackTrace() const { printStackTrace(std::cerr); }

////////////////////////////////////////////////////////////////////////////////
void CMSException::printStackTrace(std::ostream &stream) const { stream << getStackTraceString(); }

////////////////////////////////////////////////////////////////////////////////
std::string CMSException::getStackTraceString() const {
  // Create the output stream.
  std::ostringstream stream;

  // Write the message and each stack entry.
  stream << "\n##### EXCEPTION #####" << std::endl;
  stream << this->data->message << std::endl;
  for (size_t ix = 0; ix < this->data->stackTrace.size(); ++ix) {
    stream << "\tat " << this->data->stackTrace[ix].first << "(" << this->data->stackTrace[ix].second << ":" << this->data->stackTrace[ix].third
           << ")" << std::endl;
  }

  // Return the string from the output stream.
  return stream.str();
}

////////////////////////////////////////////////////////////////////////////////
std::string CMSException::getMessage() const { return this->data->message; }

////////////////////////////////////////////////////////////////////////////////
const std::exception *CMSException::getCause() const { return this->data->cause.get(); }

////////////////////////////////////////////////////////////////////////////////
std::vector<triplet<std::string, std::string, int>> CMSException::getStackTrace() const { return this->data->stackTrace; }

////////////////////////////////////////////////////////////////////////////////
const char *CMSException::what() const noexcept { return this->data->message.c_str(); }
