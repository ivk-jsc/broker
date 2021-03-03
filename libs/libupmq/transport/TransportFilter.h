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

#ifndef UPMQ_TRANSPORT_TRANSPORTFILTER_H_
#define UPMQ_TRANSPORT_TRANSPORTFILTER_H_

#include <decaf/lang/Pointer.h>
#include <transport/Command.h>
#include <transport/Config.h>
#include <transport/Transport.h>
#include <transport/TransportListener.h>
#include <transport/UPMQException.h>
#include <typeinfo>

namespace upmq {
namespace transport {

using decaf::lang::Pointer;
using upmq::transport::Command;
using upmq::transport::Response;

class TransportFilterImpl;

/**
 * A filter on the transport layer.  Transport filters implement the Transport
 * interface and optionally delegate calls to another Transport object.
 *
 * @since 1.0
 */
class UPMQCPP_API TransportFilter : public Transport, public TransportListener {
 private:
  TransportFilterImpl *impl;

 protected:
  /**
   * The transport that this filter wraps around.
   */
  Pointer<Transport> next;

  /**
   * Listener of this transport.
   */
  TransportListener *listener;

 private:
  TransportFilter(const TransportFilter &);
  TransportFilter &operator=(const TransportFilter &);

 public:
  /**
   * Constructor.
   * @param next - the next Transport in the chain
   */
  TransportFilter(Pointer<Transport> next);

  ~TransportFilter() override;

  void start() override;

  void stop() override;

  void close() override;

 protected:
  /**
   * Throws an IOException if this filter chain has already been closed.
   */
  void checkClosed() const;

 public:
  /**
   * Event handler for the receipt of a command.
   * @param command - the received command object.
   */
  void onCommand(Pointer<Command> command) override;

  /**
   * Event handler for an exception from a command transport.
   * @param ex
   *      The exception to handle.
   */
  void onException(const decaf::lang::Exception &ex) override;

  /**
   * The transport has suffered an interruption from which it hopes to recover
   */
  void transportInterrupted() override;

  /**
   * The transport has resumed after an interruption
   */
  void transportResumed() override;

 public:
  void oneway(Pointer<Command> command) override {
    checkClosed();
    next->oneway(std::move(command));
  }

  Pointer<FutureResponse> asyncRequest(Pointer<Command> command, Pointer<ResponseCallback> responseCallback) override {
    checkClosed();
    return next->asyncRequest(std::move(command), responseCallback);
  }

  Pointer<Response> request(Pointer<Command> command) override {
    checkClosed();
    return next->request(std::move(command));
  }

  Pointer<Response> request(Pointer<Command> command, unsigned int timeout) override {
    checkClosed();
    return next->request(std::move(command), timeout);
  }

  void setTransportListener(TransportListener *newListener) override { this->listener = newListener; }

  TransportListener *getTransportListener() const override { return this->listener; }

  Pointer<transport::WireFormat> getWireFormat() const override;

  void setWireFormat(Pointer<transport::WireFormat> wireFormat) override;

  Transport *narrow(const std::type_info &typeId) override;

  bool isFaultTolerant() const override { return !isClosed() && next->isFaultTolerant(); }

  bool isConnected() const override { return !isClosed() && next->isConnected(); }

  bool isReconnectSupported() const override { return !isClosed() && next->isReconnectSupported(); }

  bool isUpdateURIsSupported() const override { return !isClosed() && next->isUpdateURIsSupported(); }

  bool isClosed() const override;

  std::string getRemoteAddress() const override {
    if (isClosed()) {
      return "";
    }

    return next->getRemoteAddress();
  }

  virtual void reconnect();  // const decaf::net::URI &uri

  virtual void updateURIs() {  // bool rebalance, const decaf::util::List<decaf::net::URI> &uris
    checkClosed();
    // next->updateURIs(rebalance, uris);
  }

 protected:
  /**
   * Subclasses can override this method to do their own startup work.  This method
   * will always be called before the next transport in the chain is called in order
   * to allow this transport a chance to initialize required resources.
   */
  virtual void beforeNextIsStarted() {}

  /**
   * Subclasses can override this method to do their own post startup work.  This method
   * will always be called after the doStart() method and the next transport's own start()
   * methods have been successfully run.
   */
  virtual void afterNextIsStarted() {}

  /**
   * Subclasses can override this method to do their own pre-stop work.  This method
   * will always be called before the next transport's own stop() method or this filter's
   * own doStop() method is called.
   */
  virtual void beforeNextIsStopped() {}

  /**
   * Subclasses can override this method to do their own stop work.  This method is
   * always called after all the next transports have been stopped to prevent this
   * transport for destroying resources needed by the lower level transports.
   */
  virtual void afterNextIsStopped() {}

  /**
   * Subclasses can override this method to do their own close work.  This method is
   * always called after all the next transports have been closed to prevent this
   * transport for destroying resources needed by the lower level transports.
   */
  virtual void doClose() {}
};
}  // namespace transport
}  // namespace upmq

#endif /*UPMQMQ_TRANSPORT_TRANSPORTFILTER_H_*/
