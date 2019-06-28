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

#ifndef _UPMQ_TRACKED_H_
#define _UPMQ_TRACKED_H_

#include <decaf/lang/Pointer.h>
#include <decaf/lang/Runnable.h>
#include <transport/Config.h>
#include <transport/Response.h>

namespace upmq {
namespace state {

class Tracked : public transport::Response {
 private:
  decaf::lang::Pointer<decaf::lang::Runnable> runnable;

 public:
  Tracked();
  Tracked(decaf::lang::Pointer<decaf::lang::Runnable> runnable);

  virtual ~Tracked() {}

  void onResponse();

  bool isWaitingForResponse() const { return runnable != nullptr; }
};
}  // namespace state
}  // namespace upmq

#endif /*_UPMQ_TRACKED_H_*/
