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

#ifndef __SessionImpl_H__
#define __SessionImpl_H__

#include <cms/Session.h>

#include <decaf/util/concurrent/Mutex.h>

#include "ConsumerImpl.h"
#include "DestinationImpl.h"
#include "ProducerImpl.h"
#include "ProtoHeader.h"

using namespace std;
using namespace decaf::util::concurrent;

class SessionImpl : public cms::Session {
 public:
  SessionImpl(ConnectionImpl *connectionImpl, cms::Session::AcknowledgeMode _ackMode);
  ~SessionImpl() override;

  cms::MessageProducer *createProducer(const cms::Destination *destination = nullptr) override;

  cms::MessageConsumer *createConsumer(const cms::Destination *destination) override;
  cms::MessageConsumer *createConsumer(const cms::Destination *destination, const string &selector) override;
  cms::MessageConsumer *createConsumer(const cms::Destination *destination, const string &selector, bool noLocal) override;
  cms::MessageConsumer *createDurableConsumer(const cms::Topic *destination, const string &name, const string &selector, bool noLocal = false) override;

  cms::QueueBrowser *createBrowser(const cms::Queue *queue) override;
  cms::QueueBrowser *createBrowser(const cms::Queue *queue, const string &selector) override;

  cms::Queue *createQueue(const string &queueName) override;
  cms::Topic *createTopic(const string &topicName) override;
  cms::TemporaryQueue *createTemporaryQueue() override;
  cms::TemporaryTopic *createTemporaryTopic() override;

  cms::Message *createMessage() override;
  cms::BytesMessage *createBytesMessage() override;
  cms::BytesMessage *createBytesMessage(const unsigned char *bytes, int bytesSize) override;
  cms::StreamMessage *createStreamMessage() override;
  cms::TextMessage *createTextMessage() override;
  cms::TextMessage *createTextMessage(const string &text) override;
  cms::MapMessage *createMapMessage() override;
  cms::ObjectMessage *createObjectMessage() override;
  cms::ObjectMessage *createObjectMessage(const string &text) override;

  cms::Session::AcknowledgeMode getAcknowledgeMode() const override;

  bool isTransacted() const override;
  void commit() override;
  void rollback() override;

  void recover() override;
  void unsubscribe(const string &name) override;

  void start() override;
  void stop() override;
  void close() override;

  ConnectionImpl *_connection;
  map<string, ProducerImpl *> _producersMap;
  map<string, ConsumerImpl *> _consumersMap;
  map<string, DestinationImpl *> _destinationsMap;

  string &getObjectId();
  void syncOnMessage(cms::MessageListener *messageListener, cms::Message *msg);
  cms::MessageConsumer *_createConsumer(ConsumerImpl::Type consumerType, const cms::Destination *destination, const string &selector, const string &subscription, bool noLocal);

  void addConsumer(ConsumerImpl *consumerImpl);
  void removeConsumer(ConsumerImpl *consumer);

  void addProducer(ProducerImpl *producerImpl);
  void removeProducer(ProducerImpl *producerImpl);

  void addDesination(DestinationImpl *destImpl);
  void removeDesination(DestinationImpl *destImpl);

  bool isAlive() const;
  bool isStarted() const;
  bool isStoped() const;
  bool isClosed() const;

  void setStarted(bool stoped);
  void setStoped(bool stoped);
  void setClosed(bool closed);

  void checkClosed() const;
  void checkTransactional() const;
  void checkNotTransacted() const;

  void session();
  void unsession();

  void destination(const std::string &name);
  void undestination(const std::string &name);

 private:
  Mutex _lock;

  bool _closed;
  bool _started;
  bool _stoped;

  string _objectId;
  cms::Session::AcknowledgeMode _ackMode;
};

#endif  //__SessionImpl_H__
