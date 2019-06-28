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
 
#ifndef _UPMQ_TRANSPORT_RESPONSECALLBACK_H_
#define _UPMQ_TRANSPORT_RESPONSECALLBACK_H_

#include <transport/Config.h>
#include <transport/Response.h>

#include <decaf/lang/Pointer.h>

namespace upmq {
namespace transport {

/**
 * Allows an async send to complete at a later time via a Response event.
 */
class UPMQCPP_API ResponseCallback {
 private:
  ResponseCallback(const ResponseCallback &);
  ResponseCallback &operator=(const ResponseCallback &);

 public:
  ResponseCallback();
  virtual ~ResponseCallback();

 public:
  /**
   * When an Asynchronous operations completes this event is fired.
   *
   * The provided FutureResponse can either contain the result of the operation
   * or an exception indicating that the operation failed.
   *
   * @param response
   *      The result of the asynchronous operation that registered this call-back.
   */
  virtual void onComplete(decaf::lang::Pointer<transport::Response> response) = 0;
};
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_TRANSPORT_RESPONSECALLBACK_H_ */
