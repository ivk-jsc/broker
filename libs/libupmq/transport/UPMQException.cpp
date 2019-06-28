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

#include "UPMQException.h"
#include <cstdio>

using namespace upmq;
using namespace upmq::transport;
using namespace decaf::lang;
using namespace std;

// For supporting older versions of msvc (<=2003)
#if defined(_MSC_VER) && (_MSC_VER < 1400)
#define vsnprintf _vsnprintf
#endif

////////////////////////////////////////////////////////////////////////////////
UPMQException::UPMQException() : decaf::lang::Exception() {}

////////////////////////////////////////////////////////////////////////////////
UPMQException::UPMQException(const UPMQException &ex) : decaf::lang::Exception(ex) {}

////////////////////////////////////////////////////////////////////////////////
UPMQException::UPMQException(const Exception &ex) : decaf::lang::Exception(ex.clone()) {}

////////////////////////////////////////////////////////////////////////////////
UPMQException::UPMQException(const char *file, const int lineNumber, const char *msg, ...) : decaf::lang::Exception() {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
UPMQException::UPMQException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...) : decaf::lang::Exception(cause) {
  va_list vargs;
  va_start(vargs, msg);
  Exception::buildMessage(msg, vargs);
  va_end(vargs);
  // Set the first mark for this exception.
  Exception::setMark(file, lineNumber);
}

////////////////////////////////////////////////////////////////////////////////
UPMQException::~UPMQException() throw() {}

////////////////////////////////////////////////////////////////////////////////
UPMQException *UPMQException::clone() const { return new UPMQException(*this); }

////////////////////////////////////////////////////////////////////////////////
cms::CMSException UPMQException::convertToCMSException() const {
  std::exception *result = nullptr;

  if (this->getCause() != nullptr) {
    const Exception *ptrCause = dynamic_cast<const Exception *>(this->getCause());
    if (ptrCause == nullptr) {
      result = new Exception(__FILE__, __LINE__, getCause()->what());
    } else {
      result = ptrCause->clone();
    }
  }

  return cms::CMSException(this->getMessage(), result, this->getStackTrace());
}
