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

#ifndef _DECAF_UTIL_CONCURRENT_EXECUTIONEXCEPTION_H_
#define _DECAF_UTIL_CONCURRENT_EXECUTIONEXCEPTION_H_

#include <decaf/lang/Exception.h>

namespace decaf {
namespace util {
namespace concurrent {

/*
 * Exception thrown when attempting to retrieve the result of a task that aborted by
 * throwing an exception. This exception can be inspected using the Throwable.getCause()
 * method.
 */

DECAF_DECLARE_EXCEPTION(DECAF_API, ExecutionException, lang::Exception)

}  // namespace concurrent
}  // namespace util
}  // namespace decaf

#endif /*_DECAF_UTIL_CONCURRENT_EXECUTIONEXCEPTION_H_*/
