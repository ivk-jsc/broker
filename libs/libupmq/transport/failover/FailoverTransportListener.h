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

#ifndef UPMQ_FAILOVER_TRANSPORT_LISTENER_H_
#define UPMQ_FAILOVER_TRANSPORT_LISTENER_H_

#include <decaf/lang/Pointer.h>
#include <transport/Config.h>
#include <transport/TransportListener.h>

namespace upmq {
namespace transport {
namespace failover {

class FailoverTransport;

/**
 * Utility class used by the Transport to perform the work of responding to events
 * from the active Transport.
 *
 * @since 3.0
 */
class UPMQCPP_API FailoverTransportListener : public TransportListener {
 private:
  // The Transport that created this listener
  FailoverTransport *parent;

public:
  FailoverTransportListener(const FailoverTransportListener &) = delete;
  FailoverTransportListener &operator=(const FailoverTransportListener &) = delete;

  FailoverTransportListener(FailoverTransport *parent);

  ~FailoverTransportListener() override;

  /**
   * Event handler for the receipt of a command.  The transport passes
   * off all received commands to its listeners, the listener then owns
   * the Object.  If there is no registered listener the Transport deletes
   * the command upon receipt.
   *
   * @param command the received command object.
   */
  void onCommand(Pointer<Command> command) override;

  /**
   * Event handler for an exception from a command transport.
   *
   * @param ex The exception.
   */
  void onException(const decaf::lang::Exception &ex) override;

  /**
   * The transport has suffered an interruption from which it hopes to recover
   */
  void transportInterrupted() override;

  /**
   * The transport has resumed after an interruption
   */
  void transportResumed() override;
};
}  // namespace failover
}  // namespace transport
}  // namespace upmq

#endif /* UPMQ_FAILOVER_TRANSPORT_LISTENER_H_ */
