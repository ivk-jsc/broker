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

#ifndef _DECAF_UTIL_CONCURRENT_CONCURRENT_H_
#define _DECAF_UTIL_CONCURRENT_CONCURRENT_H_

#include <decaf/util/concurrent/Lock.h>

namespace decaf {
namespace util {
namespace concurrent {

/**
 * The synchronized macro defines a mechanism for synchronizing
 * a section of code.  The macro must be passed an object that
 * implements the Synchronizable interface.
 *
 * The macro works by creating a for loop that will loop exactly
 * once, creating a Lock object that is scoped to the loop.  Once
 * the loop completes and exits the Lock object goes out of scope
 * releasing the lock on object W.  For added safety the if else
 * is used because not all compiles restrict the lifetime of
 * loop variables to the loop, they will however restrict them
 * to the scope of the else.
 *
 * The macro would be used as follows.
 *
 * Synchronizable X;
 *
 * somefunction()
 * {
 *    synchronized(X)
 *    {
 *       // Do something that needs synchronizing.
 *    }
 * }
 */

#define WAIT_INFINITE 0xFFFFFFFF

#define _DO_JOIN(symbol1, symbol2) symbol1##symbol2
#define JOIN(symbol1, symbol2) _DO_JOIN(symbol1, symbol2)
#define LINE_NAME(prefix) JOIN(prefix, __LINE__)

#define synchronized(W) \
  if (false) {          \
  } else                \
    for (decaf::util::concurrent::Lock LINE_NAME(lock_W)(W); LINE_NAME(lock_W).isLocked(); LINE_NAME(lock_W).unlock())
}  // namespace concurrent
}  // namespace util
}  // namespace decaf

#endif /*_DECAF_UTIL_CONCURRENT_CONCURRENT_H_*/
