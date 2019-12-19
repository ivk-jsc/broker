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

#ifndef _UPMQ_CONNECTION_STATE_H_
#define _UPMQ_CONNECTION_STATE_H_

#include <state/SenderState.h>
#include <state/SessionState.h>
#include <state/SubscriptionState.h>
#include <transport/UPMQCommand.h>

#include <decaf/lang/Pointer.h>
#include <decaf/util/LinkedList.h>
#include <decaf/util/StlMap.h>
#include <decaf/util/concurrent/ConcurrentStlMap.h>
#include <decaf/util/concurrent/atomic/AtomicBoolean.h>

namespace upmq {
namespace state {

using decaf::lang::Pointer;
using namespace decaf::util;
using namespace upmq::transport;

class ConnectionState {
 private:
  Pointer<Command> info;
  ConcurrentStlMap<string, Pointer<SessionState>> sessions;
  LinkedList<Pointer<Command>> tempDestinations;
  decaf::util::concurrent::atomic::AtomicBoolean disposed;
  bool connectionInterruptProcessingComplete;

 public:
  ConnectionState(Pointer<Command> info_);
  virtual ~ConnectionState();

  Pointer<Command> getInfo() const;

  void shutdown();
  void checkShutdown() const;
  void reset(Pointer<Command> command);

  void addTempDestination(const Pointer<Command> &command);
  void removeTempDestination(const Pointer<Command> &destination);

  void addSession(Pointer<Command> command);
  Pointer<SessionState> removeSession(const string &id);
  Pointer<SessionState> getSessionState(const Pointer<Command> &id) const;
  const decaf::util::Collection<Pointer<SessionState>> &getSessionStates() const;

  const LinkedList<Pointer<Command>> &getTempDestinations() const;

  void setConnectionInterruptProcessingComplete(bool isConnectionInterruptProcessingComplete);
  bool isConnectionInterruptProcessingComplete() const;
};
}  // namespace state
}  // namespace upmq

#endif /*_UPMQ_CONNECTION_STATE_H_*/
