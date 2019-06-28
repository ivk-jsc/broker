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

#include "TcpSocket.h"
#include <fake_cpp14.h>
#include <decaf/internal/net/SocketFileDescriptor.h>
#include <decaf/internal/net/tcp/TcpSocketInputStream.h>
#include <decaf/internal/net/tcp/TcpSocketOutputStream.h>

#include <decaf/lang/Character.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/net/SocketError.h>
#include <decaf/net/SocketOptions.h>
#include <decaf/util/concurrent/atomic/AtomicBoolean.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "decaf/lang/exceptions/ClassCastException.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SocketDefs.h>
#include <Poco/Error.h>

#if !defined(HAVE_WINSOCK2_H)
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <Winsock2.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#define BSD_COMP /* Get FIONREAD on Solaris2. */
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// Pick up FIONREAD on Solaris 2.5.
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

using namespace decaf;
using namespace decaf::internal;
using namespace decaf::internal::net;
using namespace decaf::internal::net::tcp;
using namespace decaf::net;
using namespace decaf::io;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::util::concurrent::atomic;

////////////////////////////////////////////////////////////////////////////////
namespace decaf {
namespace internal {
namespace net {
namespace tcp {

class TcpSocketImpl {
 public:
  std::unique_ptr<Poco::Net::StreamSocket> socketHandle;
  bool handleIsRemote;
  std::unique_ptr<Poco::Net::SocketAddress> localAddress;
  std::unique_ptr<Poco::Net::SocketAddress> remoteAddress;
  TcpSocketInputStream *inputStream;
  TcpSocketOutputStream *outputStream;
  bool inputShutdown;
  bool outputShutdown;
  AtomicBoolean closed;
  bool connected;
  int trafficClass;
  int soTimeout;
  int soLinger;

  TcpSocketImpl()
      : socketHandle(nullptr),
        handleIsRemote(false),
        localAddress(nullptr),
        remoteAddress(nullptr),
        inputStream(nullptr),
        outputStream(nullptr),
        inputShutdown(false),
        outputShutdown(false),
        closed(false),
        connected(false),
        trafficClass(0),
        soTimeout(-1),
        soLinger(-1) {}
};
}  // namespace tcp
}  // namespace net
}  // namespace internal
}  // namespace decaf

////////////////////////////////////////////////////////////////////////////////
TcpSocket::TcpSocket() : impl(new TcpSocketImpl) {}

