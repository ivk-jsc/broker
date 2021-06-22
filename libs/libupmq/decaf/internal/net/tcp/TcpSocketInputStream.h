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

#ifndef _DECAF_INTERNAL_NET_TCP_TCPSOCKETINPUTSTREAM_H_
#define _DECAF_INTERNAL_NET_TCP_TCPSOCKETINPUTSTREAM_H_

#include <decaf/util/Config.h>

#include <decaf/io/IOException.h>
#include <decaf/io/InputStream.h>
#include <decaf/lang/exceptions/IndexOutOfBoundsException.h>
#include <decaf/lang/exceptions/NullPointerException.h>

namespace decaf {
namespace internal {
namespace net {
namespace tcp {

class TcpSocket;

/**
 * Input stream for performing reads on a socket.  This class will only
 * work properly for blocking sockets.
 *
 * @since 1.0
 */
class DECAF_API TcpSocketInputStream : public decaf::io::InputStream {
 private:
  TcpSocket *socket;

  volatile bool closed;

 private:
  TcpSocketInputStream(const TcpSocketInputStream &);
  TcpSocketInputStream &operator=(const TcpSocketInputStream &);

 public:
  /**
   * Create a new InputStream to use for reading from the TCP/IP socket.
   *
   * @param socket
   *      The parent SocketImpl for this stream.
   */
  TcpSocketInputStream(TcpSocket *socket_);

  ~TcpSocketInputStream() override;

  /**
   * {@inheritDoc}
   */
  int available() const override;

  /**
   * Close - does nothing.  It is the responsibility of the owner
   * of the socket object to close it.
   *
   * {@inheritDoc}
   */
  void close() override;

  /**
   * Not supported.
   *
   * {@inheritDoc}
   */
  long long skip(long long num) override;

 protected:
  int doReadByte() override;

  int doReadArrayBounded(unsigned char *pbuffer, int size, int offset, int length) override;
};
}  // namespace tcp
}  // namespace net
}  // namespace internal
}  // namespace decaf

#endif /*_DECAF_INTERNAL_NET_TCP_TCPSOCKETINPUTSTREAM_H_*/
