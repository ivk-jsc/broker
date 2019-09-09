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

#ifndef __MapMessageImpl_CPP__
#define __MapMessageImpl_CPP__

#include "ConsumerImpl.h"

#include "BytesMessageImpl.h"
#include "MapMessageImpl.h"
#include "MessageImpl.h"
#include "ObjectMessageImpl.h"
#include "StreamMessageImpl.h"
#include "TextMessageImpl.h"

#include <decaf/lang/exceptions/InterruptedException.h>
#include <decaf/util/UUID.h>

#include <transport/SimplePriorityMessageDispatchChannel.h>
#include <transport/FifoMessageDispatchChannel.h>
#include <transport/UPMQCommand.h>

#include <stdexcept>
#include <utility>

using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace upmq::transport;
using namespace upmq::transport;

ConsumerImpl::ConsumerImpl(
    SessionImpl *session, const cms::Destination *destination, ConsumerImpl::Type type, const string &selector, const string &subscr, bool noLocal)
    : _session(session),
      _destination(nullptr),
      _started(false),
      _stoped(false),
      _closed(false),
      _subscribed(false),
      _subscriptioned(false),
      _objectId(UUID::randomUUID().toString()),
      _type(type),
      _subscription(subscr),
      _selector(selector),
      _noLocal(noLocal),
      _deleteDurableConsumer(false),
      _messageListener(nullptr),
      _browserCount(0),
      _onMessageThread(nullptr),
      _activeOnMessageLock(new ReentrantLock()) {
  if (session == nullptr) {
    throw cms::CMSException("invalid session (is null)");
  }

  try {
    _messageQueue = new SimplePriorityMessageDispatchChannel();

    _destination = dynamic_cast<DestinationImpl *>(const_cast<cms::Destination *>(destination));
    if (_destination == nullptr) {
      throw cms::CMSException("invalid destination (is null)");
    }

    _destination = new DestinationImpl(_session, _destination->getName(), _destination->getType());

    subscription();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

ConsumerImpl::~ConsumerImpl() {
  try {
    if (!isClosed()) {
      close();
    }

    delete _destination;
    delete _messageQueue;
    delete _onMessageThread;
    delete _activeOnMessageLock;
  }
  CATCH_ALL_NOTHROW
}

void ConsumerImpl::start() {
  try {
    setStarted(true);
    setStoped(false);
    setClosed(false);

    _messageQueue->start();

    if (_onMessageThread == nullptr && getMessageListener() != nullptr) {
      _onMessageThread = new Thread(this, "UPMQConsumer: " + _objectId);
      _onMessageThread->start();
    }

    subscribe();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::stop() {
  try {
    unsubscribe();

    setStarted(false);
    setStoped(true);

    _messageQueue->clear();
    _messageQueue->stop();

    if (_onMessageThread != nullptr) {
      _messageQueue->close();  // need areceive return before join
      _onMessageThread->join();
      _onMessageThread = nullptr;
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::close() {
  try {
    if (isClosed()) {
      return;
    }

    if (_session) {
      _session->removeConsumer(this);
    }
    if (_destination) {
      _destination->_session = nullptr;
    }

    _activeOnMessageLock->lock();
    _activeOnMessageLock->unlock();

    if (isStarted() || (isStoped() && !isClosed())) {
      try {
        unsubscription();
      }
      CATCH_ALL_NOTHROW
    }

    _messageQueue->clear();
    _messageQueue->close();

    setStarted(false);
    setStoped(true);
    setClosed(true);

    if (_onMessageThread != nullptr) {
      _onMessageThread->join();
      _onMessageThread = nullptr;
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::subscription() {
  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    Proto::Subscription &subscription = request->getSubscription();
    subscription.set_receipt_id(_objectId);
    subscription.set_destination_uri(_destination->getUri());
    subscription.set_session_id(_session->getObjectId());
    subscription.set_no_local(_noLocal);

    if (getSubscription().empty()) {
      if ((_destination->getType() == cms::Destination::QUEUE || _destination->getType() == cms::Destination::TEMPORARY_QUEUE) &&
          getType() != Type::BROWSER) {
        setSubscription(_destination->getUri());
      } else {
        setSubscription(_objectId);
      }
    }

    subscription.set_subscription_name(getSubscription());

    if (!_selector.empty()) {
      subscription.set_selector(_selector);
    }

    subscription.set_durable(getType() == Type::DURABLE_CONSUMER);

    subscription.set_browse(getType() == Type::BROWSER);

    if (!subscription.IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();

    setSubscriptioned(true);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::unsubscription() {
  try {
    if (isSubscribed()) {
      unsubscribe();
    }

    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    Proto::Unsubscription &unsubscription = request->getUnsubscription();
    unsubscription.set_receipt_id(_objectId);
    unsubscription.set_destination_uri(_destination->getUri());
    unsubscription.set_session_id(_session->getObjectId());
    unsubscription.set_subscription_name(getSubscription());

    unsubscription.set_durable(getType() == Type::DURABLE_CONSUMER);

    unsubscription.set_browse(getType() == Type::BROWSER);

    if (!unsubscription.IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();

    setSubscriptioned(false);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::subscribe() {
  try {
    if (isSubscribed()) {
      return;
    }

    if (!isSubscriptioned()) {
      subscription();
    }

    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    Proto::Subscribe &subscribe = request->getSubscribe();
    subscribe.set_receipt_id(_objectId);
    subscribe.set_destination_uri(_destination->getUri());
    subscribe.set_subscription_name(getSubscription());
    subscribe.set_session_id(_session->getObjectId());

    if (getType() == Type::BROWSER) {
      subscribe.set_browse(true);
    } else {
      subscribe.set_browse(false);
    }

    if (!subscribe.IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();

    setSubscribed(true);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::unsubscribe() {
  try {
    if (!isSubscribed()) {
      return;
    }

    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    Proto::Unsubscribe &unsubscribe = request->getUnsubscribe();
    unsubscribe.set_receipt_id(_objectId);
    unsubscribe.set_destination_uri(_destination->getUri());
    unsubscribe.set_subscription_name(getSubscription());
    unsubscribe.set_session_id(_session->getObjectId());
    unsubscribe.set_durable(_deleteDurableConsumer);

    if (getType() == Type::BROWSER) {
      unsubscribe.set_browse(true);
    } else {
      unsubscribe.set_browse(false);
    }

    if (!unsubscribe.IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();

    setSubscribed(false);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

string &ConsumerImpl::getObjectId() { return _objectId; }

void ConsumerImpl::setMessageListener(cms::MessageListener *listener) {
  try {
    checkClosed();

    _activeOnMessageLock->lock();
    _messageListener = listener;
    _activeOnMessageLock->unlock();

    if (getMessageListener() != nullptr) {
      if (_session->_connection->isStarted()) {
        start();
      }
    } else {
      stop();
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MessageListener *ConsumerImpl::getMessageListener() const { return _messageListener; }

std::string ConsumerImpl::getMessageSelector() const { return _selector; }

cms::Message *ConsumerImpl::areceive() {
  try {
    checkClosed();

    Pointer<Command> command;
    cms::Message *message;

    do {
      command = dequeue(-1);
      if (command == nullptr) {
        return nullptr;
      }

      message = commandToMessage(command);
      command.reset(nullptr);
    } while (message->isExpired());

    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Message *ConsumerImpl::receive() {
  try {
    checkClosed();

    Pointer<Command> command;
    cms::Message *message;

    do {
      command = dequeue(-1);
      if (command == nullptr) {
        if (_session != nullptr)
          if (_session->_connection->_transportFailed.get()) throw cms::CMSException("transport failed");
        return nullptr;
      }

      message = commandToMessage(command);
      command.reset(nullptr);
    } while (message->isExpired());

    if (_session->getAcknowledgeMode() != cms::Session::CLIENT_ACKNOWLEDGE) {
      message->acknowledge();
    }

    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Message *ConsumerImpl::receive(int timeout) {
  try {
    checkClosed();

    cms::Message *message;

    do {
      Pointer<Command> command;
      if (timeout == 0) {
        command = dequeue(-1);
      } else {
        command = dequeue(timeout);
      }

      if (command == nullptr) {
        if (_session != nullptr)
          if (_session->_connection->_transportFailed.get()) throw cms::CMSException("transport failed");
        return nullptr;
      }

      message = commandToMessage(std::move(command));
    } while (message->isExpired());

    if (_session->getAcknowledgeMode() != cms::Session::CLIENT_ACKNOWLEDGE) {
      message->acknowledge();
    }

    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Message *ConsumerImpl::receiveNoWait() {
  try {
    checkClosed();

    cms::Message *message;
    Pointer<Command> command;

    do {
      command = dequeue(0);
      if (command == nullptr) {
        if (_session != nullptr)
          if (_session->_connection->_transportFailed.get()) throw cms::CMSException("transport failed");
        return nullptr;
      }

      message = commandToMessage(command);
      command.reset(nullptr);
    } while (message->isExpired());

    if (_session->getAcknowledgeMode() != cms::Session::CLIENT_ACKNOWLEDGE) {
      message->acknowledge();
    }

    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::run() {
  // cout << "onMessage thread start (" + _objectId + ")" << endl;

  cms::Message *msg = nullptr;
  while (isStarted() && !isStoped() && _messageListener != nullptr) {
    try {
      msg = areceive();
      if (msg != nullptr) {
        _activeOnMessageLock->lock();
        if (isStarted() && !isStoped() && _messageListener != nullptr) {
          if (_session->getAcknowledgeMode() != cms::Session::CLIENT_ACKNOWLEDGE) {
            msg->acknowledge();
          }
          _session->syncOnMessage(_messageListener, msg);
        }
        _activeOnMessageLock->unlock();
      }
    }
    // TODO need!!!
    //    catch (cms::IllegalStateException &e) {
    //      continue;
    //    }
    CATCH_ALL_THROW_CMSEXCEPTION
  }

  // cout << "onMessage thread stop (" + _objectId + ")" << endl;
}

void ConsumerImpl::deleteDurableConsumer() { _deleteDurableConsumer = true; }

bool ConsumerImpl::isAlive() const { return (_started && !_closed && !_stoped); }

bool ConsumerImpl::isStarted() const { return _started; }

void ConsumerImpl::setStarted(bool started) { _started = started; }

bool ConsumerImpl::isStoped() const { return _stoped; }

void ConsumerImpl::setStoped(bool stoped) { _stoped = stoped; }

bool ConsumerImpl::isClosed() const { return _closed; }

void ConsumerImpl::setClosed(bool closed) { _closed = closed; }

bool ConsumerImpl::isSubscribed() const { return _subscribed; }

void ConsumerImpl::setSubscribed(bool subscribed) { _subscribed = subscribed; }

bool ConsumerImpl::isSubscriptioned() const { return _subscriptioned; }

void ConsumerImpl::setSubscriptioned(bool subscriptioned) { _subscriptioned = subscriptioned; }

// QueueBrowser
const cms::Queue *ConsumerImpl::getQueue() const { return _destination; }

cms::MessageEnumeration *ConsumerImpl::getEnumeration() {
  // if (_session->_isStarted() && _type == Type::BROWSER) {
  //  start();
  //}

  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getBrowser().set_destination_uri(_destination->getUri());
    request->getBrowser().set_subscription_name(getSubscription());

    if (!request->getBrowser().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    Proto::BrowserInfo response = _session->_connection->syncRequest(request.dynamicCast<Command>())->processBrowserInfo();
    _browserCount = response.message_count();

    return this;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

// MessageEnumeration
bool ConsumerImpl::hasMoreMessages() {
  if (_browserCount == 0) {
    return false;
  } else {
    return true;
  }
}

cms::Message *ConsumerImpl::nextMessage() {
  try {
    cms::Message *message = receive();
    if (message != nullptr) {
      _browserCount--;
    }
    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::checkClosed() {
  if (isClosed()) {
    throw cms::IllegalStateException("cannot perform operation - consumer has been closed");
  }
}

Pointer<Command> ConsumerImpl::dequeue(long long timeout) {
  try {
    return _messageQueue->dequeue(timeout);
  } catch (InterruptedException &) {
    // TODO check
    return Pointer<Command>();
    // TODO:
    // Thread.currentThread().interrupt();
    // throw JMSException
  }
}

void ConsumerImpl::dispatch(const Pointer<Command> &message) {
  try {
    _messageQueue->enqueue(message);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ConsumerImpl::purge() {
  try {
    _messageQueue->stop();
    _messageQueue->clear();
    _messageQueue->start();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

ConsumerImpl::Type ConsumerImpl::getType() const { return _type; }

void ConsumerImpl::setType(ConsumerImpl::Type val) { _type = val; }

std::string ConsumerImpl::getSubscription() const { return _subscription; }

void ConsumerImpl::setSubscription(std::string subscription) { _subscription = std::move(subscription); }

cms::Message *ConsumerImpl::commandToMessage(const Pointer<Command>& comm) {
  UPMQCommand *command = dynamic_cast<UPMQCommand *>(comm.get());
  if (command == nullptr) {
    throw cms::CMSException("error message type");
  }
  cms::Message *message = nullptr;
  switch (command->getMessage().body_type()) {
    case Proto::Body::BODYTYPE_NOT_SET:
      message = new MessageImpl(*command);
      break;
    case Proto::Body::kTextBody:
      message = new TextMessageImpl(*command);
      break;
    case Proto::Body::kMapBody:
      message = new MapMessageImpl(*command);
      break;
    case Proto::Body::kObjectBody:
      message = new ObjectMessageImpl(*command);
      break;
    case Proto::Body::kBytesBody:
      message = new BytesMessageImpl(*command);
      break;
    case Proto::Body::kStreamBody:
      message = new StreamMessageImpl(*command);
      break;
    default:
      throw cms::CMSException("error message type");
  }

  return message;
}

#endif  //__MapMessageImpl_CPP__
