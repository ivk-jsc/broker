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

#ifndef _UPMQ_THREADS_TASKRUNNER_H_
#define _UPMQ_THREADS_TASKRUNNER_H_

#include <transport/Config.h>

namespace upmq {
namespace threads {

class UPMQCPP_API TaskRunner {
 public:
  virtual ~TaskRunner();

  /**
   * Starts the task runner.  Prior to call this method tasks can be added to a
   * Runner, but no executions will occur.  The start method will create the
   * background Thread(s) which do the work for this task runner.
   */
  virtual void start() = 0;

  /**
   * @return true if the start method has been called.
   */
  virtual bool isStarted() const = 0;

  /**
   * Shutdown after a timeout, does not guarantee that the task's iterate
   * method has completed and the thread halted.
   *
   * @param timeout - Time in Milliseconds to wait for the task to stop.
   */
  virtual void shutdown(long long timeout) = 0;

  /**
   * Shutdown once the task has finished and the TaskRunner's thread has exited.
   */
  virtual void shutdown() = 0;

  /**
   * Signal the TaskRunner to wakeup and execute another iteration cycle on
   * the task, the Task instance will be run until its iterate method has
   * returned false indicating it is done.
   */
  virtual void wakeup() = 0;
};
}  // namespace threads
}  // namespace upmq

#endif /*_UPMQ_THREADS_TASKRUNNER_H_*/
