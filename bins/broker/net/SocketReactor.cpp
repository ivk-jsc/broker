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

#include "SocketReactor.h"
#include "SocketNotification.h"
#include "SocketNotifier.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Thread.h"
#include "Poco/Exception.h"

using Poco::ErrorHandler;
using Poco::Exception;
using Poco::FastMutex;

std::size_t Poco::hash(const Poco::Net::Socket& socket) { return Poco::hash(static_cast<UInt64>(socket.impl()->sockfd())); }

namespace upmq {
namespace Net {

SocketReactor::SocketReactor(size_t handlersSize)
    : _stop(false),
      _timeout(DEFAULT_TIMEOUT),
      _handlers(handlersSize),
      _pReadableNotification(new ReadableNotification(this)),
      _pWritableNotification(new WritableNotification(this)),
      _pErrorNotification(new ErrorNotification(this)),
      _pTimeoutNotification(new TimeoutNotification(this)),
      _pIdleNotification(new IdleNotification(this)),
      _pShutdownNotification(new ShutdownNotification(this)),
      _pThread(nullptr) {}

SocketReactor::SocketReactor(const Poco::Timespan& timeout, size_t handlersSize)
    : _stop(false),
      _timeout(timeout),
      _handlers(handlersSize),
      _pReadableNotification(new ReadableNotification(this)),
      _pWritableNotification(new WritableNotification(this)),
      _pErrorNotification(new ErrorNotification(this)),
      _pTimeoutNotification(new TimeoutNotification(this)),
      _pIdleNotification(new IdleNotification(this)),
      _pShutdownNotification(new ShutdownNotification(this)),
      _pThread(nullptr) {}

SocketReactor::~SocketReactor() {}

void SocketReactor::run() {
  _pThread = Poco::Thread::current();

  Poco::Net::Socket::SocketList readable;
  Poco::Net::Socket::SocketList writable;
  Poco::Net::Socket::SocketList except;

  while (!_stop) {
    try {
      size_t cnt = _handlers.size();
      readable.clear();
      readable.reserve(cnt);
      writable.clear();
      writable.reserve(cnt);
      except.clear();
      writable.reserve(cnt);
      int nSockets = 0;

      _handlers.applyForEach([&readable, &writable, &except, &nSockets, this](const EventHandlerMap::ItemType::KVPair& it) {
        if (it.second->accepts(_pReadableNotification)) {
          readable.push_back(it.first);
          nSockets++;
        }
        if (it.second->accepts(_pWritableNotification)) {
          writable.push_back(it.first);
          nSockets++;
        }
        if (it.second->accepts(_pErrorNotification)) {
          except.push_back(it.first);
          nSockets++;
        }
      });

      if (nSockets == 0) {
        // onIdle();
        Poco::Thread::trySleep(static_cast<long>(_timeout.totalMilliseconds()));
      } else if (Poco::Net::Socket::select(readable, writable, except, _timeout)) {
        onBusy();

        for (auto& it : readable) {
          dispatch(it, _pReadableNotification);
        }
        for (auto& it : writable) {
          dispatch(it, _pWritableNotification);
        }
        for (auto& it : except) {
          dispatch(it, _pErrorNotification);
        }
      } else {
        onTimeout();
      }
    } catch (Exception& exc) {
      ErrorHandler::handle(exc);
    } catch (std::exception& exc) {
      ErrorHandler::handle(exc);
    } catch (...) {
      ErrorHandler::handle();
    }
  }
  onShutdown();
}

void SocketReactor::stop() { _stop = true; }

void SocketReactor::wakeUp() {
  if (_pThread) _pThread->wakeUp();
}

void SocketReactor::setTimeout(const Poco::Timespan& timeout) { _timeout = timeout; }

const Poco::Timespan& SocketReactor::getTimeout() const { return _timeout; }

void SocketReactor::addEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer) {
  NotifierPtr pNotifier;

  auto it = _handlers.find(socket);
  if (!it.hasValue()) {
    pNotifier = new upmq::Net::SocketNotifier(socket);
    _handlers.insert(std::make_pair(socket, pNotifier));
  } else
    pNotifier = *it;

  if (!pNotifier->hasObserver(observer)) {
    pNotifier->addObserver(this, observer);
  }
}

bool SocketReactor::hasEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer) {
  NotifierPtr pNotifier;

  auto it = _handlers.find(socket);
  if (it.hasValue()) {
    if ((*it)->hasObserver(observer)) return true;
  }

  return false;
}

void SocketReactor::removeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer) {
  NotifierPtr pNotifier;
  bool needErase = false;
  {
    auto it = _handlers.find(socket);
    if (it.hasValue()) {
      pNotifier = *it;
      if (pNotifier->hasObserver(observer) && pNotifier->countObservers() == 1) {
        needErase = true;
      }
    }
  }
  if (needErase) {
    _handlers.erase(socket);
  }
  if (pNotifier && pNotifier->hasObserver(observer)) {
    pNotifier->removeObserver(this, observer);
  }
}

size_t SocketReactor::handlersSize() const { return _handlers.capacity(); }

void SocketReactor::onTimeout() { dispatch(_pTimeoutNotification); }

void SocketReactor::onIdle() { dispatch(_pIdleNotification); }

void SocketReactor::onShutdown() { dispatch(_pShutdownNotification); }

void SocketReactor::onBusy() {}

void SocketReactor::dispatch(const Poco::Net::Socket& socket, SocketNotification* pNotification) {
  NotifierPtr pNotifier;
  {
    auto it = _handlers.find(socket);
    if (it.hasValue()) {
      pNotifier = *it;
    } else {
      return;
    }
  }
  dispatch(pNotifier, pNotification);
}

void SocketReactor::dispatch(SocketNotification* pNotification) {
  _handlers.applyForEach(
      [&pNotification, this](const EventHandlerMap::ItemType::KVPair& it) { dispatch(const_cast<NotifierPtr&>(it.second), pNotification); });
}

void SocketReactor::dispatch(NotifierPtr& pNotifier, SocketNotification* pNotification) {
  try {
    pNotifier->dispatch(pNotification);
  } catch (Exception& exc) {
    ErrorHandler::handle(exc);
  } catch (std::exception& exc) {
    ErrorHandler::handle(exc);
  } catch (...) {
    ErrorHandler::handle();
  }
}

}  // namespace Net
}  // namespace upmq
