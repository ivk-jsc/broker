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

#ifndef _UPMQ_CLOSE_TRANSPORT_STASK_H_
#define _UPMQ_CLOSE_TRANSPORT_STASK_H_

#include <threads/CompositeTask.h>
#include <transport/Config.h>
#include <transport/Transport.h>

#include <decaf/lang/Pointer.h>
#include <decaf/util/concurrent/LinkedBlockingQueue.h>

namespace upmq {
namespace transport {
namespace failover {

using decaf::lang::Pointer;

class UPMQCPP_API CloseTransportsTask : public upmq::threads::CompositeTask {
 private:
  mutable decaf::util::concurrent::LinkedBlockingQueue<Pointer<Transport> > transports;

 public:
  CloseTransportsTask();

  virtual ~CloseTransportsTask();

  /**
   * Add a new Transport to close.
   */
  void add(const Pointer<Transport> transport);

  /**
   * This Task is pending if there are transports in the Queue that need to be
   * closed.
   *
   * @return true if there is a transport in the queue that needs closed.
   */
  virtual bool isPending() const;

  /**
   * Return true until all transports have been closed and removed from the queue.
   */
  virtual bool iterate();
};
}  // namespace failover
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_CLOSE_TRANSPORTS_TASK_H_ */
