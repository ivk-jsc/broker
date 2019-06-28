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

#include "UPMQCPP.h"

#include <decaf/lang/Runtime.h>
#include <transport/TransportRegistry.h>

#include <transport/failover/FailoverTransportFactory.h>
#include <transport/tcp/TcpTransportFactory.h>

using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::tcp;
using namespace upmq::transport::failover;

////////////////////////////////////////////////////////////////////////////////
UPMQCPP::UPMQCPP() {}

////////////////////////////////////////////////////////////////////////////////
UPMQCPP::~UPMQCPP() {}

////////////////////////////////////////////////////////////////////////////////
void UPMQCPP::initializeLibrary(int argc, char **argv) {
  // Initialize the Decaf Library by requesting its runtime.
  decaf::lang::Runtime::initializeRuntime(argc, argv);

  // Register all Transports
  UPMQCPP::registerTransports();
}

////////////////////////////////////////////////////////////////////////////////
void UPMQCPP::initializeLibrary() { UPMQCPP::initializeLibrary(0, nullptr); }

////////////////////////////////////////////////////////////////////////////////
void UPMQCPP::shutdownLibrary() {
  TransportRegistry::shutdown();

  // Now it should be safe to shutdown Decaf.
  decaf::lang::Runtime::shutdownRuntime();
}

////////////////////////////////////////////////////////////////////////////////
void UPMQCPP::registerTransports() {
  // Each of the internally implemented Transports is registered here
  TransportRegistry::initialize();

  TransportRegistry::getInstance().registerFactory("tcp", new TcpTransportFactory());
  TransportRegistry::getInstance().registerFactory("failover", new FailoverTransportFactory());
}
