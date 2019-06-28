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

#ifndef _UPMQ_THREADS_COMPOSITETASKRUNNER_H_
#define _UPMQ_THREADS_COMPOSITETASKRUNNER_H_

#include <decaf/lang/Pointer.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/LinkedList.h>
#include <threads/CompositeTask.h>
#include <threads/TaskRunner.h>
#include <transport/Config.h>

namespace upmq {
namespace threads {

class CompositeTaskRunnerImpl;

/**
 * A Task Runner that can contain one or more CompositeTasks that are each checked
 * for pending work and run if any is present in the order that the tasks were added.
 *
 * @since 3.0
 */
class UPMQCPP_API CompositeTaskRunner : public upmq::threads::TaskRunner, public upmq::threads::Task, public decaf::lang::Runnable {
 private:
  CompositeTaskRunnerImpl *impl;

 private:
  CompositeTaskRunner(const CompositeTaskRunner &);
  CompositeTaskRunner &operator=(const CompositeTaskRunner &);

 public:
  CompositeTaskRunner();

  virtual ~CompositeTaskRunner();

  virtual void start();

  virtual bool isStarted() const;

  /**
   * Adds a new CompositeTask to the Set of Tasks that this class manages.
   * @param task - Pointer to a CompositeTask instance.
   */
  void addTask(CompositeTask *task);

  /**
   * Removes a CompositeTask that was added previously
   * @param task - Pointer to a CompositeTask instance.
   */
  void removeTask(CompositeTask *task);

  /**
   * Shutdown after a timeout, does not guarantee that the task's iterate
   * method has completed and the thread halted.
   *
   * @param timeout - Time in Milliseconds to wait for the task to stop.
   */
  virtual void shutdown(long long timeout);

  /**
   * Shutdown once the task has finished and the TaskRunner's thread has exited.
   */
  virtual void shutdown();

  /**
   * Signal the TaskRunner to wakeup and execute another iteration cycle on
   * the task, the Task instance will be run until its iterate method has
   * returned false indicating it is done.
   */
  virtual void wakeup();

 protected:
  virtual void run();

  virtual bool iterate();
};
}  // namespace threads
}  // namespace upmq

#endif /* _UPMQ_THREADS_COMPOSITETASKRUNNER_H_ */
