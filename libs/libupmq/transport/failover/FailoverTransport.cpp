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

#include "FailoverTransport.h"

#include <state/Tracked.h>
#include <threads/CompositeTaskRunner.h>
#include <transport/TransportRegistry.h>
#include <transport/failover/BackupTransportPool.h>
#include <transport/failover/CloseTransportsTask.h>
#include <transport/failover/FailoverTransportListener.h>
#include <transport/failover/URIPool.h>

#include <decaf/lang/Pointer.h>
#include <decaf/lang/System.h>
#include <decaf/util/LinkedList.h>
#include <decaf/util/Random.h>
#include <decaf/util/StlMap.h>
#include <decaf/util/StlSet.h>
#include <decaf/util/StringTokenizer.h>
#include <decaf/util/concurrent/Mutex.h>
#include <decaf/util/concurrent/TimeUnit.h>

using namespace std;
using namespace upmq;
using namespace upmq::threads;
using namespace upmq::transport;
using namespace upmq::transport::failover;
using namespace upmq::state;
using namespace decaf;
using namespace decaf::io;
using namespace decaf::net;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
namespace upmq {
namespace transport {
namespace failover {

class FailoverTransportImpl {
 private:
  FailoverTransportImpl(const FailoverTransportImpl &);
  FailoverTransportImpl &operator=(const FailoverTransportImpl &);

  static const int DEFAULT_INITIAL_RECONNECT_DELAY;
  static const int INFINITE_WAIT;

 public:
  bool closed;
  bool connected;
  bool started;

  long long timeout;
  long long initialReconnectDelay;
  long long maxReconnectDelay;
  long long backOffMultiplier;
  bool useExponentialBackOff;
  bool initialized;
  int maxReconnectAttempts;
  int startupMaxReconnectAttempts;
  int connectFailures;
  long long reconnectDelay;
  bool trackMessages;
  bool trackTransactionProducers;
  int maxCacheSize;
  int maxPullCacheSize;
  bool connectionInterruptProcessingComplete;
  bool firstConnection;
  bool updateURIsSupported;
  bool reconnectSupported;
  bool rebalanceUpdateURIs;
  bool priorityBackup;
  bool backupsEnabled;

  bool doRebalance;
  bool connectedToPrioirty;

  mutable Mutex reconnectMutex;
  mutable Mutex sleepMutex;
  mutable Mutex listenerMutex;

  StlMap<int, Pointer<Command> > requestMap;

  Pointer<URIPool> uris;
  Pointer<URIPool> priorityUris;
  Pointer<URIPool> updated;
  Pointer<URI> connectedTransportURI;
  Pointer<Transport> connectedTransport;
  Pointer<Exception> connectionFailure;
  Pointer<BackupTransportPool> backups;
  Pointer<CloseTransportsTask> closeTask;
  Pointer<CompositeTaskRunner> taskRunner;
  Pointer<TransportListener> disposedListener;
  Pointer<TransportListener> myTransportListener;

  TransportListener *transportListener;

  FailoverTransportImpl(FailoverTransport *parent)
      : closed(false),
        connected(false),
        started(false),
        timeout(INFINITE_WAIT),
        initialReconnectDelay(DEFAULT_INITIAL_RECONNECT_DELAY),
        maxReconnectDelay(1000 * 30),
        backOffMultiplier(2),
        useExponentialBackOff(true),
        initialized(false),
        maxReconnectAttempts(INFINITE_WAIT),
        startupMaxReconnectAttempts(INFINITE_WAIT),
        connectFailures(0),
        reconnectDelay(DEFAULT_INITIAL_RECONNECT_DELAY),
        trackMessages(false),
        trackTransactionProducers(true),
        maxCacheSize(128 * 1024),
        maxPullCacheSize(10),
        connectionInterruptProcessingComplete(false),
        firstConnection(true),
        updateURIsSupported(true),
        reconnectSupported(true),
        rebalanceUpdateURIs(true),
        priorityBackup(false),
        backupsEnabled(false),
        doRebalance(false),
        connectedToPrioirty(false),
        reconnectMutex(),
        sleepMutex(),
        listenerMutex(),
        requestMap(),
        uris(new URIPool()),
        priorityUris(new URIPool()),
        updated(new URIPool()),
        connectedTransportURI(),
        connectedTransport(),
        connectionFailure(),
        backups(),
        closeTask(new CloseTransportsTask()),
        taskRunner(new CompositeTaskRunner()),
        disposedListener(),
        myTransportListener(new FailoverTransportListener(parent)),
        transportListener(nullptr) {
    this->backups.reset(new BackupTransportPool(parent, taskRunner, closeTask, uris, updated, priorityUris));

    this->taskRunner->addTask(parent);
    this->taskRunner->addTask(this->closeTask.get());
  }

