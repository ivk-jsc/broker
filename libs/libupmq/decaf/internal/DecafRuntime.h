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

#ifndef _DECAF_INTERNAL_DECAFRUNTIME_H
#define _DECAF_INTERNAL_DECAFRUNTIME_H

#include <decaf/lang/Runtime.h>
#include <decaf/util/Config.h>
#include <decaf/util/concurrent/Mutex.h>

namespace decaf {
namespace internal {

/**
 * Handles APR initialization and termination.
 */
class DECAF_API DecafRuntime : public decaf::lang::Runtime {
 public:
  DecafRuntime(const DecafRuntime &) = delete;
  DecafRuntime &operator=(const DecafRuntime &) = delete;
  /**
   * Initializes the APR Runtime for a library.
   */
  DecafRuntime();

  /**
   * Terminates the APR Runtime for a library.
   */
  ~DecafRuntime() override;

  /**
   * Gets a pointer to the Decaf Runtime's Global Lock object, this can be used by
   * Decaf APIs to synchronize around certain actions such as adding or acquiring
   * a resource that must be Thread safe.
   *
   * The pointer returned is owned by the Decaf runtime and should not be
   * deleted or copied by the caller.
   *
   * @return a pointer to the Decaf Runtime's global Lock instance.
   */
  decaf::util::concurrent::Mutex *getGlobalLock();
};
}  // namespace internal
}  // namespace decaf

#endif /*_DECAF_INTERNAL_DECAFRUNTIME_H*/
