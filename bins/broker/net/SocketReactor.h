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

#ifndef UPMQ_Net_SocketReactor_INCLUDED
#define UPMQ_Net_SocketReactor_INCLUDED

#include "Poco/Net/Net.h"
#include "Poco/Net/Socket.h"
#include "Poco/Runnable.h"
#include "Poco/Timespan.h"
#include "Poco/Observer.h"
#include "Poco/AutoPtr.h"
#include "SocketNotifier.h"
#include "Poco/RWLock.h"

namespace Poco {
class Thread;
namespace Net {
class Socket;
}  // namespace Net

std::size_t hash(const Poco::Net::Socket& socket);
}  // namespace Poco
#include "FixedSizeUnorderedMap.h"
namespace upmq {

namespace Net {

class SocketNotification;
class SocketNotifier;

class SocketReactor : public Poco::Runnable {
 public:
  explicit SocketReactor(size_t handlersSize);
  /// Creates the SocketReactor.

  explicit SocketReactor(const Poco::Timespan& timeout, size_t handlersSize);
  /// Creates the SocketReactor, using the given timeout.

  ~SocketReactor() override;
  /// Destroys the SocketReactor.

  void run() override;
  /// Runs the SocketReactor. The reactor will run
  /// until stop() is called (in a separate thread).

  void stop();
  /// Stops the SocketReactor.
  ///
  /// The reactor will be stopped when the next event
  /// (including a timeout event) occurs.

  void wakeUp();
  /// Wakes up idle reactor.

  void setTimeout(const Poco::Timespan& timeout);
  /// Sets the timeout.
  ///
  /// If no other event occurs for the given timeout
  /// interval, a timeout event is sent to all event listeners.
  ///
  /// The default timeout is 250 milliseconds;
  ///
  /// The timeout is passed to the Socket::select()
  /// method.

  const Poco::Timespan& getTimeout() const;
  /// Returns the timeout.

  void addEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);
  /// Registers an event handler with the SocketReactor.
  ///
  /// Usage:
  ///     Poco::Observer<MyEventHandler, SocketNotification> obs(*this, &MyEventHandler::handleMyEvent);
  ///     reactor.addEventHandler(obs);

  bool hasEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);
  /// Returns true if the observer is registered with SocketReactor for the given socket.

  void removeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);
  /// Unregisters an event handler with the SocketReactor.
  ///
  /// Usage:
  ///     Poco::Observer<MyEventHandler, SocketNotification> obs(*this, &MyEventHandler::handleMyEvent);
  ///     reactor.removeEventHandler(obs);

  size_t handlersSize() const;

 protected:
  virtual void onTimeout();
  /// Called if the timeout expires and no other events are available.
  ///
  /// Can be overridden by subclasses. The default implementation
  /// dispatches the TimeoutNotification and thus should be called by overriding
  /// implementations.

  virtual void onIdle();
  /// Called if no sockets are available to call select() on.
  ///
  /// Can be overridden by subclasses. The default implementation
  /// dispatches the IdleNotification and thus should be called by overriding
  /// implementations.

  virtual void onShutdown();
  /// Called when the SocketReactor is about to terminate.
  ///
  /// Can be overridden by subclasses. The default implementation
  /// dispatches the ShutdownNotification and thus should be called by overriding
  /// implementations.

  virtual void onBusy();
  /// Called when the SocketReactor is busy and at least one notification
  /// has been dispatched.
  ///
  /// Can be overridden by subclasses to perform additional
  /// periodic tasks. The default implementation does nothing.

  void dispatch(const Poco::Net::Socket& socket, upmq::Net::SocketNotification* pNotification);
  /// Dispatches the given notification to all observers
  /// registered for the given socket.

  void dispatch(upmq::Net::SocketNotification* pNotification);
  /// Dispatches the given notification to all observers.

 private:
  typedef Poco::AutoPtr<upmq::Net::SocketNotifier> NotifierPtr;
  typedef Poco::AutoPtr<upmq::Net::SocketNotification> NotificationPtr;
  typedef FSUnorderedMap<Poco::Net::Socket, NotifierPtr> EventHandlerMap;

  void dispatch(NotifierPtr& pNotifier, upmq::Net::SocketNotification* pNotification);

  enum { DEFAULT_TIMEOUT = 250000 };

  bool _stop;
  Poco::Timespan _timeout;
  EventHandlerMap _handlers;
  NotificationPtr _pReadableNotification;
  NotificationPtr _pWritableNotification;
  NotificationPtr _pErrorNotification;
  NotificationPtr _pTimeoutNotification;
  NotificationPtr _pIdleNotification;
  NotificationPtr _pShutdownNotification;
  Poco::Thread* _pThread;

  friend class SocketNotifier;
};

}  // namespace Net
}  // namespace upmq

#endif  // UPMQ_Net_SocketReactor_INCLUDED