  bool isPriority(const decaf::net::URI &uri) { return priorityUris->contains(uri) || uris->isPriority(uri); }

  Pointer<URIPool> getConnectList() {
    // Pick an appropriate URI pool, updated is always preferred if updates are
    // enabled and we have any, otherwise we fallback to our original list so that
    // we ensure we always try something.
    Pointer<URIPool> uriPool = this->uris;
    if (this->updateURIsSupported && !this->updated->isEmpty()) {
      uriPool = this->updated;
    }
    return uriPool;
  }

  void doDelay() {
    if (reconnectDelay > 0) {
      synchronized(&sleepMutex) {
        try {
          sleepMutex.wait(reconnectDelay);
        } catch (InterruptedException &) {
          Thread::currentThread()->interrupt();
        }
      }
    }

    if (useExponentialBackOff) {
      // Exponential increment of reconnect delay.
      reconnectDelay *= backOffMultiplier;
      if (reconnectDelay > maxReconnectDelay) {
        reconnectDelay = maxReconnectDelay;
      }
    }
  }

  int calculateReconnectAttemptLimit() const {
    int maxReconnectValue = maxReconnectAttempts;
    if (firstConnection && startupMaxReconnectAttempts != INFINITE_WAIT) {
      maxReconnectValue = startupMaxReconnectAttempts;
    }
    return maxReconnectValue;
  }

  bool canReconnect() const { return started && 0 != calculateReconnectAttemptLimit(); }

  /**
   * This must be called with the reconnect mutex locked.
   */
  void propagateFailureToExceptionListener() {
    if (this->transportListener != nullptr) {
      Pointer<IOException> ioException;
      try {
        ioException = this->connectionFailure.dynamicCast<IOException>();
      }
      AMQ_CATCH_NOTHROW(ClassCastException)

      if (ioException != nullptr) {
        transportListener->onException(*this->connectionFailure);
      } else {
        transportListener->onException(IOException(*this->connectionFailure));
      }
    }

    reconnectMutex.notifyAll();
  }

  void resetReconnectDelay() {
    if (!useExponentialBackOff || reconnectDelay == DEFAULT_INITIAL_RECONNECT_DELAY) {
      reconnectDelay = initialReconnectDelay;
    }
  }

  bool isClosedOrFailed() const { return closed || connectionFailure != nullptr; }

  bool isConnectionStateValid() const { return connectedTransport != nullptr && !doRebalance && !this->backups->isPriorityBackupAvailable(); }

  void disconnect() {
    Pointer<Transport> transport;
    transport.swap(this->connectedTransport);

    if (transport != nullptr) {
      if (this->disposedListener != nullptr) {
        transport->setTransportListener(this->disposedListener.get());
      }

      // Hand off to the close task so it gets done in a different thread.
      this->closeTask->add(transport);

      if (this->connectedTransportURI != nullptr) {
        this->uris->addURI(*this->connectedTransportURI);
        this->connectedTransportURI.reset(nullptr);
      }
    }
  }
};

const int FailoverTransportImpl::DEFAULT_INITIAL_RECONNECT_DELAY = 10;
const int FailoverTransportImpl::INFINITE_WAIT = -1;
}  // namespace failover
}  // namespace transport
}  // namespace upmq

////////////////////////////////////////////////////////////////////////////////
FailoverTransport::FailoverTransport() : stateTracker(), impl(nullptr) { this->impl = new FailoverTransportImpl(this); }

