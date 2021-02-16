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

#include "IOTransport.h"

#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/concurrent/Concurrent.h>
#include <decaf/util/concurrent/atomic/AtomicBoolean.h>
#include <transport/Config.h>
#include <transport/UPMQException.h>
#include <transport/WireFormat.h>
#include <typeinfo>
#include <utility>

using namespace upmq;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::io;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::util::concurrent;
using namespace decaf::util::concurrent::atomic;

namespace upmq {
namespace transport {

class IOTransportImpl {
 public:
  IOTransportImpl(const IOTransportImpl &) = delete;
  IOTransportImpl &operator=(const IOTransportImpl &) = delete;

  Pointer<transport::WireFormat> wireFormat;
  TransportListener *listener;
  decaf::io::DataInputStream *inputStream;
  decaf::io::DataOutputStream *outputStream;
  Pointer<decaf::lang::Thread> thread;
  AtomicBoolean closed;
  AtomicBoolean started;

  IOTransportImpl() : wireFormat(), listener(nullptr), inputStream(nullptr), outputStream(nullptr), thread(), closed(false) {}

  IOTransportImpl(Pointer<WireFormat> wireFormat_)
      : wireFormat(std::move(wireFormat_)), listener(nullptr), inputStream(nullptr), outputStream(nullptr), thread(), closed(false) {}
};
}  // namespace transport
}  // namespace upmq

////////////////////////////////////////////////////////////////////////////////
IOTransport::IOTransport() : impl(new IOTransportImpl()) {}

////////////////////////////////////////////////////////////////////////////////
IOTransport::IOTransport(Pointer<WireFormat> wireFormat_) : impl(new IOTransportImpl(std::move(wireFormat_))) {}

