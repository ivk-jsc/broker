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

#ifndef _DECAF_SECURITY_DIGESTEXCEPTION_H_
#define _DECAF_SECURITY_DIGESTEXCEPTION_H_

#include <decaf/security/GeneralSecurityException.h>

namespace decaf {
namespace security {

/*
 * This exception is thrown when a particular cryptographic algorithm is
 * requested but is not available in the environment.
 */

DECAF_DECLARE_EXCEPTION(DECAF_API, DigestException, GeneralSecurityException)

}  // namespace security
}  // namespace decaf

#endif /*_DECAF_SECURITY_DIGESTEXCEPTION_H_*/
