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

#ifndef _DECAF_UTIL_CONCURRENT_TIMEOUTEXCEPTION_H_
#define _DECAF_UTIL_CONCURRENT_TIMEOUTEXCEPTION_H_

#include <decaf/lang/Exception.h>

namespace decaf {
namespace util {
namespace concurrent {

/*
 * Exception thrown when a blocking operation times out. Blocking operations for which
 * a timeout is specified need a means to indicate that the timeout has occurred. For
 * many such operations it is possible to return a value that indicates timeout; when
 * that is not possible or desirable then TimeoutException should be declared and thrown.
 */
DECAF_DECLARE_EXCEPTION(DECAF_API, TimeoutException, lang::Exception)

}  // namespace concurrent
}  // namespace util
}  // namespace decaf

#endif /*_DECAF_UTIL_CONCURRENT_TIMEOUTEXCEPTION_H_*/