////////////////////////////////////////////////////////////////////////////////
IOTransport::~IOTransport() {
  try {
    IOTransport::close();
  }
  AMQ_CATCHALL_NOTHROW()

  try {
    delete this->impl;
  }
  AMQ_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::fire(decaf::lang::Exception &ex) {
  if (this->impl->listener != nullptr && this->impl->started.get() && !this->impl->closed.get()) {
    try {
      this->impl->listener->onException(ex);
    } catch (...) {
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::fire(Pointer<Command> command) {
  try {
    // If we have been closed then we don't deliver any messages that
    // might have sneaked in while we where closing.
    if (this->impl->listener == nullptr || this->impl->closed.get()) {
      return;
    }

    this->impl->listener->onCommand(std::move(command));
  }
  AMQ_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::oneway(Pointer<Command> command) {
  try {
    if (impl->closed.get()) {
      throw IOException(__FILE__, __LINE__, "IOTransport::oneway() - transport is closed!");
    }

    // Make sure the thread has been started.
    if (impl->thread == nullptr) {
      throw IOException(__FILE__, __LINE__, "IOTransport::oneway() - transport is not started");
    }

    // Make sure the command object is valid.
    if (command == nullptr) {
      throw IOException(__FILE__, __LINE__, "IOTransport::oneway() - attempting to write NULL command");
    }

    // Make sure we have an output stream to write to.
    if (impl->outputStream == nullptr) {
      throw IOException(__FILE__, __LINE__, "IOTransport::oneway() - invalid output stream");
    }

    synchronized(impl->outputStream) {
      // Write the command to the output stream.
      this->impl->wireFormat->marshal(command, this, this->impl->outputStream);
      this->impl->outputStream->flush();
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::start() {
  try {
    if (impl->started.compareAndSet(false, true)) {
      if (impl->closed.get()) {
        throw IOException(__FILE__, __LINE__, "IOTransport::start() - transport is already closed - cannot restart");
      }

      // Make sure all variables that we need have been set.
      if (impl->inputStream == nullptr || impl->outputStream == nullptr || impl->wireFormat.get() == nullptr) {
        throw IOException(__FILE__,
                          __LINE__,
                          "IOTransport::start() - "
                          "IO streams and wireFormat instances must be set before calling start");
      }

      // Start the polling thread.
      impl->thread = makePointer<Thread>(this, "IOTransport reader Thread");
      impl->thread->start();
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::stop() {
  try {
    this->impl->started.set(false);
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::close() {
  class Finalizer {
   private:
    Pointer<Thread> target;

   public:
    Finalizer(Pointer<Thread> target_) : target(std::move(target_)) {}

    ~Finalizer() {
      try {
        target->join();
        target.reset(nullptr);
      }
      DECAF_CATCHALL_NOTHROW()
    }
  };

  try {
    // Mark this transport as closed.
    if (impl->closed.compareAndSet(false, true)) {
      Finalizer finalize(impl->thread);

      // No need to fire anymore async events now.
      this->impl->listener = nullptr;

      IOException error;
      bool hasException = false;

      // We have to close the input stream before we stop the thread.  this will
      // force us to wake up the thread if it's stuck in a read (which is likely).
      // Otherwise, the join that follows will block forever.
      try {
        if (impl->inputStream != nullptr) {
          impl->inputStream->close();
        }
      } catch (IOException &ex) {
        error = ex;
        error.setMark(__FILE__, __LINE__);
        hasException = true;
      }

      try {
        // Close the output stream.
        if (impl->outputStream != nullptr) {
          impl->outputStream->close();
        }
      } catch (IOException &ex) {
        if (!hasException) {
          error = ex;
          error.setMark(__FILE__, __LINE__);
          hasException = true;
        }
      }

      if (hasException) {
        throw error;
      }
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::run() {
  try {
    while (this->impl->started.get() && !this->impl->closed.get()) {
      // Read the next command from the input stream.
      Pointer<Command> command(impl->wireFormat->unmarshal(this, this->impl->inputStream));

      // Notify the listener.
      fire(command);
    }
  } catch (transport::UPMQException &ex) {
    ex.setMark(__FILE__, __LINE__);
    fire(ex);
  } catch (decaf::lang::Exception &ex) {
    transport::UPMQException exl(ex);
    exl.setMark(__FILE__, __LINE__);
    fire(exl);
  } catch (...) {
    transport::UPMQException ex(__FILE__, __LINE__, "IOTransport::run - caught unknown exception");
    fire(ex);
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<FutureResponse> IOTransport::asyncRequest(Pointer<Command> command UPMQCPP_UNUSED,
                                                  Pointer<ResponseCallback> responseCallback UPMQCPP_UNUSED) {
  DECAF_UNUSED_VAR(command);
  DECAF_UNUSED_VAR(responseCallback);
  throw UnsupportedOperationException(__FILE__, __LINE__, "IOTransport::asyncRequest() - unsupported operation");
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> IOTransport::request(Pointer<Command> command UPMQCPP_UNUSED) {
  DECAF_UNUSED_VAR(command);
  throw UnsupportedOperationException(__FILE__, __LINE__, "IOTransport::request() - unsupported operation");
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Response> IOTransport::request(Pointer<Command> command UPMQCPP_UNUSED, unsigned int timeout UPMQCPP_UNUSED) {
  DECAF_UNUSED_VAR(command);
  DECAF_UNUSED_VAR(timeout);
  throw UnsupportedOperationException(__FILE__, __LINE__, "IOTransport::request() - unsupported operation");
}

////////////////////////////////////////////////////////////////////////////////
void IOTransport::setInputStream(decaf::io::DataInputStream *is) { this->impl->inputStream = is; }

////////////////////////////////////////////////////////////////////////////////
void IOTransport::setOutputStream(decaf::io::DataOutputStream *os) { this->impl->outputStream = os; }

////////////////////////////////////////////////////////////////////////////////
Pointer<transport::WireFormat> IOTransport::getWireFormat() const { return this->impl->wireFormat; }

////////////////////////////////////////////////////////////////////////////////
void IOTransport::setWireFormat(Pointer<transport::WireFormat> wireFormat) { this->impl->wireFormat = std::move(wireFormat); }

////////////////////////////////////////////////////////////////////////////////
void IOTransport::setTransportListener(TransportListener *newListener) { this->impl->listener = newListener; }

////////////////////////////////////////////////////////////////////////////////
TransportListener *IOTransport::getTransportListener() const { return this->impl->listener; }

////////////////////////////////////////////////////////////////////////////////
bool IOTransport::isConnected() const { return !this->impl->closed.get(); }

////////////////////////////////////////////////////////////////////////////////
bool IOTransport::isClosed() const { return this->impl->closed.get(); }
