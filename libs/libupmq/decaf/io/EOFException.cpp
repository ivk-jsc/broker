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

#include <cstdarg>
#include "EOFException.h"

using namespace decaf;
using namespace decaf::io;

////////////////////////////////////////////////////////////////////////////////
EOFException::EOFException() = default;

////////////////////////////////////////////////////////////////////////////////
EOFException::~EOFException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
EOFException::EOFException(const decaf::lang::Exception &ex) noexcept : io::IOException(ex) {}

////////////////////////////////////////////////////////////////////////////////
EOFException::EOFException(const EOFException &ex) noexcept = default;

EOFException::EOFException(EOFException &&ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
EOFException::EOFException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...) : io::IOException(cause) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
EOFException::EOFException(const std::exception *cause) : io::IOException(cause) {}

////////////////////////////////////////////////////////////////////////////////
EOFException::EOFException(const char *file, const int lineNumber, const char *msg, ...) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

EOFException *EOFException::clone() const { return new EOFException(*this); }
