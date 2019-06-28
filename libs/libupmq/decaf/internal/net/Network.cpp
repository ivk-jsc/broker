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

#include "Network.h"

#include <decaf/internal/util/ResourceLifecycleManager.h>
#include <decaf/lang/Exception.h>
#include <decaf/lang/Runnable.h>
#include <decaf/lang/exceptions/IllegalStateException.h>
#include <decaf/util/LinkedList.h>
#include <decaf/util/concurrent/Mutex.h>

#ifndef WIN32
#include <signal.h>
#endif

using namespace decaf;
using namespace decaf::internal;
using namespace decaf::internal::net;
using namespace decaf::internal::util;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::util;
using namespace decaf::util::concurrent;

////////////////////////////////////////////////////////////////////////////////
Network *Network::networkRuntime = nullptr;

////////////////////////////////////////////////////////////////////////////////
namespace decaf {
namespace internal {
namespace net {

class NetworkData {
 public:
  ResourceLifecycleManager resources;
  Mutex lock;
  LinkedList<Runnable *> shutdownTasks;
  NetworkData(const NetworkData &) = delete;
  NetworkData &operator=(const NetworkData &) = delete;

  NetworkData() = default;

  ~NetworkData() {
    try {
      std::unique_ptr<Iterator<Runnable *> > iter(shutdownTasks.iterator());
      while (iter->hasNext()) {
        Runnable *task = iter->next();
        try {
          task->run();
          delete task;
        } catch (...) {
        }
      }
    } catch (...) {
    }
  }
};
}  // namespace net
}  // namespace internal
}  // namespace decaf

////////////////////////////////////////////////////////////////////////////////
Network::Network() : data(new NetworkData()) {}

////////////////////////////////////////////////////////////////////////////////
Network::~Network() {
  try {
    delete this->data;
  }
  DECAF_CATCH_NOTHROW(Exception)
  DECAF_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void Network::addNetworkResource(Resource *value) { this->data->resources.addResource(value); }

////////////////////////////////////////////////////////////////////////////////
Network *Network::getNetworkRuntime() {
  if (Network::networkRuntime == nullptr) {
    throw IllegalStateException(__FILE__, __LINE__, "Network Runtime is not Initialized.");
  }

  return Network::networkRuntime;
}

////////////////////////////////////////////////////////////////////////////////
Mutex *Network::getRuntimeLock() const { return &(this->data->lock); }

////////////////////////////////////////////////////////////////////////////////
void Network::initializeNetworking() {
#ifndef WIN32
  // Remove the SIGPIPE so that the application isn't aborted if a connected
  // socket breaks during a read or write.
  signal(SIGPIPE, SIG_IGN);
#endif

  Network::networkRuntime = new Network();
}

////////////////////////////////////////////////////////////////////////////////
void Network::shutdownNetworking() { delete Network::networkRuntime; }

////////////////////////////////////////////////////////////////////////////////
void Network::addShutdownTask(decaf::lang::Runnable *task) {
  if (task != nullptr) {
    this->data->shutdownTasks.add(task);
  }
}