////////////////////////////////////////////////////////////////////////////////
TcpSocket::~TcpSocket() {
  try {
    TcpSocket::close();
  }
  DECAF_CATCHALL_NOTHROW()

  try {
    if (this->impl->inputStream != nullptr) {
      delete this->impl->inputStream;
      this->impl->inputStream = nullptr;
    }
  }
  DECAF_CATCHALL_NOTHROW()

  try {
    if (this->impl->outputStream != nullptr) {
      delete this->impl->outputStream;
      this->impl->outputStream = nullptr;
    }
  }
  DECAF_CATCHALL_NOTHROW()

  try {
    if (!this->impl->handleIsRemote && this->impl->socketHandle != nullptr) {
      this->impl->socketHandle->close();
    }

    delete this->impl;
  }
  DECAF_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::create() {
  try {
    if (this->impl->socketHandle != nullptr) {
      throw IOException(__FILE__, __LINE__, "The System level socket has already been created.");
    }

    // Create the actual socket.
    this->impl->socketHandle = std::make_unique<Poco::Net::StreamSocket>(Poco::Net::SocketAddress::Family::IPv4);

    // Initialize the Socket's FileDescriptor
    this->fd = new SocketFileDescriptor(static_cast<long>(this->impl->socketHandle->impl()->sockfd()));
  }
  DECAF_CATCH_RETHROW(decaf::io::IOException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, decaf::io::IOException)
  DECAF_CATCHALL_THROW(decaf::io::IOException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::accept(SocketImpl *socket) {
  try {
    if (socket == nullptr) {
      throw IOException(__FILE__, __LINE__, "SocketImpl instance passed was null.");
    }

    TcpSocket *tcpSocket = dynamic_cast<TcpSocket *>(socket);
    if (tcpSocket == nullptr) {
      throw ClassCastException(__FILE__, __LINE__, "can't cast to TcpSocket.");
    }
    if (impl == nullptr) {
      throw IOException(__FILE__, __LINE__, "SocketImpl instance passed was not a TcpSocket.");
    }

    tcpSocket->impl->handleIsRemote = true;

    // Loop to ignore any signal interruptions that occur during the operation.
    Poco::Net::SocketAddress sa;
    Poco::Net::SocketImpl *socketImpl = nullptr;
    try {
      socketImpl = this->impl->socketHandle->impl()->acceptConnection(sa);
      tcpSocket->impl->socketHandle = std::make_unique<Poco::Net::StreamSocket>(socketImpl);
    } catch (Poco::Exception &pex) {
      throw SocketException(__FILE__, __LINE__, "ServerSocket::accept - %s", pex.what());
    }

    // the socketHandle will have been allocated in the apr_pool of the ServerSocket.
    tcpSocket->impl->connected = true;
  }
  DECAF_CATCH_RETHROW(decaf::io::IOException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, decaf::io::IOException)
  DECAF_CATCHALL_THROW(decaf::io::IOException)
}

////////////////////////////////////////////////////////////////////////////////
InputStream *TcpSocket::getInputStream() {
  if (this->impl->socketHandle == nullptr || isClosed()) {
    throw IOException(__FILE__, __LINE__, "The Socket is not Connected.");
  }

  if (this->impl->inputShutdown) {
    throw IOException(__FILE__, __LINE__, "Input has been shut down on this Socket.");
  }

  try {
    if (this->impl->inputStream == nullptr) {
      this->impl->inputStream = new TcpSocketInputStream(this);
    }

    return impl->inputStream;
  }
  DECAF_CATCH_RETHROW(decaf::io::IOException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, decaf::io::IOException)
  DECAF_CATCHALL_THROW(decaf::io::IOException)
}

////////////////////////////////////////////////////////////////////////////////
OutputStream *TcpSocket::getOutputStream() {
  if (this->impl->socketHandle == nullptr || isClosed()) {
    throw IOException(__FILE__, __LINE__, "The Socket is not Connected.");
  }

  if (this->impl->outputShutdown) {
    throw IOException(__FILE__, __LINE__, "Output has been shut down on this Socket.");
  }

  try {
    if (this->impl->outputStream == nullptr) {
      this->impl->outputStream = new TcpSocketOutputStream(this);
    }

    return impl->outputStream;
  }
  DECAF_CATCH_RETHROW(decaf::io::IOException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, decaf::io::IOException)
  DECAF_CATCHALL_THROW(decaf::io::IOException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::bind(const std::string &ipaddress, int bindPort) {
  try {
    // Create the Address Info for the Socket
    if (ipaddress.empty() && bindPort == 0) {
      impl->localAddress = std::make_unique<Poco::Net::SocketAddress>();
    } else {
      impl->localAddress = std::make_unique<Poco::Net::SocketAddress>(ipaddress, static_cast<Poco::UInt16>(bindPort));
    }

    // Set the socket to reuse the address and default as blocking with no timeout.
    impl->socketHandle->setReuseAddress(true);
    impl->socketHandle->setBlocking(true);

    // Only incur the overhead of a lookup if we don't already know the local port.
    if (bindPort != 0) {
      // Bind to the Socket, this may be where we find out if the port is in use.
      try {
        impl->socketHandle->impl()->bind(*impl->localAddress);
      } catch (Poco::Exception &pex) {
        close();
        throw SocketException(__FILE__, __LINE__, "ServerSocket::bind - %s", pex.what());
      }
      this->localPort = bindPort;
    } else {
      this->localPort = 0;
    }
  }
  DECAF_CATCH_RETHROW(decaf::io::IOException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, decaf::io::IOException)
  DECAF_CATCHALL_THROW(decaf::io::IOException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::connect(const std::string &hostname, int bindPort, int timeout) {
  try {
    if (bindPort < 0 || bindPort > 65535) {
      throw IllegalArgumentException(__FILE__, __LINE__, "Given port is out of range: %d", bindPort);
    }

    if (this->impl->socketHandle == nullptr) {
      throw IOException(__FILE__, __LINE__, "The socket was not yet created.");
    }

    // Create the Address data
    impl->remoteAddress = std::make_unique<Poco::Net::SocketAddress>(Poco::Net::AddressFamily::IPv4, hostname, static_cast<Poco::UInt16>(bindPort));

    bool oldNonblockSetting = 0;
    Poco::Timespan oldRecvTimeoutSetting = 0;
    Poco::Timespan oldSendTimeoutSetting = 0;

    // Record the old settings.
    oldNonblockSetting = impl->socketHandle->getBlocking();
    oldRecvTimeoutSetting = impl->socketHandle->getReceiveTimeout();
    oldSendTimeoutSetting = impl->socketHandle->getSendTimeout();

    // Temporarily make it what we want, blocking.
    impl->socketHandle->setBlocking(true);

    // Timeout and non-timeout case require very different logic.
    if (timeout > 0) {
      impl->socketHandle->setReceiveTimeout(Poco::Timespan(timeout, 0));
      impl->socketHandle->setSendTimeout(Poco::Timespan(timeout, 0));
    }

    // try to Connect to the provided address.
    impl->socketHandle->connect(*impl->remoteAddress);

    // Now that we are connected, we want to go back to old settings.
    impl->socketHandle->setBlocking(oldNonblockSetting);
    impl->socketHandle->setReceiveTimeout(oldRecvTimeoutSetting);
    impl->socketHandle->setSendTimeout(oldSendTimeoutSetting);

    // Now that we connected, cache the port value for later lookups.
    this->port = bindPort;
    this->impl->connected = true;

  } catch (IOException &ex) {
    ex.setMark(__FILE__, __LINE__);
    try {
      close();
    } catch (lang::Exception &) { /* Absorb */
    }
    throw;
  } catch (IllegalArgumentException &ex) {
    ex.setMark(__FILE__, __LINE__);
    try {
      close();
    } catch (lang::Exception &) { /* Absorb */
    }
    throw;
  } catch (Exception &ex) {
    try {
      close();
    } catch (lang::Exception &) { /* Absorb */
    }
    throw SocketException(ex.clone());
  } catch (...) {
    try {
      close();
    } catch (lang::Exception &) { /* Absorb */
    }
    throw SocketException(__FILE__, __LINE__, "TcpSocket::connect() - caught unknown exception");
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string TcpSocket::getLocalAddress() const {
  if (!isClosed()) {
    return this->impl->socketHandle->address().host().toString();
  }

  return "0.0.0.0";
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::listen(int backlog) {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "The stream is closed");
    }

    // Setup the listen for incoming connection requests
    try {
      impl->socketHandle->impl()->listen(backlog);
    } catch (Poco::Exception &pex) {
      close();
      throw SocketException(__FILE__, __LINE__, "Error on Bind - %s", pex.what());
    }
  }
  DECAF_CATCH_RETHROW(decaf::io::IOException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, decaf::io::IOException)
  DECAF_CATCHALL_THROW(decaf::io::IOException)
}

////////////////////////////////////////////////////////////////////////////////
int TcpSocket::available() {
  if (isClosed()) {
    throw IOException(__FILE__, __LINE__, "The stream is closed");
  }

  // Convert to an OS level socket.

// The windows version
#if defined(HAVE_WINSOCK2_H)

  unsigned long numBytes = 0;

  if (::ioctlsocket(impl->socketHandle->impl()->sockfd(), FIONREAD, &numBytes) == SOCKET_ERROR) {
    throw SocketException(__FILE__, __LINE__, "ioctlsocket failed");
  }

  return numBytes;

#else  // !defined(HAVE_WINSOCK2_H)
// If FIONREAD is defined - use ioctl to find out how many bytes
// are available.
#if defined(FIONREAD)

  int numBytes = 0;
  if (::ioctl(impl->socketHandle->impl()->sockfd(), FIONREAD, &numBytes) != -1) {
    return numBytes;
  }

#endif

// If we didn't get anything we can use select.  This is a little
// less functional.  We will poll on the socket - if there is data
// available, we'll return 1, otherwise we'll return zero.
#if defined(HAVE_SELECT)

  fd_set rd;
  FD_ZERO(&rd);
  FD_SET(oss, &rd);
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  int returnCode = ::select(impl->socketHandle->impl()->sockfd() + 1, &rd, nullptr, nullptr, &tv);
  if (returnCode == -1) {
    throw IOException(__FILE__, __LINE__, SocketError::getErrorString().c_str());
  }
  return (returnCode == 0) ? 0 : 1;

#else

  return 0;

#endif /* HAVE_SELECT */

#endif  // !defined(HAVE_WINSOCK2_H)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::close() {
  try {
    if (this->impl->closed.compareAndSet(false, true)) {
      this->impl->connected = false;

      // Destroy the input stream.
      if (impl->inputStream != nullptr) {
        impl->inputStream->close();
      }

      // Destroy the output stream.
      if (impl->outputStream != nullptr) {
        impl->outputStream->close();
      }

      // When connected we first shutdown, which breaks our reads and writes
      // then we close to free APR resources.
      if (this->impl->socketHandle != nullptr) {
        impl->socketHandle->impl()->shutdownReceive();
        impl->socketHandle->impl()->shutdownSend();

        // Member data from parent
        delete this->fd;
        this->port = 0;
        this->localPort = 0;
      }
    }
  }
  DECAF_CATCH_RETHROW(decaf::io::IOException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, decaf::io::IOException)
  DECAF_CATCHALL_THROW(decaf::io::IOException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::shutdownInput() {
  if (isClosed()) {
    throw IOException(__FILE__, __LINE__, "The stream is closed");
  }

  this->impl->inputShutdown = true;
  impl->socketHandle->impl()->shutdownReceive();
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::shutdownOutput() {
  if (isClosed()) {
    throw IOException(__FILE__, __LINE__, "The stream is closed");
  }

  this->impl->outputShutdown = true;
  impl->socketHandle->impl()->shutdownSend();
}

////////////////////////////////////////////////////////////////////////////////
int TcpSocket::getOption(int option) const {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "The Socket is closed.");
    }

    if (option == SocketOptions::SOCKET_OPTION_TIMEOUT) {
      // Time in APR on socket is stored in microseconds.
      return impl->socketHandle->getSendTimeout().seconds();
    } else if (option == SocketOptions::SOCKET_OPTION_LINGER) {
      bool lingerOn = false;
      int sec = 0;
      impl->socketHandle->getLinger(lingerOn, sec);

      // In case the socket linger is on by default we reset to match,
      // we just use one since we really don't know what the linger time is
      // with APR.
      if (lingerOn && this->impl->soLinger == -1) {
        this->impl->soLinger = 1;
      }

      return this->impl->soLinger;
    }
    int value = 0;
    if (option == SocketOptions::SOCKET_OPTION_REUSEADDR) {
      value = impl->socketHandle->getReuseAddress();
    } else if (option == SocketOptions::SOCKET_OPTION_SNDBUF) {
      value = impl->socketHandle->getSendBufferSize();
    } else if (option == SocketOptions::SOCKET_OPTION_RCVBUF) {
      value = impl->socketHandle->getReceiveBufferSize();
    } else if (option == SocketOptions::SOCKET_OPTION_TCP_NODELAY) {
      value = impl->socketHandle->getNoDelay();
    } else if (option == SocketOptions::SOCKET_OPTION_KEEPALIVE) {
      value = impl->socketHandle->getKeepAlive();
    } else {
      throw IOException(__FILE__, __LINE__, "Socket Option is not valid for this Socket type.");
    }

    return value;
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::setOption(int option, int value) {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "The Socket is closed.");
    }

    if (option == SocketOptions::SOCKET_OPTION_TIMEOUT) {
      impl->socketHandle->setBlocking(true);
      // Time in APR for sockets is in microseconds so multiply by 1000.
      impl->socketHandle->setReceiveTimeout(Poco::Timespan(value, 0));
      impl->socketHandle->setSendTimeout(Poco::Timespan(value, 0));
      this->impl->soTimeout = value;
      return;
    } else if (option == SocketOptions::SOCKET_OPTION_LINGER) {
      // Store the real setting for later.
      this->impl->soLinger = value;

      // Now use the APR API to set it to the boolean state that APR expects
      impl->socketHandle->setLinger((value > 0), 30);
      return;
    }

    if (option == SocketOptions::SOCKET_OPTION_REUSEADDR) {
      impl->socketHandle->setReuseAddress(value > 0);
    } else if (option == SocketOptions::SOCKET_OPTION_SNDBUF) {
      impl->socketHandle->setSendBufferSize(value);
    } else if (option == SocketOptions::SOCKET_OPTION_RCVBUF) {
      impl->socketHandle->setReceiveBufferSize(value);
    } else if (option == SocketOptions::SOCKET_OPTION_TCP_NODELAY) {
      impl->socketHandle->setNoDelay(value > 0);
    } else if (option == SocketOptions::SOCKET_OPTION_KEEPALIVE) {
      impl->socketHandle->setKeepAlive(value > 0);
    } else {
      throw IOException(__FILE__, __LINE__, "Socket Option is not valid for this Socket type.");
    }
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::checkResult(int value) const {
  if (value != 0) {
    throw SocketException(__FILE__, __LINE__, SocketError::getErrorString().c_str());
  }
}

////////////////////////////////////////////////////////////////////////////////
int TcpSocket::read(unsigned char *buffer, int size, int offset, int length) {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "The Stream has been closed");
    }

    if (this->impl->inputShutdown) {
      return -1;
    }

    if (length == 0) {
      return 0;
    }

    if (buffer == nullptr) {
      throw NullPointerException(__FILE__, __LINE__, "Buffer passed is Null");
    }

    if (size < 0) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "size parameter out of Bounds: %d.", size);
    }

    if (offset > size || offset < 0) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "offset parameter out of Bounds: %d.", offset);
    }

    if (length < 0 || length > size - offset) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "length parameter out of Bounds: %d.", length);
    }

    int aprSize = length;
    ptrdiff_t result = 0;

    // Read data from the socket, size on input is size of buffer, when done
    // size is the number of bytes actually read, can be <= bufferSize.

    int recvSize = 0;
    int currentRecvSize = 0;
    unsigned char *lbuffer = buffer + offset;

    try {
      currentRecvSize = impl->socketHandle->receiveBytes(&lbuffer[recvSize], aprSize - recvSize);
    } catch (Poco::TimeoutException &) {
      if (!impl->socketHandle->getBlocking()) {
        result = 0;

      } else {
        result = -1;
      }
    } catch (Poco::Net::InvalidSocketException &) {
      result = -1;
    }
    if (currentRecvSize < 0) {
      int err = Poco::Error::last();
      if (err == POCO_EAGAIN || err == POCO_EWOULDBLOCK) {
        result = 0;
      } else {
        result = -1;
      }
    }
    recvSize += currentRecvSize;

    // Check for EOF, on windows we only get size==0 so check that to, if we
    // were closed though then we throw an IOException so the caller knows we
    // aren't usable anymore.
    if (((result < 0) || recvSize == 0) && !isClosed()) {
      this->impl->inputShutdown = true;
      return -1;
    }

    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "The connection is closed");
    }

    if (result != 0) {
      throw IOException(__FILE__, __LINE__, "Socket Read Error - %s", SocketError::getErrorString().c_str());
    }

    return recvSize;
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCH_RETHROW(NullPointerException)
  DECAF_CATCH_RETHROW(IndexOutOfBoundsException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpSocket::write(const unsigned char *buffer, int size, int offset, int length) {
  try {
    if (length == 0) {
      return;
    }

    if (buffer == nullptr) {
      throw NullPointerException(__FILE__, __LINE__, "TcpSocket::write - passed buffer is null");
    }

    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "TcpSocket::write - This Stream has been closed.");
    }

    if (size < 0) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "size parameter out of Bounds: %d.", size);
    }

    if (offset > size || offset < 0) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "offset parameter out of Bounds: %d.", offset);
    }

    if (length < 0 || length > size - offset) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "length parameter out of Bounds: %d.", length);
    }

    int remaining = length;

    const unsigned char *lbuffer = buffer + offset;

    int sent = 0;
    int currentSent;
    do {
      try {
        currentSent = impl->socketHandle->sendBytes(&lbuffer[sent], (remaining - sent), MSG_NOSIGNAL);
      } catch (Poco::TimeoutException &tex) {
        if (!impl->socketHandle->getBlocking()) {
          continue;
        }
        throw IOException(__FILE__, __LINE__, "TcpSocketOutputStream::timeout - %s", tex.what());
      } catch (Poco::Net::InvalidSocketException &ise) {
        throw IOException(__FILE__, __LINE__, "TcpSocketOutputStream::write - %s", ise.what());
      }

      if (currentSent < 0) {
        int err = Poco::Error::last();
        if (err == POCO_EAGAIN || err == POCO_EWOULDBLOCK) {
          continue;
        }
        if (err != 0 || isClosed()) {
          throw IOException(__FILE__, __LINE__, "TcpSocketOutputStream::write - %s", Poco::Error::getMessage(err).c_str());
        }
      }
      sent += currentSent;
    } while (sent < remaining);
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCH_RETHROW(NullPointerException)
  DECAF_CATCH_RETHROW(IndexOutOfBoundsException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
bool TcpSocket::isConnected() const { return this->impl->connected; }

////////////////////////////////////////////////////////////////////////////////
bool TcpSocket::isClosed() const { return this->impl->closed.get(); }
