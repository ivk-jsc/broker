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

#ifndef _DECAF_INTERNAL_NET_TCP_TCPSOCKET_H_
#define _DECAF_INTERNAL_NET_TCP_TCPSOCKET_H_

#include <decaf/io/IOException.h>
#include <decaf/io/InputStream.h>
#include <decaf/io/OutputStream.h>
#include <decaf/lang/exceptions/IndexOutOfBoundsException.h>
#include <decaf/lang/exceptions/NullPointerException.h>
#include <decaf/net/SocketException.h>
#include <decaf/net/SocketImpl.h>
#include <decaf/net/SocketTimeoutException.h>
#include <decaf/util/Config.h>

namespace decaf {
namespace internal {
namespace net {
namespace tcp {

class TcpSocketInputStream;
class TcpSocketOutputStream;
class TcpSocketImpl;

/**
 * Platform-independent implementation of the socket interface.
 */
class DECAF_API TcpSocket : public decaf::net::SocketImpl {
 private:
  TcpSocketImpl *impl;

 public:
  TcpSocket(const TcpSocket &) = delete;
  TcpSocket &operator=(const TcpSocket &) = delete;
  /**
   * Construct a non-connected socket.
   *
   * @throws SocketException thrown if an error occurs while creating the Socket.
   */
  TcpSocket();

  /**
   * Releases the socket handle but not gracefully shut down the connection.
   */
  ~TcpSocket() override;

  /**
   * @return true if the socketHandle is not in a disconnected state.
   */
  bool isConnected() const;

  /**
   * @return true if the close method has been called on this Socket.
   */
  bool isClosed() const;

  /**
   * {@inheritDoc}
   */
  std::string getLocalAddress() const override;

  /**
   * {@inheritDoc}
   */
  void create() override;

  /**
   * {@inheritDoc}
   */
  void accept(SocketImpl *socket) override;

  /**
   * {@inheritDoc}
   */
  void bind(const std::string &ipaddress, int bindPort) override;

  /**
   * {@inheritDoc}
   */
  void connect(const std::string &hostname, int bindPort, int timeout) override;

  /**
   * {@inheritDoc}
   */
  void listen(int backlog) override;

  /**
   * {@inheritDoc}
   */
  decaf::io::InputStream *getInputStream() override;

  /**
   * {@inheritDoc}
   */
  decaf::io::OutputStream *getOutputStream() override;

  /**
   * {@inheritDoc}
   */
  int available() override;

  /**
   * {@inheritDoc}
   */
  void close() override;

  /**
   * {@inheritDoc}
   */
  void shutdownInput() override;

  /**
   * {@inheritDoc}
   */
  void shutdownOutput() override;

  /**
   * {@inheritDoc}
   */
  int getOption(int option) const override;

  /**
   * {@inheritDoc}
   */
  void setOption(int option, int value) override;

 public:
  /**
   * Reads the requested data from the Socket and write it into the passed in buffer.
   *
   * @param buffer
   *      The buffer to read into
   * @param size
   *      The size of the specified buffer
   * @param offset
   *      The offset into the buffer where reading should start filling.
   * @param length
   *      The number of bytes past offset to fill with data.
   *
   * @return the actual number of bytes read or -1 if at EOF.
   *
   * @throw IOException if an I/O error occurs during the read.
   * @throw NullPointerException if buffer is Null.
   * @throw IndexOutOfBoundsException if offset + length is greater than buffer size.
   */
  int read(unsigned char *buffer, int size, int offset, int length);

  /**
   * Writes the specified data in the passed in buffer to the Socket.
   *
   * @param buffer
   *      The buffer to write to the socket.
   * @param size
   *      The size of the specified buffer.
   * @param offset
   *      The offset into the buffer where the data to write starts at.
   * @param length
   *      The number of bytes past offset to write.
   *
   * @throw IOException if an I/O error occurs during the write.
   * @throw NullPointerException if buffer is Null.
   * @throw IndexOutOfBoundsException if offset + length is greater than buffer size.
   */
  void write(const unsigned char *buffer, int size, int offset, int length);

 protected:
  void checkResult(int value) const;
};
}  // namespace tcp
}  // namespace net
}  // namespace internal
}  // namespace decaf

#endif /*_DECAF_INTERNAL_NET_TCP_TCPSOCKET_H_*/
