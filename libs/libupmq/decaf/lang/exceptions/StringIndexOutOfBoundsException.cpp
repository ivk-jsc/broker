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

#include "StringIndexOutOfBoundsException.h"

#include <decaf/lang/Integer.h>

using namespace decaf;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException() = default;

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(int index) { this->Exception::setMessage("String index out of range: %d", index); }

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::~StringIndexOutOfBoundsException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(const Exception &ex) noexcept : IndexOutOfBoundsException(ex) {}

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(const StringIndexOutOfBoundsException &ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(StringIndexOutOfBoundsException &&ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException &StringIndexOutOfBoundsException::operator=(const StringIndexOutOfBoundsException &exc) = default;

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException &StringIndexOutOfBoundsException::operator=(StringIndexOutOfBoundsException &&exc) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(const std::exception *cause) : IndexOutOfBoundsException(cause) {}

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(const char *file, const int lineNumber, const char *msg, ...) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(const char *file, const int lineNumber, int index) {
  Exception::setMark(file, lineNumber);
  this->Exception::setMessage("String index out of range: %d", index);
}

StringIndexOutOfBoundsException *StringIndexOutOfBoundsException::clone() const { return new StringIndexOutOfBoundsException(*this); }

////////////////////////////////////////////////////////////////////////////////
StringIndexOutOfBoundsException::StringIndexOutOfBoundsException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...) : IndexOutOfBoundsException(cause) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}
