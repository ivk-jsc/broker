//
// NotificationCenter.cpp
//
// Library: Foundation
// Package: Notifications
// Module:  NotificationCenter
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//

#include "NotificationCenter.h"
#include "Poco/Notification.h"
#include "Poco/Observer.h"
#include "Poco/AutoPtr.h"
#include "Poco/SingletonHolder.h"

namespace upmq {

NotificationCenter::NotificationCenter() {}

NotificationCenter::~NotificationCenter() {}

void NotificationCenter::addObserver(const Poco::AbstractObserver& observer) {
  upmq::ScopedWriteRWLock writeRwLock(_mutex);
  _observers.push_back(observer.clone());
}

void NotificationCenter::removeObserver(const Poco::AbstractObserver& observer) {
  upmq::ScopedWriteRWLock writeRwLock(_mutex);
  for (auto it = _observers.begin(); it != _observers.end(); ++it) {
    if (observer.equals(**it)) {
      (*it)->disable();
      _observers.erase(it);
      return;
    }
  }
}

bool NotificationCenter::hasObserver(const Poco::AbstractObserver& observer) const {
  upmq::ScopedReadRWLock readRwLock(_mutex);
  for (const auto& _observer : _observers) {
    if (observer.equals(*_observer)) {
      return true;
    }
  }

  return false;
}

void NotificationCenter::postNotification(Poco::Notification::Ptr pNotification) {
  poco_check_ptr(pNotification);

  upmq::ScopedReadRWLockWithUnlock readRwLock(_mutex);
  ObserverList observersToNotify(_observers);
  readRwLock.unlock();
  for (auto& it : observersToNotify) {
    it->notify(pNotification);
  }
}

bool NotificationCenter::hasObservers() const {
  upmq::ScopedReadRWLock readRwLock(_mutex);
  return !_observers.empty();
}

std::size_t NotificationCenter::countObservers() const {
  upmq::ScopedReadRWLock readRwLock(_mutex);
  return _observers.size();
}

namespace {
static Poco::SingletonHolder<upmq::NotificationCenter> sh;
}

upmq::NotificationCenter& NotificationCenter::defaultCenter() { return *sh.get(); }

}  // namespace upmq
