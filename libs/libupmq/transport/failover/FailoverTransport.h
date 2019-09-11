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

#ifndef _UPMQ_FAILOVER_TRANSPORT_H_
#define _UPMQ_FAILOVER_TRANSPORT_H_

#include <transport/Config.h>

#include <state/ConnectionStateTracker.h>
#include <threads/CompositeTaskRunner.h>
#include <transport/Command.h>
#include <transport/CompositeTransport.h>
#include <transport/Response.h>
#include <transport/Transport.h>
#include <transport/WireFormat.h>

#include <decaf/io/IOException.h>
#include <decaf/net/URI.h>
#include <decaf/util/List.h>
#include <decaf/util/Properties.h>

namespace upmq {
namespace transport {
namespace failover {

using namespace decaf::lang;
using namespace decaf::util;
using upmq::transport::Command;
using upmq::transport::Response;

class FailoverTransportListener;
class BackupTransportPool;
class FailoverTransportImpl;

class UPMQCPP_API FailoverTransport : public upmq::transport::CompositeTransport, public upmq::threads::CompositeTask {
 private:
  friend class FailoverTransportListener;
  friend class BackupTransportPool;

  state::ConnectionStateTracker stateTracker;

  FailoverTransportImpl *impl;

 public:
  FailoverTransport(const FailoverTransport &) = delete;
  FailoverTransport &operator=(const FailoverTransport &) = delete;

  FailoverTransport();

  ~FailoverTransport() override;

  /**
   * Indicates that the Transport needs to reconnect to another URI in its
   * list.
   *
   * @param rebalance
   *      Indicates if the current connection should be broken and reconnected.
   */
  void reconnect(bool rebalance);

  /**
   * Adds a New URI to the List of URIs this transport can Connect to.
   *
   * @param rebalance
   *      Should the transport reconnect to a different broker to balance load.
   * @param uri
   *      A String version of a URI to add to the URIs to failover to.
   */
  void add(bool rebalance, const std::string &uri);

 public:  // CompositeTransport methods
  void addURI(bool rebalance, const List<decaf::net::URI> &uris) override;

  void removeURI(bool rebalance, const List<decaf::net::URI> &uris) override;

 public:
  void start() override;

  void stop() override;

  void close() override;

  void oneway(Pointer<Command> command) override;

  Pointer<FutureResponse> asyncRequest(Pointer<Command> command, Pointer<ResponseCallback> responseCallback) override;

  Pointer<Response> request(Pointer<Command> command) override;

  Pointer<Response> request(Pointer<Command> command, unsigned int timeout) override;

  Pointer<WireFormat> getWireFormat() const override;

  void setWireFormat(Pointer<WireFormat> wireFormat UPMQCPP_UNUSED) override {}

  void setTransportListener(TransportListener *newListener) override;

  TransportListener *getTransportListener() const override;

  bool isFaultTolerant() const override { return true; }

  bool isConnected() const override;

  bool isClosed() const override;

  bool isInitialized() const;

  void setInitialized(bool value);

  Transport *narrow(const std::type_info &typeId) override;

  std::string getRemoteAddress() const override;

  virtual void reconnect(const decaf::net::URI &uri);

  virtual void updateURIs(bool rebalance, const decaf::util::List<decaf::net::URI> &uris);

 public:
  /**
   * @return true if there is a need for the iterate method to be called by this
   *          classes task runner.
   */
  bool isPending() const override;

  /**
   * Performs the actual Reconnect operation for the FailoverTransport, when a
   * connection is made this method returns false to indicate it doesn't need to
   * run again, otherwise it returns true to indicate its still trying to connect.
   *
   * @return false to indicate a connection, true to indicate it needs to try again.
   */
  bool iterate() override;

 public:
  long long getTimeout() const;

  void setTimeout(long long value);

  long long getInitialReconnectDelay() const;

  void setInitialReconnectDelay(long long value);

  long long getMaxReconnectDelay() const;

  void setMaxReconnectDelay(long long value);

  long long getBackOffMultiplier() const;

  void setBackOffMultiplier(long long value);

  bool isUseExponentialBackOff() const;

  void setUseExponentialBackOff(bool value);

  bool isRandomize() const;

  void setRandomize(bool value);

  int getMaxReconnectAttempts() const;

  void setMaxReconnectAttempts(int value);

  int getStartupMaxReconnectAttempts() const;

  void setStartupMaxReconnectAttempts(int value);

  long long getReconnectDelay() const;

  void setReconnectDelay(long long value);

  bool isBackup() const;

  void setBackup(bool value);

  int getBackupPoolSize() const;

  void setBackupPoolSize(int value);

  bool isTrackMessages() const;

  void setTrackMessages(bool value);

  bool isTrackTransactionProducers() const;

  void setTrackTransactionProducers(bool value);

  int getMaxCacheSize() const;

  void setMaxCacheSize(int value);

  int getMaxPullCacheSize() const;

  void setMaxPullCacheSize(int value);

  bool isReconnectSupported() const override;

  void setReconnectSupported(bool value);

  bool isUpdateURIsSupported() const override;

  void setUpdateURIsSupported(bool value);

  bool isRebalanceUpdateURIs() const;

  void setRebalanceUpdateURIs(bool rebalanceUpdateURIs);

  bool isPriorityBackup() const;

  void setPriorityBackup(bool priorityBackup);

  void setPriorityURIs(const std::string &priorityURIs);

  const decaf::util::List<decaf::net::URI> &getPriorityURIs() const;

  void setConnectionInterruptProcessingComplete(Pointer<Command> connectionId);

  bool isConnectedToPriority() const;

 protected:
  /**
   * Given a Transport restore the state of the Client's connection to the Broker
   * using the data accumulated in the State Tracker.
   *
   * @param transport
   *        The new Transport connected to the Broker.
   *
   * @throw IOException if an errors occurs while restoring the old state.
   */
  void restoreTransport(const Pointer<Transport> &transport);

  /**
   * Called when this class' TransportListener is notified of a Failure.
   * @param error - The CMS Exception that was thrown.
   * @throw Exception if an error occurs.
   */
  void handleTransportFailure(const decaf::lang::Exception &error);

  /**
   * Called when the Broker sends a ConnectionControl command which could
   * signal that this Client needs to reconnect in order to rebalance the
   * connections on a Broker or the set of Known brokers has changed.
   *
   * @param control
   *      The ConnectionControl command sent from the Broker.
   */
  void handleConnectionControl(Pointer<Command> control);

 private:
  /**
   * Looks up the correct Factory and create a new Composite version of the
   * Transport requested.
   *
   * @param location - The URI to connect to
   *
   * @throw IOException if an I/O error occurs while creating the new Transport.
   */
  Pointer<Transport> createTransport(const decaf::net::URI &location) const;

  void processNewTransports(bool rebalance, std::string newTransports);

  void processResponse(const Pointer<Response> &response);
};
}  // namespace failover
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_FAILOVER_TRANSPORT_H_ */