////////////////////////////////////////////////////////////////////////////////
FailoverTransport::~FailoverTransport() {
  try {
    close();
  }
  AMQ_CATCHALL_NOTHROW()

  try {
    delete this->impl;
  }
  AMQ_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::add(bool rebalance, const std::string &uri) {
  try {
    if (this->impl->uris->addURI(URI(uri))) {
      reconnect(rebalance);
    }
  }
  AMQ_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::addURI(bool rebalance, const List<URI> &uris) {
  bool newUri = false;

  std::unique_ptr<Iterator<URI> > iter(uris.iterator());
  while (iter->hasNext()) {
    if (this->impl->uris->addURI(iter->next())) {
      newUri = true;
    }
  }

  if (newUri) {
    reconnect(rebalance);
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::removeURI(bool rebalance, const List<URI> &uris) {
  bool changed = false;

  std::unique_ptr<Iterator<URI> > iter(uris.iterator());
  synchronized(&this->impl->reconnectMutex) {
    while (iter->hasNext()) {
      if (this->impl->uris->removeURI(iter->next())) {
        changed = true;
      }
    }
  }

  if (changed) {
    reconnect(rebalance);
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::reconnect(const decaf::net::URI &uri) {
  try {
    if (this->impl->uris->addURI(uri)) {
      reconnect(true);
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setTransportListener(TransportListener *newListener) {
  synchronized(&this->impl->listenerMutex) {
    this->impl->transportListener = newListener;
    this->impl->listenerMutex.notifyAll();
  }
}

////////////////////////////////////////////////////////////////////////////////
TransportListener *FailoverTransport::getTransportListener() const {
  TransportListener *listener = nullptr;
  synchronized(&this->impl->listenerMutex) { listener = this->impl->transportListener; }

  return listener;
}

////////////////////////////////////////////////////////////////////////////////
std::string FailoverTransport::getRemoteAddress() const {
  std::string addr;
  synchronized(&this->impl->reconnectMutex) {
    if (this->impl->connectedTransport != nullptr) {
      addr = this->impl->connectedTransport->getRemoteAddress();
    }
  }
  return addr;
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::oneway(const Pointer<Command> command) {
  Pointer<Exception> error;
  class TScopedLock {
    decaf::util::concurrent::Lock &lock_;

   public:
    explicit TScopedLock(decaf::util::concurrent::Lock &lock) : lock_(lock) { lock_.lock(); }
    ~TScopedLock() noexcept {
      try {
        lock_.unlock();
      } catch (...) {
      }
    }
  };
  try {
    {
      Lock lock(&this->impl->reconnectMutex);
      if (command != nullptr && this->impl->connectedTransport == nullptr) {
        if (command->isSender()) {
          // Skipping send of ShutdownInfo command when not connected.
          return;
        }

        if (command->isBrowserInfo() || command->isBegin()) {
          // Simulate response to RemoveInfo command or Ack as they will be stale.
          //          stateTracker.track(command);

          if (command->isResponseRequired()) {
            //            Pointer<Response> response(new Response());
            //            response->setCorrelationId(command->getCommandId());
            //            this->impl->myTransportListener->onCommand(response);
          }

          return;
        }
        if (command->isCommit()) {
          // Simulate response to MessagePull if timed as we can't honor that now.
          //          Pointer<MessagePull> pullRequest = command.dynamicCast<MessagePull>();
          //          if (pullRequest->getTimeout() != 0) {
          //            Pointer<MessageDispatch> dispatch(new MessageDispatch());
          //            dispatch->setConsumerId(pullRequest->getConsumerId());
          //            dispatch->setDestination(pullRequest->getDestination());
          //            this->impl->myTransportListener->onCommand(dispatch);
          //          }

          return;
        }
      }

      // Keep trying until the message is sent.
      for (int i = 0; !this->impl->closed; i++) {
        try {
          // Wait for transport to be connected.
          Pointer<Transport> transport = this->impl->connectedTransport;
          long long start = System::currentTimeMillis();
          bool timedout = false;

          while (transport == nullptr && !this->impl->closed && this->impl->connectionFailure == nullptr) {
            long long end = System::currentTimeMillis();
            if (command->isUnsubscription() && this->impl->timeout > 0 && (end - start > this->impl->timeout)) {
              timedout = true;
              break;
            }

            this->impl->reconnectMutex.wait(100);
            transport = this->impl->connectedTransport;
          }

          if (transport == nullptr) {
            // Previous loop may have exited due to us being disposed.
            if (this->impl->closed) {
              error.reset(new IOException(__FILE__, __LINE__, "Transport disposed."));
            } else if (this->impl->connectionFailure != nullptr) {
              error = this->impl->connectionFailure;
            } else if (timedout) {
              error.reset(new IOException(__FILE__, __LINE__, "Failover timeout of %d ms reached.", this->impl->timeout));
            } else {
              error.reset(new IOException(__FILE__, __LINE__, "Unexpected failure."));
            }

            break;
          }

          // If it was a request and it was not being tracked by the state
          // tracker, then hold it in the requestMap so that we can replay
          // it later.
          Pointer<Tracked> tracked;
          try {
            tracked = stateTracker.track(command);
            synchronized(&this->impl->requestMap) {
              if (tracked != nullptr && tracked->isWaitingForResponse()) {
                this->impl->requestMap.put(command->getCommandId(), tracked.dynamicCast<Command>());
              } else if (tracked == nullptr && command->isResponseRequired()) {
                this->impl->requestMap.put(command->getCommandId(), command);
              }
            }
          } catch (Exception &ex) {
            ex.setMark(__FILE__, __LINE__);
            error.reset(ex.clone());
            break;
          }

          // Send the message.
          try {
            transport->oneway(command);
          } catch (IOException &e) {
            e.setMark(__FILE__, __LINE__);

            // If the command was not tracked.. we will retry in
            // this method
            if (tracked == nullptr) {
              // since we will retry in this method.. take it out of the
              // request map so that it is not sent 2 times on recovery
              if (command->isResponseRequired()) {
                this->impl->requestMap.remove(command->getCommandId());
              }

              // re-throw the exception so it will handled by the outer catch
              throw;
            }
            // Trigger the reconnect since we can't count on inactivity or
            // other socket events to trip the failover condition.
            handleTransportFailure(e);
          }

          return;
        } catch (IOException &e) {
          e.setMark(__FILE__, __LINE__);
          handleTransportFailure(e);
        }
      }
    }
  } catch (InterruptedException &) {
    Thread::currentThread()->interrupt();
    throw InterruptedIOException(__FILE__, __LINE__, "FailoverTransport oneway() interrupted");
  }
  AMQ_CATCHALL_NOTHROW()

  if (!this->impl->closed) {
    if (error != nullptr) {
      throw IOException(*error);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<FutureResponse> FailoverTransport::asyncRequest(const Pointer<Command> command UPMQCPP_UNUSED,
                                                        const Pointer<ResponseCallback> responseCallback UPMQCPP_UNUSED) {
  DECAF_UNUSED_VAR(command);
  DECAF_UNUSED_VAR(responseCallback);
  throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "FailoverTransport::asyncRequest - Not Supported");
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> FailoverTransport::request(const Pointer<Command> command UPMQCPP_UNUSED) {
  DECAF_UNUSED_VAR(command);
  throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "FailoverTransport::request - Not Supported");
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> FailoverTransport::request(const Pointer<Command> command UPMQCPP_UNUSED, unsigned int timeout UPMQCPP_UNUSED) {
  DECAF_UNUSED_VAR(command);
  DECAF_UNUSED_VAR(timeout);
  throw decaf::lang::exceptions::UnsupportedOperationException(__FILE__, __LINE__, "FailoverTransport::request - Not Supported");
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::start() {
  try {
    synchronized(&this->impl->reconnectMutex) {
      if (this->impl->started) {
        return;
      }

      this->impl->started = true;

      if (this->impl->backupsEnabled || this->impl->priorityBackup) {
        this->impl->backups->setEnabled(true);
      }
      this->impl->taskRunner->start();

      if (this->impl->connectedTransport != nullptr) {
        stateTracker.restore(this->impl->connectedTransport);
      } else {
        reconnect(false);
      }
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::stop() {
  try {
    synchronized(&this->impl->reconnectMutex) {
      this->impl->started = false;
      this->impl->backups->setEnabled(false);
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::close() {
  try {
    Pointer<Transport> transportToStop;

    synchronized(&this->impl->reconnectMutex) {
      if (this->impl->closed) {
        return;
      }

      this->impl->started = false;
      this->impl->closed = true;
      this->impl->connected = false;

      this->impl->backups->setEnabled(false);
      this->impl->requestMap.clear();

      if (this->impl->connectedTransport != nullptr) {
        transportToStop.swap(this->impl->connectedTransport);
      }

      this->impl->reconnectMutex.notifyAll();
    }

    this->impl->backups->close();

    synchronized(&this->impl->sleepMutex) { this->impl->sleepMutex.notifyAll(); }

    this->impl->taskRunner->shutdown(TimeUnit::MINUTES.toMillis(5));

    if (transportToStop != nullptr) {
      transportToStop->close();
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::reconnect(bool rebalance) {
  Pointer<Transport> transport;

  synchronized(&this->impl->reconnectMutex) {
    if (this->impl->started) {
      if (rebalance) {
        this->impl->doRebalance = true;
      }

      try {
        this->impl->taskRunner->wakeup();
      } catch (InterruptedException &) {
        Thread::currentThread()->interrupt();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::restoreTransport(const Pointer<Transport> transport) {
  try {
    transport->start();

    // send information to the broker - informing it we are an ft client
    //    Pointer<ConnectionControl> cc(new ConnectionControl());
    //    cc->setFaultTolerant(true);
    //    transport->oneway(cc);

    stateTracker.restore(transport);

    decaf::util::StlMap<int, Pointer<Command> > commands;
    synchronized(&this->impl->requestMap) { commands.copy(this->impl->requestMap); }

    Pointer<Iterator<Pointer<Command> > > iter(commands.values().iterator());
    while (iter->hasNext()) {
      transport->oneway(iter->next());
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::handleTransportFailure(const decaf::lang::Exception &error) {
  synchronized(&this->impl->reconnectMutex) {
    Pointer<Transport> transport;
    this->impl->connectedTransport.swap(transport);

    if (transport != nullptr) {
      if (this->impl->disposedListener != nullptr) {
        transport->setTransportListener(this->impl->disposedListener.get());
      }

      // Hand off to the close task so it gets done in a different thread.
      this->impl->closeTask->add(transport);

      bool reconnectOk = this->impl->canReconnect();
      URI failedUri = *this->impl->connectedTransportURI;

      this->impl->initialized = false;
      this->impl->uris->addURI(failedUri);
      this->impl->connectedTransportURI.reset(nullptr);
      this->impl->connected = false;
      this->impl->connectedToPrioirty = false;

      // Place the State Tracker into a reconnection state.
      this->stateTracker.transportInterrupted();

      // Notify before we attempt to reconnect so that the consumers have a chance
      // to cleanup their state.
      if (reconnectOk) {
        if (this->impl->transportListener != nullptr) {
          this->impl->transportListener->transportInterrupted();
        }

        this->impl->updated->removeURI(failedUri);
        this->impl->taskRunner->wakeup();
      } else if (!this->impl->closed) {
        this->impl->connectionFailure.reset(error.clone());
        this->impl->propagateFailureToExceptionListener();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::handleConnectionControl(const Pointer<Command> control) {
  try {
    //    Pointer<ConnectionControl> ctrlCommand = control.dynamicCast<ConnectionControl>();
    //
    //    std::string reconnectStr = ctrlCommand->getReconnectTo();
    //    if (!reconnectStr.empty()) {
    //
    //      std::remove(reconnectStr.begin(), reconnectStr.end(), ' ');
    //
    //      if (reconnectStr.length() > 0) {
    //        try {
    //          if (isReconnectSupported()) {
    //            reconnect(URI(reconnectStr));
    //          }
    //        }
    //        catch (Exception &e) {
    //        }
    //      }
    //    }
    //
    //    processNewTransports(ctrlCommand->isRebalanceConnection(), ctrlCommand->getConnectedBrokers());
  }
  AMQ_CATCH_RETHROW(Exception)
  AMQ_CATCHALL_THROW(Exception)
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::processNewTransports(bool rebalance, std::string newTransports) {
  if (!newTransports.empty()) {
    auto it = std::remove(newTransports.begin(), newTransports.end(), ' ');
    newTransports.erase(it);

    if (newTransports.length() > 0 && isUpdateURIsSupported()) {
      LinkedList<URI> list;
      StringTokenizer tokenizer(newTransports, ",");

      while (tokenizer.hasMoreTokens()) {
        std::string str = tokenizer.nextToken();
        try {
          URI uri(str);
          list.add(uri);
        } catch (Exception &) {
        }
      }

      if (!list.isEmpty()) {
        try {
          updateURIs(rebalance, list);
        } catch (IOException &) {
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::updateURIs(bool rebalance, const decaf::util::List<decaf::net::URI> &updatedURIs) {
  if (isUpdateURIsSupported()) {
    Pointer<URIPool> copy(new URIPool(*this->impl->updated));
    this->impl->updated->clear();

    if (!updatedURIs.isEmpty()) {
      StlSet<URI> set;

      for (int i = 0; i < updatedURIs.size(); i++) {
        set.add(updatedURIs.get(i));
      }

      Pointer<Iterator<URI> > setIter(set.iterator());
      while (setIter->hasNext()) {
        URI value = setIter->next();
        this->impl->updated->addURI(value);
      }

      if (!(copy->isEmpty() && this->impl->updated->isEmpty()) && !(copy->equals(*this->impl->updated))) {
        synchronized(&this->impl->reconnectMutex) { reconnect(rebalance); }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isPending() const {
  bool result = false;

  synchronized(&this->impl->reconnectMutex) {
    if (!this->impl->isConnectionStateValid() && this->impl->started && !this->impl->isClosedOrFailed()) {
      int maxReconnectAttempts = this->impl->calculateReconnectAttemptLimit();

      if (maxReconnectAttempts != -1 && this->impl->connectFailures >= maxReconnectAttempts) {
        result = false;
      } else {
        result = true;
      }
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::iterate() {
  Pointer<Exception> failure;

  synchronized(&this->impl->reconnectMutex) {
    if (this->impl->isClosedOrFailed()) {
      this->impl->reconnectMutex.notifyAll();
    }

    if (this->impl->isConnectionStateValid() || this->impl->isClosedOrFailed()) {
      return false;
    } else {
      Pointer<URIPool> connectList = this->impl->getConnectList();

      if (connectList->isEmpty()) {
        failure.reset(new IOException(__FILE__, __LINE__, "No URIs available for reconnect."));
      } else {
        if (this->impl->doRebalance) {
          if (this->impl->connectedToPrioirty || connectList->getPriorityURI().equals(*this->impl->connectedTransportURI)) {
            // already connected to first in the list, no need to rebalance
            this->impl->doRebalance = false;
            return false;
          } else {
            // break any existing connect for rebalance.
            this->impl->disconnect();
          }

          this->impl->doRebalance = false;
        }

        this->impl->resetReconnectDelay();

        LinkedList<URI> failures;
        Pointer<Transport> transport;
        URI uri;

        if (this->impl->backups->isEnabled()) {
          Pointer<BackupTransport> backupTransport = this->impl->backups->getBackup();
          if (backupTransport != nullptr) {
            transport = backupTransport->getTransport();
            uri = backupTransport->getUri();
            if (this->impl->priorityBackup && this->impl->backups->isPriorityBackupAvailable()) {
              // A priority connection is available and we aren't connected to
              // any other priority transports so disconnect and use the backup.
              this->impl->disconnect();
            }
          }
        }

        // Sleep for the reconnectDelay if there's no backup and we aren't trying
        // for the first time, or we were disposed for some reason.
        if (transport == nullptr && !this->impl->firstConnection && (this->impl->reconnectDelay > 0) && !this->impl->closed) {
          synchronized(&this->impl->sleepMutex) {
            try {
              this->impl->sleepMutex.wait(this->impl->reconnectDelay);
            } catch (InterruptedException &) {
              Thread::currentThread()->interrupt();
            }
          }
        }

        while (transport == nullptr && this->impl->connectedTransport == nullptr && !this->impl->closed) {
          try {
            // We could be starting the loop with a backup already.
            if (transport == nullptr) {
              try {
                uri = connectList->getURI();
              } catch (NoSuchElementException &) {
                break;
              }

              transport = createTransport(uri);
            }

            transport->setTransportListener(this->impl->myTransportListener.get());
            transport->start();

            if (this->impl->started && !this->impl->firstConnection) {
              restoreTransport(transport);
            }

            this->impl->reconnectDelay = this->impl->initialReconnectDelay;
            this->impl->connectedTransportURI.reset(new URI(uri));
            this->impl->connectedTransport = transport;
            this->impl->reconnectMutex.notifyAll();
            this->impl->connectFailures = 0;

            if (isPriorityBackup()) {
              this->impl->connectedToPrioirty = connectList->getPriorityURI().equals(uri) || this->impl->priorityUris->contains(uri);
            } else {
              this->impl->connectedToPrioirty = false;
            }

            // Make sure on initial startup, that the transportListener
            // has been initialized for this instance.
            synchronized(&this->impl->listenerMutex) {
              if (this->impl->transportListener == nullptr) {
                // if it isn't set after 2secs - it probably never will be
                this->impl->listenerMutex.wait(2000);
              }
            }

            if (this->impl->transportListener != nullptr) {
              this->impl->transportListener->transportResumed();
            }

            if (this->impl->firstConnection) {
              this->impl->firstConnection = false;
            }

            // Return the failures to the pool, we will try again on the next iteration.
            connectList->addURIs(failures);

            this->impl->connected = true;
            return false;

          } catch (Exception &e) {
            e.setMark(__FILE__, __LINE__);

            if (transport != nullptr) {
              if (this->impl->disposedListener != nullptr) {
                transport->setTransportListener(this->impl->disposedListener.get());
              }

              try {
                transport->stop();
              } catch (...) {
              }

              // Hand off to the close task so it gets done in a different thread
              // this prevents a deadlock from occurring if the Transport happens
              // to call back through our onException method or locks in some other
              // way.
              this->impl->closeTask->add(transport);
              this->impl->taskRunner->wakeup();
              transport.reset(nullptr);
            }

            failures.add(uri);
            failure.reset(e.clone());
          }
        }

        // Return the failures to the pool, we will try again on the next iteration.
        connectList->addURIs(failures);
      }
    }

    int reconnectAttempts = this->impl->calculateReconnectAttemptLimit();

    if (reconnectAttempts >= 0 && ++this->impl->connectFailures >= reconnectAttempts) {
      this->impl->connectionFailure = failure;

      // Make sure on initial startup, that the transportListener has been initialized
      // for this instance.
      synchronized(&this->impl->listenerMutex) {
        if (this->impl->transportListener == nullptr) {
          this->impl->listenerMutex.wait(2000);
        }
      }

      this->impl->propagateFailureToExceptionListener();
      return false;
    }
  }

  if (!this->impl->closed) {
    this->impl->doDelay();
  }

  return !this->impl->closed;
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> FailoverTransport::createTransport(const URI &location) const {
  try {
    TransportFactory *factory = TransportRegistry::getInstance().findFactory(location.getScheme());

    if (factory == nullptr) {
      throw IOException(__FILE__, __LINE__, "Invalid URI specified, no valid Factory Found.");
    }

    Pointer<Transport> transport(factory->createComposite(location));

    return transport;
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setConnectionInterruptProcessingComplete(const Pointer<Command> connectionId) {
  synchronized(&this->impl->reconnectMutex) {
    //    stateTracker.connectionInterruptProcessingComplete(this, connectionId);
  }
}

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isConnected() const { return this->impl->connected; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isClosed() const { return this->impl->closed; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isInitialized() const { return this->impl->initialized; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setInitialized(bool value) { this->impl->initialized = value; }

////////////////////////////////////////////////////////////////////////////////
Transport *FailoverTransport::narrow(const std::type_info &typeId) {
  if (typeid(*this) == typeId) {
    return this;
  }

  if (this->impl->connectedTransport != nullptr) {
    return this->impl->connectedTransport->narrow(typeId);
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::processResponse(const Pointer<Response> response) {
  Pointer<Command> object;

  synchronized(&(this->impl->requestMap)) {
    try {
      object = this->impl->requestMap.remove(response->getCorrelationId());
    } catch (NoSuchElementException &) {
      // Not tracking this request in our map, not an error.
    }
  }

  if (object != nullptr) {
    try {
      //      Pointer<Tracked> tracked = object.dynamicCast<Tracked>();
      //      tracked->onResponse();
    }
    AMQ_CATCH_NOTHROW(ClassCastException)
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<WireFormat> FailoverTransport::getWireFormat() const {
  Pointer<WireFormat> result;
  Pointer<Transport> transport = this->impl->connectedTransport;

  if (transport != nullptr) {
    result = transport->getWireFormat();
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
long long FailoverTransport::getTimeout() const { return this->impl->timeout; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setTimeout(long long value) { this->impl->timeout = value; }

////////////////////////////////////////////////////////////////////////////////
long long FailoverTransport::getInitialReconnectDelay() const { return this->impl->initialReconnectDelay; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setInitialReconnectDelay(long long value) { this->impl->initialReconnectDelay = value; }

////////////////////////////////////////////////////////////////////////////////
long long FailoverTransport::getMaxReconnectDelay() const { return this->impl->maxReconnectDelay; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setMaxReconnectDelay(long long value) { this->impl->maxReconnectDelay = value; }

////////////////////////////////////////////////////////////////////////////////
long long FailoverTransport::getBackOffMultiplier() const { return this->impl->backOffMultiplier; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setBackOffMultiplier(long long value) { this->impl->backOffMultiplier = value; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isUseExponentialBackOff() const { return this->impl->useExponentialBackOff; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setUseExponentialBackOff(bool value) { this->impl->useExponentialBackOff = value; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isRandomize() const { return this->impl->uris->isRandomize(); }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setRandomize(bool value) { this->impl->uris->setRandomize(value); }

////////////////////////////////////////////////////////////////////////////////
int FailoverTransport::getMaxReconnectAttempts() const { return this->impl->maxReconnectAttempts; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setMaxReconnectAttempts(int value) { this->impl->maxReconnectAttempts = value; }

////////////////////////////////////////////////////////////////////////////////
int FailoverTransport::getStartupMaxReconnectAttempts() const { return this->impl->startupMaxReconnectAttempts; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setStartupMaxReconnectAttempts(int value) { this->impl->startupMaxReconnectAttempts = value; }

////////////////////////////////////////////////////////////////////////////////
long long FailoverTransport::getReconnectDelay() const { return this->impl->reconnectDelay; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setReconnectDelay(long long value) { this->impl->reconnectDelay = value; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isBackup() const { return this->impl->backupsEnabled; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setBackup(bool value) { this->impl->backupsEnabled = value; }

////////////////////////////////////////////////////////////////////////////////
int FailoverTransport::getBackupPoolSize() const { return this->impl->backups->getBackupPoolSize(); }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setBackupPoolSize(int value) { this->impl->backups->setBackupPoolSize(value); }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isTrackMessages() const { return this->impl->trackMessages; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setTrackMessages(bool value) { this->impl->trackMessages = value; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isTrackTransactionProducers() const { return this->impl->trackTransactionProducers; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setTrackTransactionProducers(bool value) { this->impl->trackTransactionProducers = value; }

////////////////////////////////////////////////////////////////////////////////
int FailoverTransport::getMaxCacheSize() const { return this->impl->maxCacheSize; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setMaxCacheSize(int value) { this->impl->maxCacheSize = value; }

////////////////////////////////////////////////////////////////////////////////
int FailoverTransport::getMaxPullCacheSize() const { return this->impl->maxPullCacheSize; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setMaxPullCacheSize(int value) { this->impl->maxPullCacheSize = value; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isReconnectSupported() const { return this->impl->reconnectSupported; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setReconnectSupported(bool value) { this->impl->reconnectSupported = value; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isUpdateURIsSupported() const { return this->impl->updateURIsSupported; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setUpdateURIsSupported(bool value) { this->impl->updateURIsSupported = value; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isRebalanceUpdateURIs() const { return this->impl->rebalanceUpdateURIs; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setRebalanceUpdateURIs(bool rebalanceUpdateURIs) { this->impl->rebalanceUpdateURIs = rebalanceUpdateURIs; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isPriorityBackup() const { return this->impl->priorityBackup; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setPriorityBackup(bool priorityBackup) { this->impl->priorityBackup = priorityBackup; }

////////////////////////////////////////////////////////////////////////////////
bool FailoverTransport::isConnectedToPriority() const { return this->impl->connectedToPrioirty; }

////////////////////////////////////////////////////////////////////////////////
void FailoverTransport::setPriorityURIs(const std::string &priorityURIs UPMQCPP_UNUSED) {
  StringTokenizer tokenizer(priorityURIs, ",");
  while (tokenizer.hasMoreTokens()) {
    std::string str = tokenizer.nextToken();
    try {
      this->impl->priorityUris->addURI(URI(str));
    } catch (Exception &) {
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
const List<URI> &FailoverTransport::getPriorityURIs() const { return this->impl->priorityUris->getURIList(); }
