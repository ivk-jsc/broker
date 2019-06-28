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

#ifndef _WIREFORMAT_H_
#define _WIREFORMAT_H_

#include <decaf/io/DataInputStream.h>
#include <decaf/io/DataOutputStream.h>
#include <decaf/io/IOException.h>
#include <decaf/lang/Pointer.h>

#include <transport/Command.h>
#include <transport/Config.h>
#include <transport/Transport.h>

namespace upmq {
namespace transport {

using decaf::lang::Pointer;

/**
 * Provides a mechanism to marshal commands into and out of packets
 * or into and out of streams, Channels and Datagrams.
 */
class UPMQCPP_API WireFormat {
 public:
  virtual ~WireFormat(){};

  /**
   * Stream based marshaling of a Command, this method blocks until the entire
   * Command has been written out to the output stream.
   *
   * @param command
   *      The Command to Marshal
   * @param transport
   *      The Transport that called this method.
   * @param out
   *      The output stream to write the command to.
   *
   * @throws IOException if an I/O error occurs.
   */
  virtual void marshal(const Pointer<transport::Command> command, const upmq::transport::Transport *transport, decaf::io::DataOutputStream *out) = 0;

  /**
   * Stream based unmarshaling, blocks on reads on the input stream until a complete
   * command has been read and unmarshaled into the correct form.  Returns a Pointer
   * to the newly unmarshaled Command.
   *
   * @param transport
   *      Pointer to the transport that is making this request.
   * @param in
   *      The input stream to read the command from.
   *
   * @return the newly marshaled Command, caller owns the pointer
   *
   * @throws IOException if an I/O error occurs.
   */
  virtual Pointer<transport::Command> unmarshal(const upmq::transport::Transport *transport, decaf::io::DataInputStream *in) = 0;
};
}  // namespace transport
}  // namespace upmq

#endif /*_WIREFORMAT_H_*/
