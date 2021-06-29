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

#include "BackupTransportPool.h"

#include <memory>

#include <transport/TransportFactory.h>
#include <transport/TransportRegistry.h>
#include <transport/UPMQException.h>
#include <transport/failover/FailoverTransport.h>

#include <decaf/lang/exceptions/IllegalStateException.h>
#include <decaf/lang/exceptions/NullPointerException.h>

using namespace upmq;
using namespace upmq::threads;
using namespace upmq::transport;
using namespace upmq::transport::failover;
using namespace decaf;
using namespace decaf::io;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::net;
using namespace decaf::util;
using namespace decaf::util::concurrent;

////////////////////////////////////////////////////////////////////////////////
namespace upmq {
namespace transport {
namespace failover {

class BackupTransportPoolImpl {
 public:
  BackupTransportPoolImpl(const BackupTransportPoolImpl &) = delete;
  BackupTransportPoolImpl &operator=(const BackupTransportPoolImpl &) = delete;

  LinkedList<Pointer<BackupTransport> > backups;
  volatile bool pending;
  volatile bool closed;
  volatile int priorityBackups;

  BackupTransportPoolImpl() : backups(), pending(false), closed(false), priorityBackups(0) {}
};
}  // namespace failover
}  // namespace transport
}  // namespace upmq

////////////////////////////////////////////////////////////////////////////////
BackupTransportPool::BackupTransportPool(FailoverTransport *parent_,
                                         Pointer<CompositeTaskRunner> taskRunner_,
                                         Pointer<CloseTransportsTask> closeTask_,
                                         Pointer<URIPool> uriPool_,
                                         Pointer<URIPool> updates_,
                                         Pointer<URIPool> priorityUriPool_)
    : impl(nullptr),
      parent(parent_),
      taskRunner(std::move(taskRunner_)),
      closeTask(std::move(closeTask_)),
      uriPool(std::move(uriPool_)),
      updates(std::move(updates_)),
      priorityUriPool(std::move(priorityUriPool_)),
      backupPoolSize(1),
      enabled(false),
      maxReconnectDelay(0) {
  if (this->parent == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Parent transport passed is NULL");
  }

  if (this->taskRunner == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "TaskRunner passed is NULL");
  }

  if (this->uriPool == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "URIPool passed is NULL");
  }

  if (this->priorityUriPool == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Piroirty URIPool passed is NULL");
  }

  if (this->closeTask == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Close Transport Task passed is NULL");
  }

  this->impl = new BackupTransportPoolImpl();

  // Add this instance as a Task so that we can create backups when nothing else is
  // going on.
  this->taskRunner->addTask(this);
}

////////////////////////////////////////////////////////////////////////////////
BackupTransportPool::BackupTransportPool(FailoverTransport *parent_,
                                         int backupPoolSize_,
                                         Pointer<CompositeTaskRunner> taskRunner_,
                                         Pointer<CloseTransportsTask> closeTask_,
                                         Pointer<URIPool> uriPool_,
                                         Pointer<URIPool> updates_,
                                         Pointer<URIPool> priorityUriPool_)
    : impl(nullptr),
      parent(parent_),
      taskRunner(std::move(taskRunner_)),
      closeTask(std::move(closeTask_)),
      uriPool(std::move(uriPool_)),
      updates(std::move(updates_)),
      priorityUriPool(std::move(priorityUriPool_)),
      backupPoolSize(backupPoolSize_),
      enabled(false),
      maxReconnectDelay(0) {
  if (this->parent == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Parent transport passed is NULL");
  }

  if (this->taskRunner == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "TaskRunner passed is NULL");
  }

  if (this->uriPool == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "URIPool passed is NULL");
  }

  if (this->priorityUriPool == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Piroirty URIPool passed is NULL");
  }

  if (this->closeTask == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Close Transport Task passed is NULL");
  }

  this->impl = new BackupTransportPoolImpl();

  // Add this instance as a Task so that we can create backups when nothing else is
  // going on.
  this->taskRunner->addTask(this);
}

