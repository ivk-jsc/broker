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

#include "DecafRuntime.h"

#include <decaf/internal/net/Network.h>
#include <decaf/internal/security/SecurityRuntime.h>
#include <decaf/internal/util/concurrent/Threading.h>
#include <decaf/lang/System.h>
#include <decaf/lang/Thread.h>

using namespace decaf;
using namespace decaf::internal;
using namespace decaf::internal::net;
using namespace decaf::internal::security;
using namespace decaf::internal::util::concurrent;
using namespace decaf::lang;
using namespace decaf::util::concurrent;

////////////////////////////////////////////////////////////////////////////////
namespace {
Mutex *globalLock;
}  // namespace

////////////////////////////////////////////////////////////////////////////////
DecafRuntime::DecafRuntime() {}

////////////////////////////////////////////////////////////////////////////////
DecafRuntime::~DecafRuntime() = default;

////////////////////////////////////////////////////////////////////////////////
Mutex *DecafRuntime::getGlobalLock() { return globalLock; }

////////////////////////////////////////////////////////////////////////////////
Runtime *Runtime::getRuntime() {
  static DecafRuntime runtime;
  return &runtime;
}

////////////////////////////////////////////////////////////////////////////////
void Runtime::initializeRuntime(int argc, char **argv) {
  Runtime::getRuntime();
  Threading::initialize();

  globalLock = new Mutex;

  System::initSystem(argc, argv);
  Network::initializeNetworking();
  SecurityRuntime::initializeSecurity();
}

////////////////////////////////////////////////////////////////////////////////
void Runtime::initializeRuntime() { Runtime::initializeRuntime(0, nullptr); }

////////////////////////////////////////////////////////////////////////////////
void Runtime::shutdownRuntime() {
  SecurityRuntime::shutdownSecurity();

  // Shutdown the networking layer before Threading, many network routines need
  // to be thread safe and require Threading primitives.
  Network::shutdownNetworking();

  System::shutdownSystem();

  // This must go away before Threading is shutdown.
  delete globalLock;

  // Threading is the last to by shutdown since most other parts of the Runtime
  // need to make use of Thread primitives.
  Threading::shutdown();
}
