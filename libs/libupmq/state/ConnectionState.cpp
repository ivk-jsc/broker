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

#include "ConnectionState.h"

#include <decaf/lang/exceptions/IllegalStateException.h>

using namespace upmq;
using namespace upmq::state;
using namespace upmq::transport;

////////////////////////////////////////////////////////////////////////////////
ConnectionState::ConnectionState(Pointer<Command> info)
    : info(info), sessions(), tempDestinations(), disposed(false), connectionInterruptProcessingComplete(true) {}

////////////////////////////////////////////////////////////////////////////////
ConnectionState::~ConnectionState() {
  sessions.clear();
  tempDestinations.clear();
}

////////////////////////////////////////////////////////////////////////////////
void ConnectionState::reset(Pointer<Command> command) {
  this->info = std::move(command);
  sessions.clear();
  tempDestinations.clear();
  disposed.set(false);
}

////////////////////////////////////////////////////////////////////////////////
void ConnectionState::shutdown() {
  if (this->disposed.compareAndSet(false, true)) {
    Pointer<Iterator<Pointer<SessionState>>> iterator(this->sessions.values().iterator());
    while (iterator->hasNext()) {
      iterator->next()->shutdown();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ConnectionState::checkShutdown() const {
  if (this->disposed.get()) {
    throw decaf::lang::exceptions::IllegalStateException(__FILE__, __LINE__, "Connection already Disposed");
  }
}

////////////////////////////////////////////////////////////////////////////////
void ConnectionState::removeTempDestination(const Pointer<Command> &destination) {
  DECAF_UNUSED_VAR(destination);
  //  std::unique_ptr<decaf::util::Iterator<Pointer<DestinationInfo> > > iter(tempDestinations.iterator());
  //
  //  while (iter->hasNext()) {
  //    Pointer<DestinationInfo> di = iter->next();
  //    if (di->getDestination()->equals(destination.get())) {
  //      iter->remove();
  //    }
  //  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionState::getInfo() const { return this->info; }

////////////////////////////////////////////////////////////////////////////////
void ConnectionState::addTempDestination(const Pointer<Command> &command) {
  checkShutdown();
  tempDestinations.add(command);
}

////////////////////////////////////////////////////////////////////////////////
void ConnectionState::addSession(Pointer<Command> command) {
  checkShutdown();
  const std::string currId = command->getCurrId();
  sessions.put(currId, Pointer<SessionState>(new SessionState(std::move(command))));
}

////////////////////////////////////////////////////////////////////////////////
Pointer<SessionState> ConnectionState::removeSession(const string &id) { return sessions.remove(id); }

////////////////////////////////////////////////////////////////////////////////
Pointer<SessionState> ConnectionState::getSessionState(const Pointer<Command> &id) const { return sessions.get(id->getParentId()); }

////////////////////////////////////////////////////////////////////////////////
const LinkedList<Pointer<Command>> &ConnectionState::getTempDestinations() const { return tempDestinations; }

////////////////////////////////////////////////////////////////////////////////
const decaf::util::Collection<Pointer<SessionState>> &ConnectionState::getSessionStates() const { return sessions.values(); }

////////////////////////////////////////////////////////////////////////////////
void ConnectionState::setConnectionInterruptProcessingComplete(bool isConnectionInterruptProcessingComplete) {
  this->connectionInterruptProcessingComplete = isConnectionInterruptProcessingComplete;
}

////////////////////////////////////////////////////////////////////////////////
bool ConnectionState::isConnectionInterruptProcessingComplete() const { return this->connectionInterruptProcessingComplete; }
