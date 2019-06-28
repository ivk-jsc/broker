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

#ifndef _UPMQ_SUBSCRIPTION_STATE_H_
#define _UPMQ_SUBSCRIPTION_STATE_H_

#include <state/SubscribeState.h>
#include <transport/Command.h>

#include <decaf/lang/Pointer.h>
#include <decaf/util/concurrent/ConcurrentStlMap.h>

namespace upmq {
namespace state {

using namespace std;
using decaf::lang::Pointer;
using decaf::util::concurrent::ConcurrentStlMap;

class SubscriptionState {
 private:
  Pointer<Command> info;
  ConcurrentStlMap<string, Pointer<SubscribeState>> subscribes;

 private:
  SubscriptionState(const SubscriptionState &);
  SubscriptionState &operator=(const SubscriptionState &);

 public:
  SubscriptionState(Pointer<Command> info);
  virtual ~SubscriptionState();

  Pointer<Command> getInfo() const;

  void addSubscribe(Pointer<Command> command);
  Pointer<SubscribeState> removeSubscribe(string id);

  const decaf::util::Collection<Pointer<SubscribeState>> &getSubscribeStates() const;
};
}  // namespace state
}  // namespace upmq

#endif /*_UPMQ_SUBSCRIPTION_STATE_H_*/
