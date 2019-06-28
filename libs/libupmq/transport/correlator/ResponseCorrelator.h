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

#ifndef _UPMQ_TRANSPORT_CORRELATOR_RESPONSECORRELATOR_H_
#define _UPMQ_TRANSPORT_CORRELATOR_RESPONSECORRELATOR_H_

#include <transport/Command.h>
#include <transport/Config.h>
#include <transport/FutureResponse.h>
#include <transport/Response.h>
#include <transport/ResponseCallback.h>
#include <transport/TransportFilter.h>

#include <decaf/lang/Exception.h>
#include <decaf/lang/Pointer.h>

namespace upmq {
namespace transport {
namespace correlator {

using decaf::lang::Pointer;
using upmq::transport::Command;
using upmq::transport::Response;

class CorrelatorData;

/**
 * This type of transport filter is responsible for correlating asynchronous responses
 * with requests.  Non-response messages are simply sent directly to the CommandListener.
 * It owns the transport that it
 */
class UPMQCPP_API ResponseCorrelator : public TransportFilter {
 private:
  CorrelatorData *impl;

 private:
  ResponseCorrelator(const ResponseCorrelator &);
  ResponseCorrelator &operator=(const ResponseCorrelator &);

 public:
  /**
   * Creates a new ResponseCorrelator transport filter that wraps the given transport.
   *
   * @param next
   *      the next transport in the chain
   *
   * @throws NullPointerException if next if NULL.
   */
  ResponseCorrelator(Pointer<Transport> next);

  virtual ~ResponseCorrelator();

 public:  // Transport Methods
  virtual void oneway(const Pointer<Command> command);

  virtual Pointer<FutureResponse> asyncRequest(const Pointer<Command> command, const Pointer<ResponseCallback> responseCallback);

  virtual Pointer<Response> request(const Pointer<Command> command);

  virtual Pointer<Response> request(const Pointer<Command> command, unsigned int timeout);

  /**
   * This is called in the context of the nested transport's reading thread.  In
   * the case of a response object, updates the request map and notifies those
   * waiting on the response.  Non-response messages are just delegated to the
   * command listener.
   *
   * @param command
   *      The received from the nested transport.
   */
  virtual void onCommand(const Pointer<Command> command);

  /**
   * Event handler for an exception from a command transport.
   *
   * @param source
   *      The source of the exception.
   * @param ex
   *      The exception that was caught.
   */
  virtual void onException(const decaf::lang::Exception &ex);

 protected:
  virtual void doClose();

 private:
  void dispose(Pointer<decaf::lang::Exception> ex);
};
}  // namespace correlator
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_TRANSPORT_CORRELATOR_RESPONSECORRELATOR_H_*/
