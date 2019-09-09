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

#ifndef _UPMQ_DEFAULT_TRANSPORT_LISTENER_H_
#define _UPMQ_DEFAULT_TRANSPORT_LISTENER_H_

#include <decaf/lang/Pointer.h>
#include <transport/Config.h>
#include <transport/TransportListener.h>

namespace upmq {
namespace transport {

using decaf::lang::Pointer;

/**
 * A Utility class that create empty implementations for the TransportListener interface
 * so that a subclass only needs to override the one's its interested.
 */
class UPMQCPP_API DefaultTransportListener : public TransportListener {
 public:
  ~DefaultTransportListener() override;

  /**
   * Event handler for the receipt of a command.  The transport passes
   * off all received commands to its listeners, the listener then owns
   * the Object.  If there is no registered listener the Transport deletes
   * the command upon receipt.
   *
   * @param command the received command object.
   */
  void onCommand(Pointer<Command> command) override {}

  /**
   * Event handler for an exception from a command transport.
   *
   * @param ex The exception.
   */
  void onException(const decaf::lang::Exception &) override {}

  /**
   * The transport has suffered an interruption from which it hopes to recover
   */
  void transportInterrupted() override {}

  /**
   * The transport has resumed after an interruption
   */
  void transportResumed() override {}
};
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_DEFAULT_TRANSPORT_LISTENER_H_*/
