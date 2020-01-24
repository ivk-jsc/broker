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
#include <stdexcept>

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
  _tmRead.update();
#ifdef _WIN32
  AcquireSRWLockShared(_rwLock.get());
#else
  _rwLock->readLock();
#endif
}

bool MRWLock::tryReadLock() {
  _tmRead.update();
#ifdef _WIN32
  return static_cast<bool>(TryAcquireSRWLockShared(_rwLock.get()));
#else
  return _rwLock->tryReadLock();
#endif
}

void MRWLock::writeLock() {
  _tmWrite.update();
#ifdef _WIN32
  AcquireSRWLockExclusive(_rwLock.get());
#else
  _rwLock->writeLock();
#endif
}

bool MRWLock::tryWriteLock() {
  _tmWrite.update();
#ifdef _WIN32
  return static_cast<bool>(TryAcquireSRWLockExclusive(_rwLock.get()));
#else
  return _rwLock->tryWriteLock();
#endif
}

void MRWLock::unlockRead(const std::string& parentFunc) noexcept {
#ifdef _WIN32
  ReleaseSRWLockShared(_rwLock.get());
#else
  try {
    auto tmp = _tmRead.elapsed();
    if (tmp > _tmDiffRead) {
      _tmDiffRead = tmp;
      _parentFunc = parentFunc;
      if (_tmDiffRead > 100) {
        std::string s = "read max : ";
        s += std::to_string(_tmDiffRead).append(" ").append(_parentFunc);
        puts(s.c_str());
      }
      if (_parentFunc.empty()) {
        int ii = 0;
      }
    }
    _rwLock->unlock();
  } catch (...) {
  }
#endif
}

void MRWLock::unlockWrite(const std::string& parentFunc) noexcept {
#ifdef _WIN32
  ReleaseSRWLockExclusive(_rwLock.get());
#else
  try {
    auto tmp = _tmWrite.elapsed();
    if (tmp > _tmDiffWrite) {
      _tmDiffWrite = tmp;
      _parentFunc = parentFunc;
      if (_tmDiffWrite > 100) {
        std::string s = "write max : ";
        s += std::to_string(_tmDiffWrite).append(" ").append(_parentFunc);
        puts(s.c_str());
      }
    }
    _rwLock->unlock();
  } catch (...) {
  }
#endif
}

bool MRWLock::isValid() const { return _rwLock != nullptr; }

ScopedReadRWLock::ScopedReadRWLock(MRWLock& mrwLock, std::string parentFunc) : _rwLock(mrwLock), _parentFunc(std::move(parentFunc)) {
  _rwLock.readLock();
}

ScopedReadRWLock::~ScopedReadRWLock() noexcept { _rwLock.unlockRead(_parentFunc); }

ScopedReadRWLockWithUnlock::ScopedReadRWLockWithUnlock(MRWLock& mrwLock, std::string parentFunc)
    : _rwLock(mrwLock), _parentFunc(std::move(parentFunc)) {
  _rwLock.readLock();
}

ScopedReadRWLockWithUnlock::~ScopedReadRWLockWithUnlock() noexcept { unlock(); }

void ScopedReadRWLockWithUnlock::unlock() noexcept {
  if (_locked) {
    _locked = false;
    _rwLock.unlockRead(_parentFunc);
  }
}

ScopedWriteRWLock::ScopedWriteRWLock(MRWLock& mrwLock, std::string parentFunc) : _rwLock(mrwLock), _parentFunc(std::move(parentFunc)) {
  _rwLock.writeLock();
}

ScopedWriteRWLock::~ScopedWriteRWLock() noexcept { _rwLock.unlockWrite(_parentFunc); }

ScopedWriteRWLockWithUnlock::ScopedWriteRWLockWithUnlock(MRWLock& mrwLock, std::string parentFunc)
    : _rwLock(mrwLock), _parentFunc(std::move(parentFunc)) {
  _rwLock.writeLock();
}

ScopedWriteRWLockWithUnlock::~ScopedWriteRWLockWithUnlock() noexcept { unlock(); }

void ScopedWriteRWLockWithUnlock::unlock() noexcept {
  if (_locked) {
    _locked = false;
    _rwLock.unlockWrite(_parentFunc);
  }
}

ScopedWriteTryLocker::ScopedWriteTryLocker(MRWLock& mrwLock, bool locked, std::string parentFunc)
    : _rwLock(mrwLock), _parentFunc(std::move(parentFunc)), _locked(locked) {}
ScopedWriteTryLocker::~ScopedWriteTryLocker() noexcept { unlock(); }
bool ScopedWriteTryLocker::tryLock() {
  _locked = _rwLock.tryWriteLock();
  return _locked;
}
void ScopedWriteTryLocker::unlock() noexcept {
  if (_locked) {
    _locked = false;
    _rwLock.unlockWrite(_parentFunc);
  }
}
}  // namespace upmq