////////////////////////////////////////////////////////////////////////////////
BackupTransportPool::~BackupTransportPool() {
  this->taskRunner->removeTask(this);

  try {
    delete this->impl;
  }
  AMQ_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void BackupTransportPool::close() {
  if (this->impl->closed) {
    return;
  }

  synchronized(&this->impl->backups) {
    this->enabled = false;
    this->impl->closed = true;
    this->impl->backups.clear();
  }
}

////////////////////////////////////////////////////////////////////////////////
void BackupTransportPool::setEnabled(bool value) {
  if (this->impl->closed) {
    return;
  }

  this->enabled = value;

  if (enabled == true) {
    this->taskRunner->wakeup();
  } else {
    synchronized(&this->impl->backups) { this->impl->backups.clear(); }
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<BackupTransport> BackupTransportPool::getBackup() {
  if (!isEnabled()) {
    throw IllegalStateException(__FILE__, __LINE__, "The Backup Pool is not enabled.");
  }

  Pointer<BackupTransport> result;

  synchronized(&this->impl->backups) {
    if (!this->impl->backups.isEmpty()) {
      result = this->impl->backups.removeAt(0);
    }
  }

  // Flag as pending so the task gets run again and new backups are created.
  this->impl->pending = true;
  this->taskRunner->wakeup();

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool BackupTransportPool::isPending() const {
  if (this->isEnabled()) {
    return this->impl->pending;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool BackupTransportPool::iterate() {
  LinkedList<URI> failures;

  synchronized(&this->impl->backups) {
    Pointer<URIPool> anUriPool = this->uriPool;

    // We prefer the Broker updated URIs list if it has any URIs.
    if (!updates->isEmpty()) {
      anUriPool = updates;
    }

    bool wakeupParent = false;

    while (isEnabled() && static_cast<int>(this->impl->backups.size()) < backupPoolSize) {
      URI connectTo;

      // Try for a URI, if one isn't free return and indicate this task
      // is done for now, the next time a backup is requested this task
      // will become pending again and we will attempt to fill the pool.
      // This will break the loop once we've tried all possible UIRs.
      try {
        connectTo = anUriPool->getURI();
      } catch (NoSuchElementException &) {
        break;
      }

      Pointer<BackupTransport> backup(new BackupTransport(this));
      backup->setUri(connectTo);

      if (priorityUriPool->contains(connectTo)) {
        backup->setPriority(true);

        if (!parent->isConnectedToPriority()) {
          wakeupParent = true;
        }
      }

      try {
        Pointer<Transport> transport = createTransport(connectTo);

        transport->setTransportListener(backup.get());
        transport->start();
        backup->setTransport(transport);

        // Put any priority connections first so a reconnect picks them
        // up automatically.
        if (backup->isPriority()) {
          this->impl->priorityBackups++;
          this->impl->backups.addFirst(backup);
        } else {
          this->impl->backups.addLast(backup);
        }
      } catch (...) {
        // Store it in the list of URIs that didn't work, once done we
        // return those to the pool.
        failures.add(connectTo);
      }

      // We connected to a priority backup and the parent isn't already using one
      // so wake it up and quick the backups process for now.
      if (wakeupParent) {
        this->parent->reconnect(true);
        break;
      }
    }

    // return all failures to the URI Pool, we can try again later.
    anUriPool->addURIs(failures);
    this->impl->pending = false;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void BackupTransportPool::onBackupTransportFailure(BackupTransport *failedTransport) {
  synchronized(&this->impl->backups) {
    std::unique_ptr<Iterator<Pointer<BackupTransport> > > iter(this->impl->backups.iterator());

    while (iter->hasNext()) {
      if (iter->next() == failedTransport) {
        iter->remove();
      }

      if (failedTransport->isPriority() && this->impl->priorityBackups > 0) {
        this->impl->priorityBackups--;
      }

      this->uriPool->addURI(failedTransport->getUri());
      this->closeTask->add(failedTransport->getTransport());
      this->taskRunner->wakeup();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool BackupTransportPool::isPriorityBackupAvailable() const {
  bool result = false;
  synchronized(&this->impl->backups) { result = this->impl->priorityBackups > 0; }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> BackupTransportPool::createTransport(const URI &location) const {
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
