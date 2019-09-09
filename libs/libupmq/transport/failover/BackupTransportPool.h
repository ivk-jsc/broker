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

#ifndef _UPMQ_BACKUP_TRANSPORT_POOL_H_
#define _UPMQ_BACKUP_TRANSPORT_POOL_H_

#include <threads/CompositeTask.h>
#include <threads/CompositeTaskRunner.h>
#include <transport/Config.h>
#include <transport/failover/BackupTransport.h>
#include <transport/failover/CloseTransportsTask.h>
#include <transport/failover/URIPool.h>

#include <decaf/io/IOException.h>
#include <decaf/lang/Pointer.h>
#include <decaf/util/LinkedList.h>

namespace upmq {
namespace transport {
namespace failover {

using decaf::lang::Pointer;
using decaf::util::LinkedList;
using upmq::threads::CompositeTaskRunner;

class BackupTransportPoolImpl;
class FailoverTransport;

class UPMQCPP_API BackupTransportPool : public upmq::threads::CompositeTask {
 private:
  friend class BackupTransport;

  BackupTransportPoolImpl *impl;

  FailoverTransport *parent;
  Pointer<CompositeTaskRunner> taskRunner;
  Pointer<CloseTransportsTask> closeTask;
  Pointer<URIPool> uriPool;
  Pointer<URIPool> updates;
  Pointer<URIPool> priorityUriPool;
  volatile int backupPoolSize;
  volatile bool enabled;
  volatile int maxReconnectDelay;

 public:
  BackupTransportPool(FailoverTransport *parent,
                      Pointer<CompositeTaskRunner> taskRunner,
                      Pointer<CloseTransportsTask> closeTask,
                      Pointer<URIPool> uriPool,
                      Pointer<URIPool> updates,
                      Pointer<URIPool> priorityUriPool);

  BackupTransportPool(FailoverTransport *parent,
                      int backupPoolSize,
                      Pointer<CompositeTaskRunner> taskRunner,
                      Pointer<CloseTransportsTask> closeTask,
                      Pointer<URIPool> uriPool,
                      Pointer<URIPool> updates,
                      Pointer<URIPool> priorityUriPool);

  ~BackupTransportPool() override;

  /**
   * Closes down the pool and destroys any Backups contained in the pool.
   */
  void close();

  /**
   * Return true if we don't currently have enough Connected Transports
   */
  bool isPending() const override;

  /**
   * Get a Connected Transport from the pool of Backups if any are present,
   * otherwise it return a NULL Pointer.
   *
   * @return Pointer to a Connected Transport or NULL
   */
  Pointer<BackupTransport> getBackup();

  /**
   * Connect to a Backup Broker if we haven't already connected to the max
   * number of Backups.
   */
  bool iterate() override;

  /**
   * Gets the Max number of Backups this Task will create.
   * @return the max number of active BackupTransports that will be created.
   */
  int getBackupPoolSize() const { return this->backupPoolSize; }

  /**
   * Sets the Max number of Backups this Task will create.
   * @param size - the max number of active BackupTransports that will be created.
   */
  void setBackupPoolSize(int size) { this->backupPoolSize = size; }

  /**
   * Gets if the backup Transport Pool has been enabled or not, when not enabled
   * no backups are created and any that were are destroyed.
   *
   * @return true if enable.
   */
  bool isEnabled() const { return this->enabled; }

  /**
   * Sets if this Backup Transport Pool is enabled.  When not enabled no Backups
   * are created and any that were are destroyed.
   *
   * @param value - true to enable backup creation, false to disable.
   */
  void setEnabled(bool value);

  /**
   * Returns true if there is a Backup in the pool that's on the priority
   * backups list.
   *
   * @return true if there is a priority backup available.
   */
  bool isPriorityBackupAvailable() const;

 private:
  // The backups report their failure to the pool, the pool removes them
  // from the list and returns their URIs to the URIPool, and then adds
  // the internal transport to the close transport's task for cleanup.
  void onBackupTransportFailure(BackupTransport *failedTransport);

  Pointer<Transport> createTransport(const decaf::net::URI &location) const;
};
}  // namespace failover
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_BACKUP_TRANSPORT_POOL_H_*/
