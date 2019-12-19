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

#include "SubscriptionState.h"

using namespace upmq;
using namespace upmq::state;
using namespace decaf::lang;

////////////////////////////////////////////////////////////////////////////////
SubscriptionState::SubscriptionState(Pointer<Command> info_) : info(std::move(info_)) {}

////////////////////////////////////////////////////////////////////////////////
SubscriptionState::~SubscriptionState() {}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> SubscriptionState::getInfo() const { return this->info; }

////////////////////////////////////////////////////////////////////////////////
void SubscriptionState::addSubscribe(Pointer<Command> command) {
  const std::string currId = command->getCurrId();
  subscribes.put(currId, Pointer<SubscribeState>(new SubscribeState(std::move(command))));
}

////////////////////////////////////////////////////////////////////////////////
Pointer<SubscribeState> SubscriptionState::removeSubscribe(const string& id) {
  Pointer<SubscribeState> producerState = subscribes.remove(id);
  return producerState;
}

////////////////////////////////////////////////////////////////////////////////
const decaf::util::Collection<Pointer<SubscribeState>>& SubscriptionState::getSubscribeStates() const { return subscribes.values(); }
