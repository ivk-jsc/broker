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

#ifndef MOVABLE_POCO_RWLOCK_H
#define MOVABLE_POCO_RWLOCK_H

#include <memory>
#include <atomic>

#ifdef _WIN32
struct _RTL_SRWLOCK;
typedef _RTL_SRWLOCK SRWLOCK;
#else
namespace Poco {
class RWLock;
}
using SRWLOCK = Poco::RWLock;
#endif

namespace upmq {

class MRWLock {
  std::unique_ptr<SRWLOCK> _rwLock;

 public:
  MRWLock();
  MRWLock(MRWLock &&o) noexcept;

  MRWLock &operator=(MRWLock &&) = default;
  MRWLock(const MRWLock &) = delete;
  MRWLock &operator=(const MRWLock &) = delete;
  ~MRWLock() = default;

  void readLock();

  bool tryReadLock();

  void writeLock();

  bool tryWriteLock();

  void unlockRead() noexcept;

  void unlockWrite() noexcept;

  bool isValid() const;
};

class ScopedReadRWLock {
  MRWLock &_rwLock;

 public:
  explicit ScopedReadRWLock(MRWLock &mrwLock);
  ~ScopedReadRWLock() noexcept;
};

class ScopedReadRWLockWithUnlock {
  MRWLock &_rwLock;
  std::atomic_bool _locked{true};

 public:
  explicit ScopedReadRWLockWithUnlock(MRWLock &mrwLock);
  ~ScopedReadRWLockWithUnlock() noexcept;
  void unlock() noexcept;
};

class ScopedWriteRWLock {
  MRWLock &_rwLock;

 public:
  explicit ScopedWriteRWLock(MRWLock &mrwLock);
  ~ScopedWriteRWLock() noexcept;
};

class ScopedWriteRWLockWithUnlock {
  MRWLock &_rwLock;
  std::atomic_bool _locked{true};

 public:
  explicit ScopedWriteRWLockWithUnlock(MRWLock &mrwLock);
  ~ScopedWriteRWLockWithUnlock() noexcept;
  void unlock() noexcept;
};

class ScopedWriteTryLocker {
  MRWLock &_rwLock;
  std::atomic_bool _locked{false};

 public:
  explicit ScopedWriteTryLocker(MRWLock &mrwLock, bool locked = false);
  ~ScopedWriteTryLocker() noexcept;
  bool tryLock();
  void unlock() noexcept;
};

}  // namespace upmq

#endif  // MOVABLE_POCO_RWLOCK_H
