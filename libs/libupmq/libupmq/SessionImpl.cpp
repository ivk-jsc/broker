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

#ifndef __SessionImpl_CPP__
#define __SessionImpl_CPP__

#include "SessionImpl.h"

#include "DestinationImpl.h"

#include "BytesMessageImpl.h"
#include "MapMessageImpl.h"
#include "MessageImpl.h"
#include "ObjectMessageImpl.h"
#include "StreamMessageImpl.h"
#include "TextMessageImpl.h"

#include "ConsumerImpl.h"
#include "ProducerImpl.h"

#include <decaf/util/UUID.h>
#include <transport/UPMQCommand.h>

SessionImpl::SessionImpl(ConnectionImpl *connectionImpl, cms::Session::AcknowledgeMode ackMode)
    : _connection(connectionImpl), _closed(false), _started(false), _stoped(false), _objectId(UUID::randomUUID().toString()), _ackMode(ackMode) {
  if (connectionImpl == nullptr) {
    throw cms::CMSException("invalid parameter (is null)");
  }

  try {
    session();
    if (_connection->isStarted()) {
      start();
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

SessionImpl::~SessionImpl() {
  try {
    if (!isClosed()) {
      close();
    }
  }
  CATCH_ALL_NOTHROW
}

void SessionImpl::start() {
  try {
    checkClosed();

    setStarted(true);
    setStoped(false);
    setClosed(false);

    for (std::map<std::string, ConsumerImpl *>::iterator it = _consumersMap.begin(); it != _consumersMap.end(); ++it) {
      it->second->start();
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::stop() {
  try {
    checkClosed();

    for (std::map<std::string, ConsumerImpl *>::iterator it = _consumersMap.begin(); it != _consumersMap.end(); ++it) {
      it->second->stop();
    }

    setStarted(false);
    setStoped(true);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::close() {
  try {
    if (isClosed()) {
      return;
    }

    if (_connection) {
      _connection->_sessionsMap.erase(_objectId);
    }

    std::vector<ConsumerImpl *> cons;
    for (auto& it : _consumersMap) {
      cons.push_back(it.second);
    }
    for (auto value : cons) {
      value->close();
      value->_session = nullptr;
    }

    std::vector<ProducerImpl *> prods;
    for (auto& it : _producersMap) {
      prods.push_back(it.second);
    }
    for (auto value : prods) {
      value->close();
      value->_session = nullptr;
    }

    std::vector<DestinationImpl *> dests;
    for (auto& it : _destinationsMap) {
      dests.push_back(it.second);
    }
    for (auto value : dests) {
      value->destroy();
    }
    
    try {
      unsession();
    }
    CATCH_ALL_NOTHROW

    setStarted(false);
    setStoped(true);
    setClosed(true);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MessageConsumer *SessionImpl::createConsumer(const cms::Destination *destination) {
  try {
    return _createConsumer(ConsumerImpl::Type::CONSUMER, destination, EMPTY_STRING, EMPTY_STRING, false);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MessageConsumer *SessionImpl::createConsumer(const cms::Destination *destination, const string &selector) {
  try {
    return _createConsumer(ConsumerImpl::Type::CONSUMER, destination, selector, EMPTY_STRING, false);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MessageConsumer *SessionImpl::createConsumer(const cms::Destination *destination, const string &selector, bool noLocal) {
  try {
    return _createConsumer(ConsumerImpl::Type::CONSUMER, destination, selector, EMPTY_STRING, noLocal);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MessageConsumer *SessionImpl::createDurableConsumer(const cms::Topic *destination, const string &subscription, const string &selector, bool noLocal /*= false*/) {
  try {
    return _createConsumer(ConsumerImpl::Type::DURABLE_CONSUMER, destination, selector, subscription, noLocal);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MessageConsumer *SessionImpl::_createConsumer(ConsumerImpl::Type consumerType, const cms::Destination *destination, const string &selector, const string &subscription, bool noLocal) {
  try {
    checkClosed();

    if (destination == nullptr) {
      throw cms::InvalidDestinationException("don't understand null destinations");
    }

    ConsumerImpl *consumerImpl = new ConsumerImpl(this, destination, consumerType, selector, subscription, noLocal);
    addConsumer(consumerImpl);
    if (isStarted()) {
      consumerImpl->start();
    }
    return consumerImpl;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MessageProducer *SessionImpl::createProducer(const cms::Destination *destination /*= NULL*/) {
  try {
    checkClosed();

    ProducerImpl *producerImpl = new ProducerImpl(this, destination);
    addProducer(producerImpl);
    return producerImpl;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::QueueBrowser *SessionImpl::createBrowser(const cms::Queue *queue) {
  checkClosed();

  try {
    ConsumerImpl *consumerImpl = new ConsumerImpl(this, queue, ConsumerImpl::Type::BROWSER, EMPTY_STRING, EMPTY_STRING, false);
    addConsumer(consumerImpl);
    if (isStarted()) {
      consumerImpl->start();
    }
    return consumerImpl;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::QueueBrowser *SessionImpl::createBrowser(const cms::Queue *queue, const std::string &selector) {
  try {
    checkClosed();

    ConsumerImpl *consumerImpl = new ConsumerImpl(this, queue, ConsumerImpl::Type::BROWSER, selector, EMPTY_STRING, false);
    addConsumer(consumerImpl);
    if (isStarted()) {
      consumerImpl->start();
    }
    return consumerImpl;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Queue *SessionImpl::createQueue(const string &queueName) {
  try {
    checkClosed();

    return (cms::Queue *)new DestinationImpl(this, queueName, cms::Destination::QUEUE);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Topic *SessionImpl::createTopic(const string &topicName) {
  try {
    checkClosed();

    return (cms::Topic *)new DestinationImpl(this, topicName, cms::Destination::TOPIC);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::TemporaryQueue *SessionImpl::createTemporaryQueue() {
  try {
    checkClosed();

    DestinationImpl *dest = new DestinationImpl(this, UUID::randomUUID().toString(), cms::Destination::TEMPORARY_QUEUE);
    destination(dest->getUri());
    addDesination(dest);
    return dest;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::TemporaryTopic *SessionImpl::createTemporaryTopic() {
  try {
    checkClosed();

    DestinationImpl *dest = new DestinationImpl(this, UUID::randomUUID().toString(), cms::Destination::TEMPORARY_TOPIC);
    destination(dest->getUri());
    addDesination(dest);
    return dest;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Message *SessionImpl::createMessage() {
  try {
    checkClosed();

    return new MessageImpl();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::BytesMessage *SessionImpl::createBytesMessage() {
  try {
    checkClosed();

    return new BytesMessageImpl();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::BytesMessage *SessionImpl::createBytesMessage(const unsigned char *bytes, int bytesSize) {
  try {
    checkClosed();

    BytesMessageImpl *message = new BytesMessageImpl();
    message->setBodyBytes(bytes, bytesSize);
    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::StreamMessage *SessionImpl::createStreamMessage() {
  try {
    checkClosed();

    return new StreamMessageImpl();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::TextMessage *SessionImpl::createTextMessage() {
  try {
    checkClosed();

    return new TextMessageImpl();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::TextMessage *SessionImpl::createTextMessage(const string &text) {
  try {
    checkClosed();

    TextMessageImpl *message = new TextMessageImpl();
    message->setText(text);
    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::MapMessage *SessionImpl::createMapMessage() {
  try {
    checkClosed();

    return new MapMessageImpl();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::ObjectMessage *SessionImpl::createObjectMessage() {
  try {
    checkClosed();

    return new ObjectMessageImpl();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::ObjectMessage *SessionImpl::createObjectMessage(const string &text) {
  try {
    checkClosed();

    ObjectMessageImpl *message = new ObjectMessageImpl();
    message->setObjectBytes(text);
    return message;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Session::AcknowledgeMode SessionImpl::getAcknowledgeMode() const {
  checkClosed();

  return _ackMode;
}

bool SessionImpl::isTransacted() const {
  checkClosed();

  if (_ackMode == Session::SESSION_TRANSACTED) {
    return true;
  }
  return false;
}

void SessionImpl::unsubscribe(const string &name) {
  try {
    checkClosed();

    for (auto& it : _consumersMap) {
      ConsumerImpl* consumerImpl = it.second;
      if (consumerImpl->getType() == ConsumerImpl::Type::DURABLE_CONSUMER && !consumerImpl->getSubscription().empty() && (consumerImpl->getSubscription() == name)) {
        consumerImpl->deleteDurableConsumer();
        consumerImpl->unsubscription();
      }
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::commit() {
  try {
    checkClosed();
    checkTransactional();

    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getCommit().set_session_id(_objectId);
    request->getCommit().set_receipt_id(_objectId);
    if (!request->getCommit().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::rollback() {
  try {
    checkClosed();
    checkTransactional();

    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getAbort().set_session_id(_objectId);
    request->getAbort().set_receipt_id(_objectId);
    if (!request->getAbort().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    for (std::map<std::string, ConsumerImpl *>::iterator it = _consumersMap.begin(); it != _consumersMap.end(); ++it) {
      it->second->purge();
    }

    _connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::recover() {
  try {
    checkClosed();
    checkNotTransacted();

    stop();
    for (std::map<std::string, ConsumerImpl *>::iterator it = _consumersMap.begin(); it != _consumersMap.end(); ++it) {
      it->second->purge();
    }
    start();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

string &SessionImpl::getObjectId() { return _objectId; }

bool SessionImpl::isAlive() const { return (_started && !_closed && !_stoped); }

bool SessionImpl::isStarted() const { return _started; }

void SessionImpl::setStarted(bool started) { _started = started; }

bool SessionImpl::isStoped() const { return _stoped; }

void SessionImpl::setStoped(bool stoped) { _stoped = stoped; }

bool SessionImpl::isClosed() const { return _closed; }

void SessionImpl::setClosed(bool closed) { _closed = closed; }

void SessionImpl::checkClosed() const {
  if (isClosed()) {
    throw cms::IllegalStateException();
  }
}

void SessionImpl::checkTransactional() const {
  if (!isTransacted()) {
    throw cms::IllegalStateException();
  }
}

void SessionImpl::checkNotTransacted() const {
  if (isTransacted()) {
    throw cms::IllegalStateException();
  }
}

void SessionImpl::addConsumer(ConsumerImpl *consumerImpl) {
  synchronized(&_lock) {
    _consumersMap.insert(make_pair(consumerImpl->getObjectId(), consumerImpl));
    _connection->addDispatcher(consumerImpl);
  }
}

void SessionImpl::removeConsumer(ConsumerImpl *consumerImpl) {
  synchronized(&_lock) {
    _consumersMap.erase(consumerImpl->getObjectId());
    _connection->removeDispatcher(consumerImpl);
  }
}

void SessionImpl::addProducer(ProducerImpl *producerImpl) {
  synchronized(&_lock) { _producersMap.insert(make_pair(producerImpl->getObjectId(), producerImpl)); }
}

void SessionImpl::removeProducer(ProducerImpl *producerImpl) {
  synchronized(&_lock) { _producersMap.erase(producerImpl->getObjectId()); }
}

void SessionImpl::addDesination(DestinationImpl *destImpl) {
  synchronized(&_lock) { _destinationsMap.insert(make_pair(destImpl->getName(), destImpl)); }
}

void SessionImpl::removeDesination(DestinationImpl *destImpl) {
  synchronized(&_lock) { _destinationsMap.erase(destImpl->getName()); }
}

void SessionImpl::syncOnMessage(cms::MessageListener *messageListener, cms::Message *msg) {
  synchronized(&_lock) { messageListener->onMessage(msg); }
}

void SessionImpl::session() {
  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    Proto::Session &session = request->getSession();
    session.set_session_id(_objectId);
    session.set_connection_id(_connection->getObjectId());
    session.set_receipt_id(_objectId);
    session.set_acknowledge_type((Proto::Acknowledge)_ackMode);

    if (!session.IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::unsession() {
  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getUnsession().set_session_id(_objectId);
    request->getUnsession().set_receipt_id(_objectId);

    if (!request->getUnsession().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::destination(const std::string &name) {
  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getDestination().set_destination_uri(name);
    request->getDestination().set_receipt_id(_objectId);

    if (!request->getDestination().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void SessionImpl::undestination(const std::string &name) {
  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getUndestination().set_destination_uri(name);
    request->getUndestination().set_receipt_id(_objectId);

    if (!request->getUndestination().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

#endif  //__SessionImpl_CPP__
