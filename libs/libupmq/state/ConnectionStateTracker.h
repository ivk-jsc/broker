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

#ifndef _UPMQ_CONNECTION_STATE_TRACKER_H_
#define _UPMQ_CONNECTION_STATE_TRACKER_H_

#include <state/ConnectionState.h>
#include <state/SessionState.h>
#include <state/Tracked.h>
#include <transport/Transport.h>

#include <decaf/lang/Pointer.h>

namespace upmq {
namespace state {

using namespace std;

class StateTrackerImpl;

class ConnectionStateTracker {
 private:
  StateTrackerImpl *impl;

  bool restoreSessions;
  bool restoreConsumers;
  bool restoreProducers;

 public:
  ConnectionStateTracker();
  virtual ~ConnectionStateTracker();

  Pointer<Tracked> track(const Pointer<Command> &command);
  void restore(const Pointer<transport::Transport> &transport);
  void transportInterrupted();

  virtual decaf::lang::Pointer<Command> processConnect(const Pointer<Command> &info);
  virtual decaf::lang::Pointer<Command> processDisconnect(const Pointer<Command> &info);

  virtual decaf::lang::Pointer<Command> processSession(const Pointer<Command> &info);
  virtual decaf::lang::Pointer<Command> processUnsession(const Pointer<Command> &info);

  virtual decaf::lang::Pointer<Command> processSender(const Pointer<Command> &info);
  virtual decaf::lang::Pointer<Command> processUnsender(const Pointer<Command> &info);

  virtual decaf::lang::Pointer<Command> processSubscription(const Pointer<Command> &info);
  virtual decaf::lang::Pointer<Command> processUnsubscription(const Pointer<Command> &info);

  virtual decaf::lang::Pointer<Command> processSubscribe(const Pointer<Command> &info);
  virtual decaf::lang::Pointer<Command> processUnsubscribe(const Pointer<Command> &info);

  void setRestoreSessions(bool isRestoreSessions);
  void setRestoreConsumers(bool isRestoreConsumers);
  void setRestoreProducers(bool isRestoreProducers);

  bool isRestoreSessions() const;
  bool isRestoreConsumers() const;
  bool isRestoreProducers() const;

 private:
  void doRestoreSessions(const decaf::lang::Pointer<transport::Transport> &transport, const decaf::lang::Pointer<ConnectionState> &connectionState);
  void doRestoreConsumers(const decaf::lang::Pointer<transport::Transport> &transport, const decaf::lang::Pointer<SessionState> &sessionState);
  void doRestoreProducers(const decaf::lang::Pointer<transport::Transport> &transport, const decaf::lang::Pointer<SessionState> &sessionState);
  void doRestoreTempDestinations(const decaf::lang::Pointer<transport::Transport> &transport,
                                 const decaf::lang::Pointer<ConnectionState> &connectionState);
};
}  // namespace state
}  // namespace upmq

#endif /*_UPMQ_CONNECTION_STATE_TRACKER_H_*/
