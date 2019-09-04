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

#ifndef __ConnectionImpl_CPP__
#define __ConnectionImpl_CPP__

#include "ConnectionImpl.h"
#include "DecafSingleton.h"
#include "Util.h"

#include <transport/TransportRegistry.h>
#include <transport/UPMQCommand.h>
#include <transport/URISupport.h>
#include <transport/correlator/ResponseCorrelator.h>
#include <transport/failover/FailoverTransportFactory.h>
#include <transport/tcp/TcpTransport.h>

#include <decaf/util/UUID.h>
#include <utility>
using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::tcp;
using namespace upmq::transport::failover;
using namespace upmq::transport::correlator;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::net;

ConnectionImpl::ConnectionImpl(const string &uri, const string &username, const string &password, const string &clientId)
    : _transportFailed(false),
      _exception(nullptr),
      _exceptionListener(nullptr),
      _objectId(),
      _username(username),
      _password(password),
      _uriInternal(uri),
      _uri(),
      _transportWait(),
      _closed(false),
      _started(false),
      _stoped(false) {
  try {
    DecafSingleton::Instance();

    _objectId = clientId.empty() ? UUID::randomUUID().toString() : clientId;
    trim(_uriInternal);
    _uri = URI(_uriInternal);

    Properties properties = upmq::transport::URISupport::parseQuery(_uri.getQuery());
    _transportWait = Integer::parseInt(properties.getProperty("transport.wait", "30000"));

    transport = TransportRegistry::getInstance().findFactory(_uri.getScheme())->create(_uri);
    if (transport.get() == nullptr) {
      throw cms::CMSException("failed creating new transport");
    }

    transport->setTransportListener(this);
    transport->start();

    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getConnect().set_client_id(_objectId);
    if (!request->getConnect().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    Proto::Connected connected = this->syncRequest(request.dynamicCast<Command>())->processConnected();
    _cmd._sv = connected.server_version();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

ConnectionImpl::~ConnectionImpl() {
  try {
    if (!isClosed()) {
      close();
    }
  }
  CATCH_ALL_NOTHROW
}

void ConnectionImpl::close() {
  try {
    synchronized(&_lock) {
      if (isClosed()) {
        return;
      }

      std::vector<SessionImpl *> sessions;
      for (auto &it : _sessionsMap) {
        sessions.push_back(it.second);
      }
      for (auto value : sessions) {
        value->close();
        value->_connection = nullptr;
      }

      Pointer<UPMQCommand> request(new UPMQCommand());
      request->getProtoMessage().set_object_id(_objectId);

      request->getDisconnect().set_client_id(_objectId);
      request->getDisconnect().set_receipt_id("");
      if (!request->getDisconnect().IsInitialized()) {
        throw cms::CMSException("request not initialized");
      }

      this->transport->oneway(request.dynamicCast<Command>());

      setStarted(false);
      setStoped(true);
      setClosed(true);
      if (!this->_transportFailed.get()) {
        transport->close();
      }
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

string ConnectionImpl::getClientID() const {
  _checkClosed();
  return _objectId;
}

void ConnectionImpl::setClientID(const string &clientId) {
  try {
    synchronized(&_lock) {
      _checkClosed();

      _checkOpenSessionsAndSetExListener();
      _checkBeforeStartedStoped();

      if (clientId.empty()) {
        return;
      }

      Pointer<UPMQCommand> request(new UPMQCommand());
      request->getProtoMessage().set_object_id(_objectId);

      request->getClientInfo().set_new_client_id(clientId);
      request->getClientInfo().set_old_client_id(_objectId);
      request->getClientInfo().set_receipt_id(_objectId);

      if (!request->getClientInfo().IsInitialized()) {
        throw cms::CMSException("request not initialized");
      }

      this->syncRequest(request.dynamicCast<Command>())->processReceipt();

      _objectId = clientId;
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

const cms::ConnectionMetaData *ConnectionImpl::getMetaData() const {
  _checkClosed();

  return &_cmd;
}

cms::Session *ConnectionImpl::createSession() {
  SessionImpl *sessionImpl = nullptr;
  try {
    synchronized(&_lock) {
      _checkClosed();
      sessionImpl = new SessionImpl(this, cms::Session::AUTO_ACKNOWLEDGE);
      _sessionsMap.insert(make_pair(sessionImpl->getObjectId(), sessionImpl));
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
  return sessionImpl;
}

cms::Session *ConnectionImpl::createSession(cms::Session::AcknowledgeMode ackMode) {
  SessionImpl *sessionImpl = nullptr;
  try {
    synchronized(&_lock) {
      _checkClosed();
      sessionImpl = new SessionImpl(this, ackMode);
      _sessionsMap.insert(make_pair(sessionImpl->getObjectId(), sessionImpl));
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
  return sessionImpl;
}

void ConnectionImpl::setExceptionListener(cms::ExceptionListener *listener) {
  synchronized(&_lock) {
    _checkClosed();
    _exceptionListener = listener;
  }
}

cms::ExceptionListener *ConnectionImpl::getExceptionListener() const {
  _checkClosed();
  return _exceptionListener;
}

void ConnectionImpl::start() {
  try {
    synchronized(&_lock) {
      _checkClosed();

      setStarted(true);
      setStoped(false);
      setClosed(false);

      for (map<string, SessionImpl *>::iterator it = _sessionsMap.begin(); it != _sessionsMap.end(); ++it) {
        it->second->start();
      }
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConnectionImpl::stop() {
  try {
    synchronized(&_lock) {
      _checkClosed();

      for (map<string, SessionImpl *>::iterator it = _sessionsMap.begin(); it != _sessionsMap.end(); ++it) {
        it->second->stop();
      }

      setStarted(false);
      setStoped(true);
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

bool ConnectionImpl::isAlive() const { return (_started && !_closed && !_stoped); }

bool ConnectionImpl::isStarted() const { return _started; }

void ConnectionImpl::setStarted(bool started) { _started = started; }

bool ConnectionImpl::isStoped() const { return _stoped; }

void ConnectionImpl::setStoped(bool stoped) { _stoped = stoped; }

bool ConnectionImpl::isClosed() const { return _closed; }

void ConnectionImpl::setClosed(bool closed) { _closed = closed; }

void ConnectionImpl::_checkClosed() const {
  if (isClosed()) {
    throw cms::IllegalStateException();
  }
}

void ConnectionImpl::_checkOpenSessionsAndSetExListener() const {
  if (!_sessionsMap.empty() || _exceptionListener != nullptr) {
    throw cms::IllegalStateException();
  }
}

void ConnectionImpl::_checkBeforeStartedStoped() const {
  if ((isStarted() == false) && (isStoped() == false)) {
  } else {
    throw cms::IllegalStateException();
  }
}

Pointer<Response> ConnectionImpl::syncRequest(Pointer<Command> command) {
  try {
    Pointer<Response> response = this->transport->request(std::move(command), this->_transportWait);
    return response;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

Pointer<Response> ConnectionImpl::asyncRequest(Pointer<Command> command) {
  try {
    Pointer<Response> response = this->transport->request(std::move(command));
    return response;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConnectionImpl::addDispatcher(ConsumerImpl *consumerImpl) {
  try {
    synchronized(&_lockCommand) { _dispatchersMap.insert(make_pair(consumerImpl->getObjectId(), consumerImpl)); }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConnectionImpl::removeDispatcher(ConsumerImpl *consumerImpl) {
  try {
    synchronized(&_lockCommand) { _dispatchersMap.erase(consumerImpl->getObjectId()); }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

ConsumerImpl *ConnectionImpl::getDispatcher(string objectId) {
  ConsumerImpl *consumer = nullptr;
  try {
    synchronized(&_lockCommand) { consumer = _dispatchersMap.at(objectId); }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
  return consumer;
}

void ConnectionImpl::postDispatcher() {
  try {
    synchronized(&_lockCommand) {
      for (auto const &entry : _dispatchersMap) {
        entry.second->_messageQueue->close();
      }
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConnectionImpl::onCommand(const Pointer<Command> command) {
  try {
    UPMQCommand *upmqCommand = (UPMQCommand *)command.get();
    if (upmqCommand != nullptr) {
      if (upmqCommand->_header->ProtoMessageType_case() == Proto::ProtoMessage::kMessage) {
        // cout << "< message id " << ((UPMQCommand *)command.get())->getMessage().message_id();

        synchronized(&_lockCommand) {
          // TODO what if ex on no dispatcher
          ConsumerImpl *dispatcher = getDispatcher(((UPMQCommand *)command.get())->getObjectId());
          if (dispatcher != nullptr) {
            // Pointer<commands::Message> message = dispatch->getMessage();
            //// Message == NULL to signal the end of a Queue Browse.
            // if (message != NULL) {
            //  message->setReadOnlyBody(true);
            //  message->setReadOnlyProperties(true);
            //  message->setRedeliveryCounter(dispatch->getRedeliveryCounter());
            //  message->setConnection(this);
            //}

            long long ttl = ((UPMQCommand *)command.get())->_header->mutable_message()->timetolive();
            if (ttl > 0) {
              ((UPMQCommand *)command.get())->_header->mutable_message()->set_expiration(ttl + System::currentTimeMillis());
            }

            ((UPMQCommand *)command.get())->_consumer = dispatcher;
            dispatcher->dispatch(command);
            // cout << " to consumer " << dispatcher->getObjectId() << endl;
          }
        }
      }
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConnectionImpl::onException(const decaf::lang::Exception &ex) {
  try {
    this->_transportFailed.set(true);
    transport->stop();
    postDispatcher();

    cms::ExceptionListener *listener = this->getExceptionListener();
    if (listener != nullptr) {
      const cms::CMSException *cause = dynamic_cast<const cms::CMSException *>(ex.getCause());
      if (cause != nullptr) {
        listener->onException(*cause);
      } else {
        UPMQException upmqEx(ex);
        listener->onException(upmqEx.convertToCMSException());
      }
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}
void ConnectionImpl::transportInterrupted() {}

void ConnectionImpl::transportResumed() {}

string &ConnectionImpl::getObjectId() { return _objectId; }

#endif  //__ConnectionImpl_CPP__
