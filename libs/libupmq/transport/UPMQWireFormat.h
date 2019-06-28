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

#ifndef _UPMQWIREFORMAT_H_
#define _UPMQWIREFORMAT_H_

#include <decaf/lang/Pointer.h>
#include <transport/WireFormat.h>
#include <memory>

namespace upmq {
namespace transport {

using decaf::lang::Pointer;

class UPMQCPP_API UPMQWireFormat : public transport::WireFormat {
 public:
  UPMQWireFormat();
  virtual ~UPMQWireFormat();

  virtual void marshal(const Pointer<transport::Command> command, const upmq::transport::Transport *transport, decaf::io::DataOutputStream *out);
  virtual Pointer<transport::Command> unmarshal(const upmq::transport::Transport *transport, decaf::io::DataInputStream *in);
};
}  // namespace transport
}  // namespace upmq
//}

#endif /*_UPMQWIREFORMAT_H_*/
