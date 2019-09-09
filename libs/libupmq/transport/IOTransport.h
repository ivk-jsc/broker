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

#ifndef _UPMQ_TRANSPORT_IOTRANSPORT_H_
#define _UPMQ_TRANSPORT_IOTRANSPORT_H_

#include <transport/Command.h>
#include <transport/Config.h>
#include <transport/Response.h>
#include <transport/Transport.h>
#include <transport/TransportListener.h>
#include <transport/WireFormat.h>

#include <decaf/io/DataInputStream.h>
#include <decaf/io/DataOutputStream.h>
#include <decaf/lang/Runnable.h>
#include <decaf/lang/Thread.h>
//#include <decaf/util/logging/LoggerDefines.h>

namespace upmq {
namespace transport {

using decaf::lang::Pointer;
using upmq::transport::Command;
using upmq::transport::Response;

class IOTransportImpl;

/**
 * Implementation of the Transport interface that performs marshaling of commands
 * to IO streams.
 *
 * This class does not implement the Transport::request method, it only handles
 * oneway messages.  A thread polls on the input stream for in-coming commands.  When
 * a command is received, the command listener is notified.  The polling thread is not
 * started until the start method is called.  Polling can be suspending by calling stop;
 * however, because the read operation is blocking the transport my still pull one command
 * off the wire even after the stop method has been called.
 *
 * The close method will close the associated
 * streams.  Close can be called explicitly by the user, but is also called in the
 * destructor.  Once this object has been closed, it cannot be restarted.
 */
class UPMQCPP_API IOTransport : public Transport, public decaf::lang::Runnable {
  // LOGDECAF_DECLARE(logger)

 private:
  IOTransportImpl *impl;

 private:
  IOTransport(const IOTransport &);
  IOTransport &operator=(const IOTransport &);

 private:
  /**
   * Notify the exception listener
   *
   * @param ex
   *      The exception to send to any registered listener.
   */
  void fire(decaf::lang::Exception &ex);

  /**
   * Notify the command listener.
   *
   * @param
   *      The command the command the send to any registered listener.
   */
  void fire(Pointer<Command> command);

 public:
  /**
   * Default Constructor
   */
  IOTransport();

  /**
   * Create an instance of this Transport and assign its WireFormat instance
   * at creation time.
   *
   * @param wireFormat
   *        Data encoder / decoder to use when reading and writing.
   */
  IOTransport(Pointer<transport::WireFormat> wireFormat);

  ~IOTransport() override;

  /**
   * Sets the stream from which this Transport implementation will read its data.
   *
   * @param is
   *      The InputStream that will be read from by this object.
   */
  virtual void setInputStream(decaf::io::DataInputStream *is);

  /**
   * Sets the stream to which this Transport implementation will write its data.
   *
   * @param os
   *      The OuputStream that will be written to by this object.
   */
  virtual void setOutputStream(decaf::io::DataOutputStream *os);

 public:  // Transport methods
  void oneway(Pointer<Command> command) override;

  /**
   * {@inheritDoc}
   *
   * This method always thrown an UnsupportedOperationException.
   */
  Pointer<FutureResponse> asyncRequest(Pointer<Command> command, Pointer<ResponseCallback> responseCallback) override;

  /**
   * {@inheritDoc}
   *
   * This method always thrown an UnsupportedOperationException.
   */
  Pointer<Response> request(Pointer<Command> command) override;

  /**
   * {@inheritDoc}
   *
   * This method always thrown an UnsupportedOperationException.
   */
  Pointer<Response> request(Pointer<Command> command, unsigned int timeout) override;

  Pointer<transport::WireFormat> getWireFormat() const override;

  void setWireFormat(Pointer<transport::WireFormat> wireFormat) override;

  void setTransportListener(TransportListener *newListener) override;

  TransportListener *getTransportListener() const override;

  void start() override;

  void stop() override;

  void close() override;

  Transport *narrow(const std::type_info &typeId) override {
    if (typeid(*this) == typeId) {
      return this;
    }

    return nullptr;
  }

  bool isFaultTolerant() const override { return false; }

  bool isConnected() const override;

  bool isClosed() const override;

  std::string getRemoteAddress() const override { return ""; }

  bool isReconnectSupported() const override { return false; }

  bool isUpdateURIsSupported() const override { return false; }

  // virtual void updateURIs(bool rebalance UPMQCPP_UNUSED, const decaf::util::List<decaf::net::URI> &uris
  // UPMQCPP_UNUSED) {
  //  throw decaf::io::IOException();
  //}

  /**
   * {@inheritDoc}
   *
   * This method does nothing in this subclass.
   */
  // virtual void reconnect(const decaf::net::URI &uri UPMQCPP_UNUSED) {}

 public:  // Runnable methods.
  void run() override;
};
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_TRANSPORT_IOTRANSPORT_H_*/
