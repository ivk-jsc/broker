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

#ifndef __MessageProducerImpl_H__
#define __MessageProducerImpl_H__

#include <cms/MessageProducer.h>

#include "DestinationImpl.h"
#include "ProtoHeader.h"
#include "decaf/util/Config.h"

using namespace std;

class ProducerImpl : public cms::MessageProducer {
 public:
  ProducerImpl(SessionImpl *sessionImpl, const cms::Destination *destination);
  ~ProducerImpl() override;

  void send(cms::Message *message) override;
  void send(cms::Message *message, int deliveryMode, int priority, long long timeToLive) override;
  void send(const cms::Destination *destination, cms::Message *message) override;
  void send(const cms::Destination *destination, cms::Message *msg, int deliveryMode, int priority, long long timeToLive) override;

  void setDeliveryMode(int mode) override;
  int getDeliveryMode() const override;

  void setPriority(int priority) override;
  int getPriority() const override;

  void setTimeToLive(long long time) override;
  long long getTimeToLive() const override;

  void setDisableMessageID(bool value) override;
  bool getDisableMessageID() const override;

  void setDisableMessageTimeStamp(bool value) override;
  bool getDisableMessageTimeStamp() const override;

  void close() override;

  SessionImpl *_session;
  DestinationImpl *_destination;
  bool _nullDestProducer;

  string &getObjectId();

  void sender(DestinationImpl *destination);
  void unsender();

  bool isClosed();
  void setClosed(bool closed);

  cms::Destination *getDestination();

  string assignNewId();

 private:
  bool _closed;

  int _defaultDeliveryMode;
  int _defaultPriority;
  long long _defaultTimeToLive;

  bool _isDisableMessageId;
  bool _isDisableMessageTimestamp;

  string _objectId;
  string _lastMessageId;
};

#endif  //__MessageProducerImpl_H__
