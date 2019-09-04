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

#include "CloseTransportsTask.h"

#include <transport/UPMQException.h>

using namespace upmq;
using namespace upmq::threads;
using namespace upmq::transport;
using namespace upmq::transport::failover;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;

////////////////////////////////////////////////////////////////////////////////
CloseTransportsTask::CloseTransportsTask() : transports() {}

////////////////////////////////////////////////////////////////////////////////
CloseTransportsTask::~CloseTransportsTask() {}

////////////////////////////////////////////////////////////////////////////////
void CloseTransportsTask::add(const Pointer<Transport> transport) { transports.put(transport); }

////////////////////////////////////////////////////////////////////////////////
bool CloseTransportsTask::isPending() const {
  bool result = false;
  result = !transports.isEmpty();
  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool CloseTransportsTask::iterate() {
  while (!transports.isEmpty()) {
    Pointer<Transport> transport = transports.take();

    try {
      transport->close();
    }
    AMQ_CATCHALL_NOTHROW()

    transport.reset(nullptr);
  }

  return false;
}
