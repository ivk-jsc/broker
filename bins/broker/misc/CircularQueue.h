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

#ifndef BROKER_CIRCULARQUEUE_H
#define BROKER_CIRCULARQUEUE_H

#include <deque>
template <typename T>
class CircularQueue {
 private:
  std::deque<T> _queue;
  std::size_t _limit;

 public:
  explicit CircularQueue(size_t limit) : _limit(limit) {}
  CircularQueue(CircularQueue &&) = default;
  CircularQueue(const CircularQueue &) = default;
  CircularQueue &operator=(CircularQueue &&) = default;
  CircularQueue &operator=(const CircularQueue &) = default;
  ~CircularQueue() = default;

  void push(const T &element) {
    _queue.push_back(element);
    if (_queue.size() > _limit) {
      _queue.pop_front();
    }
  }
  void pop() { _queue.pop_front(); }
  T front() const { return _queue.front(); }
  T last() const { return _queue.at((_queue.empty() ? 0 : (_queue.size() - 1))); }
  const T &at(size_t index) const { return _queue.at(index); }
  bool empty() const { return _queue.empty(); }
};

#endif  // BROKER_CIRCULARQUEUE_H
