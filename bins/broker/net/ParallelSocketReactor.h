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

#ifndef UPMQ_Net_ParallelSocketReactor_INCLUDED
#define UPMQ_Net_ParallelSocketReactor_INCLUDED

#include "SocketReactor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/NObserver.h"
#include "Poco/Thread.h"
#include "Poco/SharedPtr.h"

using Poco::AutoPtr;
using Poco::NObserver;
using Poco::Thread;
using Poco::Net::ServerSocket;
using Poco::Net::Socket;
using Poco::Net::StreamSocket;
using upmq::Net::ReadableNotification;
using upmq::Net::ShutdownNotification;

namespace upmq {
namespace Net {

template <class SR, typename... Args>
class ParallelSocketReactor : public SR {
 public:
  typedef Poco::SharedPtr<ParallelSocketReactor> Ptr;

  explicit ParallelSocketReactor(Args... args) : SR(args...) { _thread.start(*this); }
  ~ParallelSocketReactor() override;

 protected:
  void onIdle() override {
    SR::onIdle();
    Poco::Thread::yield();
  }

 private:
  Poco::Thread _thread;
};

template <class SR, typename... Args>
ParallelSocketReactor<SR, Args...>::~ParallelSocketReactor() {
  try {
    this->stop();
    _thread.join();
  } catch (...) {
    poco_unexpected();
  }
}

}  // namespace Net
}  // namespace upmq

#endif  // UPMQ_Net_ParallelSocketReactor_INCLUDED
