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

#include "AtomicRefCounter.h"
#include <algorithm>

decaf::util::concurrent::atomic::AtomicRefCounter::AtomicRefCounter() : counter(new decaf::util::concurrent::atomic::AtomicInteger(1)) {}

decaf::util::concurrent::atomic::AtomicRefCounter::AtomicRefCounter(const AtomicRefCounter& other) : counter(other.counter) { this->counter->incrementAndGet(); }

decaf::util::concurrent::atomic::AtomicRefCounter::AtomicRefCounter(AtomicRefCounter&& other) noexcept : counter(other.releaseCounter()) {}

void decaf::util::concurrent::atomic::AtomicRefCounter::swap(AtomicRefCounter& other) noexcept { std::swap(this->counter, other.counter); }

void decaf::util::concurrent::atomic::AtomicRefCounter::swap(AtomicRefCounter&& other) noexcept { this->counter = other.releaseCounter(); }

bool decaf::util::concurrent::atomic::AtomicRefCounter::release() {
  if (this->counter == nullptr) {
    return true;
  }
  if (this->counter->decrementAndGet() == 0) {
    delete this->counter;
    return true;
  }
  return false;
}

decaf::util::concurrent::atomic::AtomicInteger* decaf::util::concurrent::atomic::AtomicRefCounter::releaseCounter() {
  decaf::util::concurrent::atomic::AtomicInteger* tmp = counter;
  counter = nullptr;
  return tmp;
}
