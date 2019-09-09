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

#include "ConnectionStateTracker.h"

#include <decaf/lang/Runnable.h>
#include <decaf/util/HashCode.h>

#include <transport/TransportListener.h>
#include <transport/UPMQException.h>
#include <transport/WireFormat.h>

using namespace std;
using namespace upmq;
using namespace upmq::state;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::io;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
namespace upmq {
namespace state {

class StateTrackerImpl {
 private:
  StateTrackerImpl(const StateTrackerImpl &);
  StateTrackerImpl &operator=(const StateTrackerImpl &);

 public:
  ConnectionStateTracker *parent;
  const Pointer<Tracked> TRACKED_RESPONSE_MARKER;
  Pointer<ConnectionState> connectionState;

  StateTrackerImpl(ConnectionStateTracker *parent) : parent(parent) {}

  Pointer<Command> markerAsCommand() const {
    return (TRACKED_RESPONSE_MARKER != nullptr ? TRACKED_RESPONSE_MARKER.dynamicCast<Command>() : Pointer<Command>());
  }

  ~StateTrackerImpl() {}
};
}  // namespace state
}  // namespace upmq

////////////////////////////////////////////////////////////////////////////////
ConnectionStateTracker::ConnectionStateTracker()
    : impl(new StateTrackerImpl(this)), restoreSessions(true), restoreConsumers(true), restoreProducers(true) {}

