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

#include "SessionState.h"

#include <libupmq/ExceptionImpl.h>
#include <utility>
using namespace upmq;
using namespace upmq::state;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::lang;

////////////////////////////////////////////////////////////////////////////////
SessionState::SessionState(Pointer<Command> info) : info(std::move(info)), producers(), consumers() {}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> SessionState::getInfo() const { return this->info; }

////////////////////////////////////////////////////////////////////////////////
SessionState::~SessionState() {
  try {
    this->shutdown();
  }
  CATCH_ALL_NOTHROW
}

////////////////////////////////////////////////////////////////////////////////
void SessionState::shutdown() {
  this->producers.clear();
  this->consumers.clear();
}

////////////////////////////////////////////////////////////////////////////////
void SessionState::addProducer(Pointer<Command> command) {
  const std::string currId = command->getCurrId();
  producers.put(currId, Pointer<SenderState>(new SenderState(std::move(command))));
}

////////////////////////////////////////////////////////////////////////////////
Pointer<SenderState> SessionState::removeProducer(const string &id) {
  Pointer<SenderState> producerState = producers.remove(id);
  return producerState;
}

////////////////////////////////////////////////////////////////////////////////
void SessionState::addConsumer(Pointer<Command> command) {
  const std::string currId = command->getCurrId();
  consumers.put(currId, Pointer<SubscriptionState>(new SubscriptionState(std::move(command))));
}

////////////////////////////////////////////////////////////////////////////////
Pointer<SubscriptionState> SessionState::removeConsumer(const string &id) { return consumers.remove(id); }

////////////////////////////////////////////////////////////////////////////////
const decaf::util::Collection<Pointer<SenderState>> &SessionState::getProducerStates() const { return producers.values(); }

////////////////////////////////////////////////////////////////////////////////
Pointer<SenderState> SessionState::getProducerState(const string &id) { return producers.get(id); }

////////////////////////////////////////////////////////////////////////////////
const decaf::util::Collection<Pointer<SubscriptionState>> &SessionState::getConsumerStates() const { return consumers.values(); }

////////////////////////////////////////////////////////////////////////////////
Pointer<SubscriptionState> SessionState::getConsumerState(const string &id) { return consumers.get(id); }
