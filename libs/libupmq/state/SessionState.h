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

#ifndef _UPMQ_SESSION_STATE_H_
#define _UPMQ_SESSION_STATE_H_

#include <decaf/lang/Pointer.h>
#include <decaf/util/concurrent/ConcurrentStlMap.h>
#include <transport/Command.h>

#include <state/SenderState.h>
#include <state/SubscriptionState.h>

namespace upmq {
namespace state {

using decaf::lang::Pointer;
using decaf::util::concurrent::ConcurrentStlMap;

using namespace std;
using namespace upmq::transport;

class SessionState {
 private:
  Pointer<Command> info;
  ConcurrentStlMap<string, Pointer<SenderState>> producers;
  ConcurrentStlMap<string, Pointer<SubscriptionState>> consumers;

 public:
  SessionState(const SessionState &) = delete;
  SessionState &operator=(const SessionState &) = delete;

  SessionState(Pointer<Command> info);
  virtual ~SessionState();

  Pointer<Command> getInfo() const;

  void addProducer(Pointer<Command> command);
  Pointer<SenderState> removeProducer(const string &id);

  void addConsumer(Pointer<Command> command);
  Pointer<SubscriptionState> removeConsumer(const string &id);

  const decaf::util::Collection<Pointer<SenderState>> &getProducerStates() const;
  Pointer<SenderState> getProducerState(const string &id);

  const decaf::util::Collection<Pointer<SubscriptionState>> &getConsumerStates() const;
  Pointer<SubscriptionState> getConsumerState(const string &id);

  void shutdown();
};
}  // namespace state
}  // namespace upmq

#endif /*UPMQ_SESSION_STATE_H_*/
