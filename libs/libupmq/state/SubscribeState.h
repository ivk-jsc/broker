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

#ifndef _UPMQ_SUBSCRIBE_STATE_H_
#define _UPMQ_SUBSCRIBE_STATE_H_

#include <decaf/lang/Pointer.h>
#include <transport/Command.h>

namespace upmq {
namespace state {

using namespace decaf::lang;
using namespace upmq::transport;

class SubscribeState {
 private:
  Pointer<Command> info;

 private:
  SubscribeState(const SubscribeState &);
  SubscribeState &operator=(const SubscribeState &);

 public:
  SubscribeState(Pointer<Command> info);
  virtual ~SubscribeState();

  Pointer<Command> getInfo() const;
};
}  // namespace state
}  // namespace upmq

#endif /*_UPMQ_SUBSCRIBE_STATE_H_*/
