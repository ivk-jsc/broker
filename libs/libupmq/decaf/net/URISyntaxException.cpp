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

#include "URISyntaxException.h"

using namespace decaf;
using namespace decaf::net;
using namespace decaf::lang;

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException() = default;

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::~URISyntaxException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(const Exception &ex) noexcept : Exception(ex) {}

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(const URISyntaxException &ex) noexcept : Exception(ex) {}

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(URISyntaxException &&ex) noexcept = default;

URISyntaxException &URISyntaxException::operator=(const URISyntaxException &ex) = default;

URISyntaxException &URISyntaxException::operator=(URISyntaxException &&ex) noexcept {
  reason = std::move(ex.reason);
  input = std::move(ex.input);
  index = ex.index;
  Exception::operator=(ex);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...)
    : lang::Exception(cause), reason(), input(), index(-1) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(const std::exception *cause) : lang::Exception(cause) {}

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(const char *file, const int lineNumber, const char *msg DECAF_UNUSED)
    : reason("<Unknown Reason>"), input("<No Address Given>") {
  DECAF_UNUSED_VAR(msg);
  const char *message = "Input: %s, Reason it failed: %s";
  this->Exception::setMessage(message, input.c_str(), reason.c_str());

  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(const char *file, const int lineNumber, const std::string &input, const std::string &reason)
    : reason(reason), input(input) {
  const char *message = "Input: %s, Reason it failed: %s";
  this->Exception::setMessage(message, input.c_str(), reason.c_str());

  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
URISyntaxException::URISyntaxException(const char *file, const int lineNumber, const std::string &input, const std::string &reason, int index)
    : reason(reason), input(input), index(index) {
  const char *message = "Input: %s, Index %d resulted in this error: %s";
  this->Exception::setMessage(message, input.c_str(), index, reason.c_str());

  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

URISyntaxException *URISyntaxException::clone() const { return new URISyntaxException(*this); }

std::string URISyntaxException::getInput() const { return input; }

std::string URISyntaxException::getReason() const { return reason; }

int URISyntaxException::getIndex() const { return index; }
