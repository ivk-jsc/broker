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

#ifndef _DECAF_UTIL_CONCURRENT_ATOMIC_ATOMICREFCOUNTER_H_
#define _DECAF_UTIL_CONCURRENT_ATOMIC_ATOMICREFCOUNTER_H_

#include <decaf/util/concurrent/atomic/AtomicInteger.h>

namespace decaf {
namespace util {
namespace concurrent {
namespace atomic {

class AtomicRefCounter {
  decaf::util::concurrent::atomic::AtomicInteger *counter;

 public:
  AtomicRefCounter();
  AtomicRefCounter(const AtomicRefCounter &other);
  AtomicRefCounter(AtomicRefCounter &&other) noexcept;

  AtomicRefCounter &operator=(const AtomicRefCounter &) = delete;
  AtomicRefCounter &operator=(AtomicRefCounter &&) = delete;

  virtual ~AtomicRefCounter() = default;

 protected:
  /**
   * Swaps this instance's reference counter with the one given, this allows
   * for copy-and-swap semantics of this object.
   *
   * @param other
   *      The value to swap with this one's.
   */
  void swap(AtomicRefCounter &other) noexcept;

  void swap(AtomicRefCounter &&other) noexcept;

  /**
   * Removes a reference to the counter Atomically and returns if the counter
   * has reached zero, once the counter hits zero, the internal counter is
   * destroyed and this instance is now considered to be unreferenced.
   *
   * @return true if the count is now zero.
   */
  bool release();

 private:
  decaf::util::concurrent::atomic::AtomicInteger *releaseCounter();
};
}  // namespace atomic
}  // namespace concurrent
}  // namespace util
}  // namespace decaf

#endif /* _DECAF_UTIL_CONCURRENT_ATOMIC_ATOMICREFCOUNTER_H_ */
