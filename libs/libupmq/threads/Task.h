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

#ifndef _UPMQ_THREADS_TASK_H_
#define _UPMQ_THREADS_TASK_H_

#include <transport/Config.h>

namespace upmq {
namespace threads {

/**
 * Represents a unit of work that requires one or more iterations to complete.
 *
 * @since 3.0
 */
class UPMQCPP_API Task {
 public:
  virtual ~Task();

  /**
   * Perform one iteration of work, returns true if the task needs
   * to run again to complete or false to indicate that the task is now
   * complete.
   *
   * @return true if the task should be run again or false if the task
   *         has completed and the runner should wait for a wakeup call.
   */
  virtual bool iterate() = 0;
};
}  // namespace threads
}  // namespace upmq

#endif /* _UPMQ_THREADS_TASK_H_ */
