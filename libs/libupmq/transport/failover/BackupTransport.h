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

#ifndef _UPMQ_BACKUP_TRANSPORT_H_
#define _UPMQ_BACKUP_TRANSPORT_H_

#include <decaf/lang/Pointer.h>
#include <decaf/net/URI.h>
#include <transport/Config.h>
#include <transport/DefaultTransportListener.h>
#include <transport/Transport.h>

namespace upmq {
namespace transport {
namespace failover {

using decaf::lang::Pointer;

class BackupTransportPool;

class UPMQCPP_API BackupTransport : public DefaultTransportListener {
 private:
  // The parent of this Backup
  BackupTransportPool *parent;

  // The Transport this one is managing.
  Pointer<Transport> transport;

  // The URI of this Backup
  decaf::net::URI uri;

  // Indicates that the contained transport is not valid any longer.
  bool closed;

  // Is this Transport one of the priority backups.
  bool priority;

 private:
  BackupTransport(const BackupTransport &);
  BackupTransport &operator=(const BackupTransport &);

 public:
  BackupTransport(BackupTransportPool *failover);

  ~BackupTransport() override;

  /**
   * Gets the URI assigned to this Backup
   * @return the assigned URI
   */
  decaf::net::URI getUri() const;

  /**
   * Sets the URI assigned to this Transport.
   */
  void setUri(const decaf::net::URI &newUri);

  /**
   * Gets the currently held transport
   * @return pointer to the held transport or NULL if not set.
   */
  const Pointer<Transport> &getTransport() const;

  /**
   * Sets the held transport, if not NULL then start to listen for exceptions
   * from the held transport.
   *
   * @param newTransport
   *        The transport to hold.
   */
  void setTransport(Pointer<Transport> newTransport);

  /**
   * Event handler for an exception from a command transport.
   * <p>
   * The BackupTransport closes its internal Transport when an exception is
   * received and returns the URI to the pool of URIs to attempt connections to.
   *
   * @param ex
   *      The exception that was passed to this listener to handle.
   */
  void onException(const decaf::lang::Exception &) override;

  /**
   * Has the Transport been shutdown and no longer usable.
   *
   * @return true if the Transport
   */
  bool isClosed() const;

  /**
   * Sets the closed flag on this Transport.
   * @param value - true for closed.
   */
  void setClosed(bool value);

  /**
   * @return true if this transport was in the priority backup list.
   */
  bool isPriority() const;

  /**
   * Set if this transport is a Priority backup or not.
   *
   * @param value
   *      True if this is a priority backup.
   */
  void setPriority(bool value);
};
}  // namespace failover
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_BACKUP_TRANSPORT_H_ */
