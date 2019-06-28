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

#ifndef __MessageProducerImpl_CPP__
#define __MessageProducerImpl_CPP__

#include "ProducerImpl.h"

#include "BytesMessageImpl.h"
#include "MapMessageImpl.h"
#include "MessageImpl.h"
#include "StreamMessageImpl.h"
#include "TextMessageImpl.h"

#include <decaf/lang/System.h>
#include <decaf/util/UUID.h>

#include <transport/UPMQCommand.h>

ProducerImpl::ProducerImpl(SessionImpl *sessionImpl, const cms::Destination *destination)
    : _session(sessionImpl),
      _destination(nullptr),
      _closed(false),
      _defaultDeliveryMode(cms::Message::DEFAULT_DELIVERY_MODE),
      _defaultPriority(cms::Message::DEFAULT_MSG_PRIORITY),
      _defaultTimeToLive(cms::Message::DEFAULT_TIME_TO_LIVE),
      _isDisableMessageId(false),
      _isDisableMessageTimestamp(false),
      _objectId(UUID::randomUUID().toString()) {
  if (sessionImpl == nullptr) {
    throw cms::CMSException("invalid session (is null)");
  }

  try {
    if (destination == nullptr) {
      _nullDestProducer = true;
      _destination = new DestinationImpl(_session, EMPTY_STRING);
    } else {
      _nullDestProducer = false;
      _destination = new DestinationImpl(_session, destination->getName(), destination->getType());
      sender(_destination);
    }
    // cout << "+ prod id " << _objectId << endl;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

ProducerImpl::~ProducerImpl() {
  try {
    if (!isClosed()) {
      close();
    }
    if (_destination) {
      delete _destination;
    }
  }
  CATCHALL_NOTHROW
}

void ProducerImpl::sender(DestinationImpl *destination) {
  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getSender().set_receipt_id(_objectId);
    request->getSender().set_destination_uri(destination->getUri());
    request->getSender().set_session_id(_session->getObjectId());
    request->getSender().set_sender_id(_objectId);

    if (!request->getSender().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ProducerImpl::unsender() {
  try {
    Pointer<UPMQCommand> request(new UPMQCommand());
    request->getProtoMessage().set_object_id(_objectId);

    request->getUnsender().set_receipt_id(_objectId);
    request->getUnsender().set_destination_uri(_destination != nullptr ? _destination->getUri() : "");
    request->getUnsender().set_session_id(_session->getObjectId());
    request->getUnsender().set_sender_id(_objectId);

    if (!request->getUnsender().IsInitialized()) {
      throw cms::CMSException("request not initialized");
    }

    _session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ProducerImpl::close() {
  try {
    if (isClosed()) {
      return;
    }

    if (_session != nullptr && _session->_connection != nullptr && !_session->_connection->isClosed()) {
      _session->removeProducer(this);

      try {
        unsender();
      }
      CATCHALL_NOTHROW

      setClosed(true);

      if (_destination) {
        _destination->_session = nullptr;
      }
    }
    // cout << "- prod id " << _objectId << endl;
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ProducerImpl::setDeliveryMode(int mode) { _defaultDeliveryMode = mode; }

int ProducerImpl::getDeliveryMode() const { return _defaultDeliveryMode; }

void ProducerImpl::setPriority(int priority) { _defaultPriority = priority; }

int ProducerImpl::getPriority() const { return _defaultPriority; }

void ProducerImpl::setTimeToLive(long long time) { _defaultTimeToLive = time; }

long long ProducerImpl::getTimeToLive() const { return _defaultTimeToLive; }

void ProducerImpl::setDisableMessageID(bool value) { _isDisableMessageId = value; }

bool ProducerImpl::getDisableMessageID() const { return _isDisableMessageId; }

void ProducerImpl::setDisableMessageTimeStamp(bool value) { _isDisableMessageTimestamp = value; }

bool ProducerImpl::getDisableMessageTimeStamp() const { return _isDisableMessageTimestamp; }

void ProducerImpl::send(cms::Message *message) {
  try {
    send(message, getDeliveryMode(), getPriority(), getTimeToLive());
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ProducerImpl::send(cms::Message *message, int deliveryMode, int priority, long long timeToLive) {
  try {
    send(getDestination(), message, deliveryMode, priority, timeToLive);
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ProducerImpl::send(const cms::Destination *destination, cms::Message *message) {
  try {
    send(destination, message, getDeliveryMode(), getPriority(), getTimeToLive());
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void ProducerImpl::send(const cms::Destination *destination, cms::Message *msg, int deliveryMode, int priority, long long timeToLive) {
  try {
    if (isClosed()) {
      throw cms::IllegalStateException("cannot perform operation - producer has been closed");
    }

    if (destination == nullptr) {
      if (_destination->getName().empty()) {
        throw cms::UnsupportedOperationException("destination is null");
      } else {
        throw cms::InvalidDestinationException("destination is null");
      }
    }
    const DestinationImpl *dest = dynamic_cast<const DestinationImpl *>(destination);
    if (!dest) {
      throw cms::InvalidDestinationException("destination is invalid");
    }
    if (_destination->getUri() != dest->getUri()) {
      if (_nullDestProducer) {
        delete _destination;
        _destination = new DestinationImpl(_session, dest->getName(), dest->getType());
        sender(_destination);
      } else {
        throw cms::UnsupportedOperationException("destination not equal");
      }
    }

    // Message
    UPMQCommand *command = dynamic_cast<UPMQCommand *>(msg);
    if (!command) {
      throw cms::MessageFormatException("can't cast to UPMQCommand");
    }
    command->_header->mutable_message()->set_sender_id(_objectId);
    command->_header->mutable_message()->set_session_id(_session->getObjectId());
    command->_header->mutable_message()->set_persistent(deliveryMode == cms::DeliveryMode::PERSISTENT);
    command->_header->mutable_message()->set_priority(priority);
    auto uuid = assignNewId();
    while (_lastMessageId == uuid) {
      uuid = assignNewId();
    }
    _lastMessageId = std::move(uuid);
    command->_header->mutable_message()->set_message_id(_lastMessageId);
    command->_header->mutable_message()->set_receipt_id(command->_header->mutable_message()->message_id());

    long long currTimeMillis = 0;
    if (timeToLive > 0 || !_isDisableMessageTimestamp) {
      currTimeMillis = System::currentTimeMillis();
    }

    if (timeToLive > 0) {
      command->_header->mutable_message()->set_timetolive(timeToLive);
      command->_header->mutable_message()->set_expiration(timeToLive + currTimeMillis);
    } else {
      command->_header->mutable_message()->set_timetolive(0);
      command->_header->mutable_message()->set_expiration(0);
    }

    if (!_isDisableMessageTimestamp) {
      command->_header->mutable_message()->set_timestamp(currTimeMillis);
    }

    // Destination
    msg->setCMSDestination(destination);

    // Send
    if (!command->_header->message().IsInitialized()) {
      throw cms::CMSException("message not initialized");
    }

    Pointer<UPMQCommand> request(command, onDeleteFunc);
    request->getProtoMessage().set_object_id(_objectId);

    _session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();
    request.release();
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

string &ProducerImpl::getObjectId() { return _objectId; }

bool ProducerImpl::isClosed() { return _closed; }

void ProducerImpl::setClosed(bool closed) { _closed = closed; }

cms::Destination *ProducerImpl::getDestination() { return _destination; }

string ProducerImpl::assignNewId() { return "ID:" + UUID::randomUUID().toString(); }

#endif  //__MessageProducerImpl_CPP__
