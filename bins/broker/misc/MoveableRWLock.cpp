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

#include "MoveableRWLock.h"

#ifdef _WIN32

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <Poco/RWLock.h>
#endif

namespace upmq {
MRWLock::MRWLock() : _rwLock(new SRWLOCK) {
#ifdef _WIN32
  InitializeSRWLock(_rwLock.get());
#endif
}

MRWLock::MRWLock(MRWLock&& o) noexcept = default;

void MRWLock::readLock() {
#ifdef _WIN32
  AcquireSRWLockShared(_rwLock.get());
#else
  _rwLock->readLock();
#endif
}

bool MRWLock::tryReadLock() {
#ifdef _WIN32
  return static_cast<bool>(TryAcquireSRWLockShared(_rwLock.get()));
#else
  return _rwLock->tryReadLock();
#endif
}

void MRWLock::writeLock() {
#ifdef _WIN32
  AcquireSRWLockExclusive(_rwLock.get());
#else
  _rwLock->writeLock();
#endif
}

bool MRWLock::tryWriteLock() {
#ifdef _WIN32
  return static_cast<bool>(TryAcquireSRWLockExclusive(_rwLock.get()));
#else
  return _rwLock->tryWriteLock();
#endif
}

void MRWLock::unlockRead() noexcept {
#ifdef _WIN32
  ReleaseSRWLockShared(_rwLock.get());
#else
  try {
    _rwLock->unlock();
  } catch (...) {
  }
#endif
}

void MRWLock::unlockWrite() noexcept {
#ifdef _WIN32
  ReleaseSRWLockExclusive(_rwLock.get());
#else
  try {
    _rwLock->unlock();
  } catch (...) {
  }
#endif
}

bool MRWLock::isValid() const { return _rwLock != nullptr; }

ScopedReadRWLock::ScopedReadRWLock(MRWLock& mrwLock) : _rwLock(mrwLock) { _rwLock.readLock(); }

ScopedReadRWLock::~ScopedReadRWLock() noexcept { _rwLock.unlockRead(); }

ScopedReadRWLockWithUnlock::ScopedReadRWLockWithUnlock(MRWLock& mrwLock) : _rwLock(mrwLock) { _rwLock.readLock(); }

ScopedReadRWLockWithUnlock::~ScopedReadRWLockWithUnlock() noexcept { unlock(); }

void ScopedReadRWLockWithUnlock::unlock() noexcept {
  if (_locked) {
    _locked = false;
    _rwLock.unlockRead();
  }
}

ScopedWriteRWLock::ScopedWriteRWLock(MRWLock& mrwLock) : _rwLock(mrwLock) { _rwLock.writeLock(); }

ScopedWriteRWLock::~ScopedWriteRWLock() noexcept { _rwLock.unlockWrite(); }

ScopedWriteRWLockWithUnlock::ScopedWriteRWLockWithUnlock(MRWLock& mrwLock) : _rwLock(mrwLock) { _rwLock.writeLock(); }

ScopedWriteRWLockWithUnlock::~ScopedWriteRWLockWithUnlock() noexcept { unlock(); }

void ScopedWriteRWLockWithUnlock::unlock() noexcept {
  if (_locked) {
    _locked = false;
    _rwLock.unlockWrite();
  }
}

ScopedWriteTryLocker::ScopedWriteTryLocker(MRWLock& mrwLock, bool locked) : _rwLock(mrwLock), _locked(locked) {}
ScopedWriteTryLocker::~ScopedWriteTryLocker() noexcept { unlock(); }
bool ScopedWriteTryLocker::tryLock() {
  _locked = _rwLock.tryWriteLock();
  return _locked;
}
void ScopedWriteTryLocker::unlock() noexcept {
  if (_locked) {
    _locked = false;
    _rwLock.unlockWrite();
  }
}
}  // namespace upmq
