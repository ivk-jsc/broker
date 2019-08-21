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

#ifndef __MessageConsumerImpl_H__
#define __MessageConsumerImpl_H__

#include <cms/MessageConsumer.h>

#include <decaf/lang/Pointer.h>
#include <decaf/util/concurrent/locks/ReentrantLock.h>
#include <transport/Command.h>
#include <transport/Dispatcher.h>
#include <transport/MessageDispatchChannel.h>
#include <transport/UPMQCommand.h>

#include "DestinationImpl.h"
#include "ProtoHeader.h"

using namespace std;
using namespace decaf::lang;
using namespace decaf::util::concurrent::locks;
using namespace upmq::transport;
using namespace upmq::transport;

class ConsumerImpl : public cms::MessageConsumer, public cms::QueueBrowser, cms::MessageEnumeration, Dispatcher, Runnable {
 public:
  enum class Type {
    CONSUMER,
    DURABLE_CONSUMER,
    BROWSER,
  };

  enum class Receive { RECEIVE = 1, RECEIVETIMEOUT, RECEIVENOWAIT };

  ConsumerImpl(SessionImpl *sessionImpl,
               const cms::Destination *destination,
               ConsumerImpl::Type clientType,
               const string &selector,
               const string &subscription,
               bool noLocal);
  ~ConsumerImpl() override;

  cms::Message *receive() override;
  cms::Message *receive(int millisecs) override;
  cms::Message *receiveNoWait() override;
  cms::Message *areceive();

  void setMessageListener(cms::MessageListener *listener) override;
  cms::MessageListener *getMessageListener() const override;
  string getMessageSelector() const override;

  const cms::Queue *getQueue() const override;
  cms::MessageEnumeration *getEnumeration() override;
  bool hasMoreMessages() override;
  cms::Message *nextMessage() override;

  void close() override;
  void start() override;
  void stop() override;

  SessionImpl *_session;
  DestinationImpl *_destination;

  string &getObjectId();

  void checkClosed();

  virtual void dispatch(const decaf::lang::Pointer<upmq::transport::Command> &message) override;

  void run() override;

  bool isAlive() const;
  bool isStarted() const;
  bool isStoped() const;
  bool isClosed() const;
  bool isSubscribed() const;
  bool isSubscriptioned() const;

  void setStarted(bool started);
  void setStoped(bool stoped);
  void setClosed(bool closed);
  void setSubscribed(bool subscribed);
  void setSubscriptioned(bool subscriptioned);

  Pointer<Command> dequeue(long long timeout);

  void subscription();
  void unsubscription();

  void subscribe();
  void unsubscribe();

  void deleteDurableConsumer();

  ConsumerImpl::Type getType() const;
  void setType(ConsumerImpl::Type val);

  std::string getSubscription() const;
  void setSubscription(std::string val);

  void purge();

  MessageDispatchChannel *_messageQueue;

 private:
  bool _started;
  bool _stoped;
  bool _closed;
  bool _subscribed;
  bool _subscriptioned;

  string _objectId;
  ConsumerImpl::Type _type;

  string _subscription;
  string _selector;
  bool _noLocal;

  bool _deleteDurableConsumer;

  cms::MessageListener *_messageListener;

  long long _browserCount;

  Thread *_onMessageThread;
  ReentrantLock *_activeOnMessageLock;

  cms::Message *commandToMessage(Pointer<Command> pointer);
};

#endif  //__MessageConsumerImpl_H__
