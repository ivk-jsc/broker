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

#ifndef BROKER_QUEUEDESTINATION_H
#define BROKER_QUEUEDESTINATION_H

#include "Destination.h"
#include "SenderList.h"
#include <Poco/Logger.h>

namespace upmq {
namespace broker {

class QueueDestination : public Destination {
 private:
  SenderList _senders;
  mutable Poco::Logger *log;

 public:
  QueueDestination(const Exchange &exchange, const std::string &uri, Destination::Type type = Destination::Type::QUEUE);
  ~QueueDestination() override;
  void save(const Session &session, const MessageDataContainer &sMessage) override;
  void ack(const Session &session, const MessageDataContainer &sMessage) override;
  void begin(const Session &session) override;
  void commit(const Session &session) override;
  void abort(const Session &session) override;
  Subscription createSubscription(const std::string &name, const std::string &routingKey, Subscription::Type type) override;
  void addSendersFromCache(const Session &session, const MessageDataContainer &sMessage, Subscription &subscription) override;
  void addSender(const Session &session, const MessageDataContainer &sMessage) override;
  void removeSender(const Session &session, const MessageDataContainer &sMessage) override;
  void removeSenders(const Session &session) override;
  void removeSenderByID(const Session &session, const std::string &senderID) override;
};
}  // namespace broker
}  // namespace upmq
#endif  // BROKER_QUEUEDESTINATION_H
