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

#ifndef UPMQ_Net_ParallelSocketAcceptor_INCLUDED
#define UPMQ_Net_ParallelSocketAcceptor_INCLUDED

#include "ParallelSocketReactor.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Environment.h"
#include "Poco/NObserver.h"
#include "Poco/SharedPtr.h"
#include <vector>

using Poco::AutoPtr;
using Poco::NObserver;
using Poco::Net::ServerSocket;
using Poco::Net::Socket;
using Poco::Net::StreamSocket;

namespace upmq {
namespace Net {

template <class ServiceHandler, class SR>
class ParallelSocketAcceptor
/// This class implements the Acceptor part of the Acceptor-Connector design pattern.
/// Only the difference from single-threaded version is documented here, For full
/// description see Poco::Net::SocketAcceptor documentation.
///
/// This is a multi-threaded version of SocketAcceptor, it differs from the
/// single-threaded version in number of reactors (defaulting to number of processors)
/// that can be specified at construction time and is rotated in a round-robin fashion
/// by event handler. See ParallelSocketAcceptor::onAccept and
/// ParallelSocketAcceptor::createServiceHandler documentation and implementation for
/// details.
{
 public:
  typedef upmq::Net::ParallelSocketReactor<SR, size_t> ParallelReactor;

  explicit ParallelSocketAcceptor(ServerSocket& socket, size_t handlerSize, unsigned threads = Poco::Environment::processorCount())
      : _socket(socket),
        _pReactor(nullptr),
        _threads(threads),
        _next(0)
  /// Creates a ParallelSocketAcceptor using the given ServerSocket,
  /// sets number of threads and populates the reactors vector.
  {
    init(handlerSize);
  }

  ParallelSocketAcceptor(ServerSocket& socket, SocketReactor& reactor, unsigned threads = Poco::Environment::processorCount())
      : _socket(socket),
        _pReactor(&reactor),
        _threads(threads),
        _next(0)
  /// Creates a ParallelSocketAcceptor using the given ServerSocket, sets the
  /// number of threads, populates the reactors vector and registers itself
  /// with the given SocketReactor.
  {
    if (_threads > 0 && _threads != 1) {
      --_threads;
    }
    init(_pReactor->handlersSize());
    _pReactor->addEventHandler(_socket, Poco::Observer<ParallelSocketAcceptor, ReadableNotification>(*this, &ParallelSocketAcceptor::onAccept));
  }

  virtual ~ParallelSocketAcceptor()
  /// Destroys the ParallelSocketAcceptor.
  {
    try {
      if (_pReactor) {
        _pReactor->removeEventHandler(_socket,
                                      Poco::Observer<ParallelSocketAcceptor, ReadableNotification>(*this, &ParallelSocketAcceptor::onAccept));
      }
    } catch (...) {
      poco_unexpected();
    }
  }

  void setReactor(SocketReactor& reactor)
  /// Sets the reactor for this acceptor.
  {
    _pReactor = &reactor;
    if (!_pReactor->hasEventHandler(_socket,
                                    Poco::Observer<ParallelSocketAcceptor, ReadableNotification>(*this, &ParallelSocketAcceptor::onAccept))) {
      registerAcceptor(reactor);
    }
  }

  virtual void registerAcceptor(SocketReactor& reactor)
  /// Registers the ParallelSocketAcceptor with a SocketReactor.
  ///
  /// A subclass can override this function to e.g.
  /// register an event handler for timeout event.
  ///
  /// The overriding method must either call the base class
  /// implementation or register the accept handler on its own.
  {
    if (_pReactor) throw Poco::InvalidAccessException("Acceptor already registered.");

    _pReactor = &reactor;
    _pReactor->addEventHandler(_socket, Poco::Observer<ParallelSocketAcceptor, ReadableNotification>(*this, &ParallelSocketAcceptor::onAccept));
  }

  virtual void unregisterAcceptor()
  /// Unregisters the ParallelSocketAcceptor.
  ///
  /// A subclass can override this function to e.g.
  /// unregister its event handler for a timeout event.
  ///
  /// The overriding method must either call the base class
  /// implementation or unregister the accept handler on its own.
  {
    if (_pReactor) {
      _pReactor->removeEventHandler(_socket, Poco::Observer<ParallelSocketAcceptor, ReadableNotification>(*this, &ParallelSocketAcceptor::onAccept));
    }
  }

  void onAccept(ReadableNotification* pNotification)
  /// Accepts connection and creates event handler.
  {
    pNotification->release();
    StreamSocket sock = _socket.acceptConnection();
    _pReactor->wakeUp();
    createServiceHandler(sock);
  }

 protected:
  virtual ServiceHandler* createServiceHandler(StreamSocket& socket)
  /// Create and initialize a new ServiceHandler instance.
  ///
  /// Subclasses can override this method.
  {
    std::size_t next = _next++;
    if (_next == _reactors.size()) _next = 0;
    _reactors[next]->wakeUp();
    return new ServiceHandler(socket, *_reactors[next]);
  }

  SocketReactor* reactor()
  /// Returns a pointer to the SocketReactor where
  /// this SocketAcceptor is registered.
  ///
  /// The pointer may be null.
  {
    return _pReactor;
  }

  Socket& socket()
  /// Returns a reference to the SocketAcceptor's socket.
  {
    return _socket;
  }

  void init(size_t handlersSize)
  /// Populates the reactors vector.
  {
    poco_assert(_threads > 0);
    _reactors.reserve(_threads);

    for (unsigned i = 0; i < _threads; ++i) {
      Poco::SharedPtr<ParallelReactor> pr(new ParallelReactor(handlersSize));
      _reactors.push_back(pr);
    }
  }

  typedef std::vector<typename ParallelReactor::Ptr> ReactorVec;

  ReactorVec& reactors()
  /// Returns reference to vector of reactors.
  {
    return _reactors;
  }

  SocketReactor* reactor(std::size_t idx)
  /// Returns reference to the reactor at position idx.
  {
    return _reactors.at(idx).get();
  }

  std::size_t& next()
  /// Returns reference to the next reactor index.
  {
    return _next;
  }

 private:
  ParallelSocketAcceptor();
  ParallelSocketAcceptor(const ParallelSocketAcceptor&);
  ParallelSocketAcceptor& operator=(const ParallelSocketAcceptor&);

  ServerSocket _socket;
  SocketReactor* _pReactor;
  unsigned _threads;
  ReactorVec _reactors;
  std::size_t _next;
};

}  // namespace Net
}  // namespace upmq

#endif  // UPMQ_Net_ParallelSocketAcceptor_INCLUDED
