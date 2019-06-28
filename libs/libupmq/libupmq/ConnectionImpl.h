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

#ifndef __ConnectionImpl_H__
#define __ConnectionImpl_H__

#include <cms/CMSException.h>
#include <cms/Closeable.h>
#include <cms/Connection.h>
#include <cms/Startable.h>
#include <cms/Stoppable.h>

#include <decaf/lang/Pointer.h>
#include <decaf/net/URI.h>
#include <decaf/util/StlMap.h>
#include <decaf/util/concurrent/Mutex.h>
#include <decaf/util/concurrent/atomic/AtomicBoolean.h>
#include <transport/Command.h>
#include <transport/Dispatcher.h>
#include <transport/Response.h>
#include <transport/Transport.h>
#include <transport/TransportListener.h>

#include "ConnectionMetaDataImpl.h"
#include "ConsumerImpl.h"
#include "ExceptionImpl.h"
#include "ProtoHeader.h"
#include "SessionImpl.h"

using namespace std;
using namespace decaf;
using namespace decaf::net;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace decaf::util::concurrent::atomic;
using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport;
using namespace upmq::transport;

class ConnectionImpl : public cms::Connection, public TransportListener {
 public:
  ConnectionImpl(const string &uri, const string &username, const string &password, const string &clientId);
  ~ConnectionImpl();

  cms::Session *createSession() override;
  cms::Session *createSession(cms::Session::AcknowledgeMode ackMode) override;

  string getClientID() const override;
  void setClientID(const string &clientID) override;

  const cms::ConnectionMetaData *getMetaData() const override;

  void setExceptionListener(cms::ExceptionListener *listener) override;
  cms::ExceptionListener *getExceptionListener() const override;

  void start() override;
  void stop() override;
  void close() override;

  string &getObjectId();

  virtual void onCommand(const Pointer<Command> command) override;
  virtual void onException(const decaf::lang::Exception &ex) override;
  virtual void transportInterrupted() override;
  virtual void transportResumed() override;

  AtomicBoolean _transportFailed;
  cms::CMSException *_exception;
  cms::ExceptionListener *_exceptionListener;

  map<string, SessionImpl *> _sessionsMap;
  map<string, ConsumerImpl *> _dispatchersMap;

  Pointer<Response> syncRequest(Pointer<Command> command);
  Pointer<Response> asyncRequest(Pointer<Command> command);

  bool isAlive() const;
  bool isStarted() const;
  bool isStoped() const;
  bool isClosed() const;

  void setStarted(bool started);
  void setStoped(bool stoped);
  void setClosed(bool closed);

  void addDispatcher(ConsumerImpl *consumerImpl);
  void removeDispatcher(ConsumerImpl *consumerImpl);
  ConsumerImpl *getDispatcher(string objectId);
  void postDispatcher();

  void _checkClosed() const;
  void _checkOpenSessionsAndSetExListener() const;
  void _checkBeforeStartedStoped() const;

 private:
  Mutex _lock;
  Mutex _lockCommand;

  string _objectId;
  string _username;
  string _password;
  string _uriInternal;
  URI _uri;
  int _transportWait;

  bool _closed;
  bool _started;
  bool _stoped;

  ConnectionMetaDataImpl _cmd;

  Pointer<Transport> transport;
};

#endif  //__ConnectionImpl_H__
