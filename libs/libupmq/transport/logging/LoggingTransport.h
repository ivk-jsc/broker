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

#ifndef _UPMQ_TRANSPORT_LOGGING_LOGGINGTRANSPORT_H_
#define _UPMQ_TRANSPORT_LOGGING_LOGGINGTRANSPORT_H_

#include <decaf/lang/Pointer.h>
#include <transport/Config.h>
#include <transport/TransportFilter.h>

namespace upmq {
namespace transport {
namespace logging {

using decaf::lang::Pointer;

/**
 * A transport filter that logs commands as they are sent/received.
 */
class UPMQCPP_API LoggingTransport : public TransportFilter {
 public:
  /**
   * Constructor.
   * @param next - the next Transport in the chain
   */
  LoggingTransport(const Pointer<Transport> next);

  virtual ~LoggingTransport() {}

 public:  // TransportFilter methods.
  virtual void onCommand(const Pointer<Command> command);

 public:  // TransportFilter methods.
  virtual void oneway(const Pointer<Command> command);

  /**
   * {@inheritDoc}
   *
   * Not supported by this class - throws an exception.
   */
  virtual Pointer<Response> request(const Pointer<Command> command);

  /**
   * {@inheritDoc}
   *
   * Not supported by this class - throws an exception.
   */
  virtual Pointer<Response> request(const Pointer<Command> command, unsigned int timeout);
};
}  // namespace logging
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_TRANSPORT_LOGGING_LOGGINGTRANSPORT_H_*/