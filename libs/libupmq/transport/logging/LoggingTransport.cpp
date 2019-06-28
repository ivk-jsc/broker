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

#include "LoggingTransport.h"
#include <iomanip>

using namespace std;
using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::logging;
using namespace decaf::io;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
LoggingTransport::LoggingTransport(const Pointer<Transport> next) : TransportFilter(next) {}

////////////////////////////////////////////////////////////////////////////////
void LoggingTransport::onCommand(const Pointer<Command> command) {
  char buff[20] = {0};
  auto now = time(nullptr);
  strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
  std::cout << buff << " |RECV: " << command->toString() << std::endl;

  // Delegate to the base class.
  TransportFilter::onCommand(command);
}

////////////////////////////////////////////////////////////////////////////////
void LoggingTransport::oneway(const Pointer<Command> command) {
  try {
    char buff[20] = {0};
    auto now = time(nullptr);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    std::cout << buff << " |SEND: " << command->toString() << std::endl;

    // Delegate to the base class.
    TransportFilter::oneway(command);
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_RETHROW(UnsupportedOperationException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> LoggingTransport::request(const Pointer<Command> command) {
  try {
    char buff[20] = {0};
    auto now = time(nullptr);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    std::cout << buff << " |SEND: " << command->toString() << std::endl;

    // Delegate to the base class.
    Pointer<Response> response = TransportFilter::request(command);

    return response;
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_RETHROW(UnsupportedOperationException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> LoggingTransport::request(const Pointer<Command> command, unsigned int timeout) {
  try {
    char buff[20] = {0};
    auto now = time(nullptr);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    std::cout << buff << " |SEND: " << command->toString() << std::endl;

    // Delegate to the base class.
    Pointer<Response> response = TransportFilter::request(command, timeout);

    return response;
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_RETHROW(UnsupportedOperationException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}
