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

#include "FailoverTransportListener.h"
#include "FailoverTransport.h"

#include <decaf/lang/exceptions/NullPointerException.h>
#include <transport/Response.h>

using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::failover;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
FailoverTransportListener::FailoverTransportListener(FailoverTransport *parent) : parent(parent) {
  if (this->parent == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Pointer to Parent Transport was NULL");
  }
}

////////////////////////////////////////////////////////////////////////////////
FailoverTransportListener::~FailoverTransportListener() {}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransportListener::onCommand(const Pointer<Command> command) {
  if (command == nullptr) {
    return;
  }

  if (command->isResponse()) {
    Pointer<Response> response = command.dynamicCast<Response>();
    parent->processResponse(response);
  }

  if (!parent->isInitialized()) {
    parent->setInitialized(true);
  }

  if (command->isDisconnect()) {
    parent->handleConnectionControl(command);
  }

  if (parent->getTransportListener() != nullptr) {
    parent->getTransportListener()->onCommand(command);
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransportListener::onException(const decaf::lang::Exception &ex) {
  try {
    parent->handleTransportFailure(ex);
  } catch (Exception &e) {
    if (parent->getTransportListener() != nullptr) {
      parent->getTransportListener()->onException(e);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransportListener::transportInterrupted() {
  if (parent->getTransportListener() != nullptr) {
    parent->getTransportListener()->transportInterrupted();
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransportListener::transportResumed() {
  if (parent->getTransportListener() != nullptr) {
    parent->getTransportListener()->transportResumed();
  }
}