////////////////////////////////////////////////////////////////////////////////
ConnectionStateTracker::~ConnectionStateTracker() {
  try {
    delete impl;
  }
  AMQ_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Tracked> ConnectionStateTracker::track(const Pointer<Command> &command) {
  try {
    Pointer<Command> result;

    if (command->isConnect()) {
      result = processConnect(command);
    }
    if (command->isDisconnect()) {
      result = processDisconnect(command);
    } else if (command->isSession()) {
      result = processSession(command);
    } else if (command->isUnsession()) {
      result = processUnsession(command);
    } else if (command->isSubscription()) {
      result = processSubscription(command);
    } else if (command->isUnsubscription()) {
      result = processUnsubscription(command);
    } else if (command->isSubscribe()) {
      result = processSubscribe(command);
    } else if (command->isUnsubscribe()) {
      result = processUnsubscribe(command);
    } else if (command->isSender()) {
      result = processSender(command);
    } else if (command->isUnsender()) {
      result = processUnsender(command);
    }

    if (result == nullptr) {
      return Pointer<Tracked>();
    }
    return result.dynamicCast<Tracked>();
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::restore(const Pointer<transport::Transport> &transport) {
  try {
    Pointer<ConnectionState> state = this->impl->connectionState;
    if (state.get() != nullptr) {
      Pointer<Command> info = state->getInfo();
      transport->oneway(info);

      doRestoreTempDestinations(transport, state);

      if (restoreSessions) {
        doRestoreSessions(transport, state);
      }
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processConnect(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<Command> infoCopy(info->duplicate());
      this->impl->connectionState = Pointer<ConnectionState>(new ConnectionState(std::move(infoCopy)));
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processDisconnect(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        this->impl->connectionState.reset(nullptr);
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processSession(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        cs->addSession(Pointer<Command>(info->duplicate()));
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processUnsession(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        cs->removeSession(info->getCurrId());
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
decaf::lang::Pointer<Command> ConnectionStateTracker::processSender(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        Pointer<SessionState> ss = cs->getSessionState(info);
        if (ss.get() != nullptr) {
          ss->addProducer(Pointer<Command>(info->duplicate()));
        }
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processUnsender(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        Pointer<SessionState> ss = cs->getSessionState(info);
        if (ss.get() != nullptr) {
          ss->removeProducer(info->getCurrId());
        }
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processSubscription(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        Pointer<SessionState> ss = cs->getSessionState(info);
        if (ss.get() != nullptr) {
          ss->addConsumer(Pointer<Command>(info->duplicate()));
        }
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processUnsubscription(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        Pointer<SessionState> ss = cs->getSessionState(info);
        if (ss.get() != nullptr) {
          ss->removeConsumer(info->getCurrId());
        }
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processSubscribe(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        Pointer<SessionState> ss = cs->getSessionState(info);
        if (ss.get() != nullptr) {
          Pointer<SubscriptionState> sss = ss->getConsumerState(info->getCurrId());
          if (sss.get() != nullptr) {
            sss->addSubscribe(Pointer<Command>(info->duplicate()));
          }
        }
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> ConnectionStateTracker::processUnsubscribe(const Pointer<Command> &info) {
  try {
    if (info.get() != nullptr) {
      Pointer<ConnectionState> cs = this->impl->connectionState;
      if (cs.get() != nullptr) {
        Pointer<SessionState> ss = cs->getSessionState(info);
        if (ss.get() != nullptr) {
          Pointer<SubscriptionState> sss = ss->getConsumerState(info->getCurrId());
          if (sss.get() != nullptr) {
            sss->removeSubscribe(info->getCurrId());
          }
        }
      }
    }
    return this->impl->markerAsCommand();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::doRestoreSessions(const Pointer<transport::Transport> &transport, const Pointer<ConnectionState> &connectionState) {
  try {
    Pointer<Iterator<Pointer<SessionState> > > iter(connectionState->getSessionStates().iterator());
    while (iter->hasNext()) {
      Pointer<SessionState> state = iter->next();
      transport->oneway(state->getInfo());

      if (restoreProducers) {
        doRestoreProducers(transport, state);
      }

      if (restoreConsumers) {
        doRestoreConsumers(transport, state);
      }
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::doRestoreConsumers(const Pointer<transport::Transport> &transport, const Pointer<SessionState> &sessionState) {
  try {
    Pointer<ConnectionState> connectionState = this->impl->connectionState;
    if (connectionState.get() != nullptr) {
      Pointer<Iterator<Pointer<SubscriptionState> > > state(sessionState->getConsumerStates().iterator());
      while (state->hasNext()) {
        Pointer<SubscriptionState> subscription = state->next();

        transport->oneway(subscription->getInfo());

        Pointer<Iterator<Pointer<SubscribeState> > > state2(subscription->getSubscribeStates().iterator());
        while (state2->hasNext()) {
          transport->oneway(state2->next()->getInfo());
        }
      }
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::doRestoreProducers(const Pointer<transport::Transport> &transport, const Pointer<SessionState> &sessionState) {
  try {
    Pointer<ConnectionState> connectionState = this->impl->connectionState;
    if (connectionState.get() != nullptr) {
      Pointer<Iterator<Pointer<SenderState> > > state(sessionState->getProducerStates().iterator());
      while (state->hasNext()) {
        Pointer<SenderState> subscription = state->next();

        transport->oneway(subscription->getInfo());
      }
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::doRestoreTempDestinations(const Pointer<transport::Transport> &transport,
                                                       const Pointer<ConnectionState> &connectionState) {
  DECAF_UNUSED_VAR(transport);
  DECAF_UNUSED_VAR(connectionState);
}

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::transportInterrupted() {
  Pointer<ConnectionState> cs = this->impl->connectionState;
  if (cs.get() != nullptr) {
    this->impl->connectionState->setConnectionInterruptProcessingComplete(false);
  }
}

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::setRestoreConsumers(bool isRestoreConsumers) { this->restoreConsumers = isRestoreConsumers; }

//////////////////////////////////////////////////////////////////////////////////
bool ConnectionStateTracker::isRestoreConsumers() const { return this->restoreConsumers; }

//////////////////////////////////////////////////////////////////////////////////
bool ConnectionStateTracker::isRestoreProducers() const { return this->restoreProducers; }

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::setRestoreProducers(bool isRestoreProducers) { this->restoreProducers = isRestoreProducers; }

//////////////////////////////////////////////////////////////////////////////////
bool ConnectionStateTracker::isRestoreSessions() const { return this->restoreSessions; }

//////////////////////////////////////////////////////////////////////////////////
void ConnectionStateTracker::setRestoreSessions(bool isRestoreSessions) { this->restoreSessions = isRestoreSessions; }
