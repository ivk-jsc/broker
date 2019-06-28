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

#include "UTFDataFormatException.h"

using namespace decaf;
using namespace decaf::io;

////////////////////////////////////////////////////////////////////////////////
UTFDataFormatException::UTFDataFormatException() = default;

////////////////////////////////////////////////////////////////////////////////
UTFDataFormatException::~UTFDataFormatException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
UTFDataFormatException::UTFDataFormatException(const Exception &ex) noexcept : io::IOException(ex) {}

////////////////////////////////////////////////////////////////////////////////
UTFDataFormatException::UTFDataFormatException(const UTFDataFormatException &ex) noexcept = default;

UTFDataFormatException::UTFDataFormatException(UTFDataFormatException &&ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
UTFDataFormatException::UTFDataFormatException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...) : io::IOException(cause) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
UTFDataFormatException::UTFDataFormatException(const std::exception *cause) : io::IOException(cause) {}

////////////////////////////////////////////////////////////////////////////////
UTFDataFormatException::UTFDataFormatException(const char *file, const int lineNumber, const char *msg, ...) : io::IOException() {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

UTFDataFormatException *UTFDataFormatException::clone() const { return new UTFDataFormatException(*this); }
