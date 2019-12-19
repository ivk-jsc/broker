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

#include "FutureResponse.h"

#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/concurrent/Concurrent.h>
#include <transport/Config.h>
#include <transport/UPMQException.h>
#include <typeinfo>
#include <utility>

using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::io;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::util::concurrent;

////////////////////////////////////////////////////////////////////////////////
FutureResponse::FutureResponse() : responseLatch(1), response(), responseCallback() {}

////////////////////////////////////////////////////////////////////////////////
FutureResponse::FutureResponse(Pointer<ResponseCallback> responseCallback_)
    : responseLatch(1), response(), responseCallback(std::move(responseCallback_)) {}

////////////////////////////////////////////////////////////////////////////////
FutureResponse::~FutureResponse() {}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> FutureResponse::getResponse() const {
  try {
    this->responseLatch.await();
    return response;
  } catch (decaf::lang::exceptions::InterruptedException &) {
    decaf::lang::Thread::currentThread()->interrupt();
    throw decaf::io::InterruptedIOException(__FILE__, __LINE__, "Interrupted while awaiting a response");
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> FutureResponse::getResponse() {
  try {
    this->responseLatch.await();
    return response;
  } catch (decaf::lang::exceptions::InterruptedException &) {
    decaf::lang::Thread::currentThread()->interrupt();
    throw decaf::io::InterruptedIOException(__FILE__, __LINE__, "Interrupted while awaiting a response");
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> FutureResponse::getResponse(unsigned int timeout) const {
  try {
    this->responseLatch.await(timeout);
    return response;
  } catch (decaf::lang::exceptions::InterruptedException &) {
    throw decaf::io::InterruptedIOException(__FILE__, __LINE__, "Interrupted while awaiting a response");
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> FutureResponse::getResponse(unsigned int timeout) {
  try {
    this->responseLatch.await(timeout);
    return response;
  } catch (decaf::lang::exceptions::InterruptedException &) {
    throw decaf::io::InterruptedIOException(__FILE__, __LINE__, "Interrupted while awaiting a response");
  }
}

////////////////////////////////////////////////////////////////////////////////
void FutureResponse::setResponse(Pointer<Response> newResponse) {
  this->response = std::move(newResponse);
  this->responseLatch.countDown();
  if (responseCallback != nullptr) {
    responseCallback->onComplete(this->response);
  }
}
