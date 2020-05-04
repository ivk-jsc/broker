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

#include "QueueDestination.h"
#include "Exchange.h"
#include "MiscDefines.h"
#include "QueueSender.h"
#include "Session.h"

namespace upmq {
namespace broker {

QueueDestination::QueueDestination(const Exchange &exchange, const std::string &uri, Destination::Type type) : Destination(exchange, uri, type) {}
QueueDestination::~QueueDestination() {}
void QueueDestination::save(const Session &session, const MessageDataContainer &sMessage) {
  _senders.fixMessageInGroup(sMessage.message().sender_id(), session, sMessage);
  if (session.isTransactAcknowledge() && !_storage.hasTransaction(session)) {
    begin(session);
  }
  TRY_POCO_DATA_EXCEPTION { _storage.save(session, sMessage); }
  CATCH_POCO_DATA_EXCEPTION_NO_INVALID_SQL("can't save message", "", session.currentDBSession->rollbackTX(), ERROR_ON_SAVE_MESSAGE)

  session.currentDBSession->commitTX();

  postNewMessageEvent();
}
void QueueDestination::ack(const Session &session, const MessageDataContainer &sMessage) {
  const Proto::Ack &ack = sMessage.ack();
  const std::string &messageID = ack.message_id();
  const std::string &subscriptionName = ack.subscription_name();

  Storage *storage = &_storage;

  bool browser = isSubscriptionBrowser(subscriptionName);
  if (browser) {
    auto it = _subscriptions.find(subscriptionName);
    if (it.hasValue()) {
      storage = &(it->storage());
    } else {
      throw EXCEPTION("can't find browser subscription", subscriptionName, ERROR_ON_ACK_MESSAGE);
    }
  }

  std::vector<MessageInfo> sentMsgs = storage->getMessagesBelow(session, messageID);
  Destination::doAck(session, sMessage, *storage, browser, sentMsgs);
}
void QueueDestination::commit(const Session &session) {
  Destination::commit(session);
  _storage.commit(session);
  postNewMessageEvent();
}
void QueueDestination::abort(const Session &session) {
  Destination::abort(session);
  _storage.abort(session);
  postNewMessageEvent();
}

Subscription QueueDestination::createSubscription(const std::string &name, const std::string &routingKey, Subscription::Type type) {
  std::string id;
  if (type != Subscription::Type::BROWSER) {
    id = _id;
  }
  return Subscription(*this, id, name, routingKey, type);
}
void QueueDestination::begin(const Session &session) {
  Destination::begin(session);
  _storage.begin(session);
}
void QueueDestination::addSender(const Session &session, const MessageDataContainer &sMessage) {
  const Proto::Sender &sender = sMessage.sender();
  std::unique_ptr<Sender> pSender(new QueueSender(sender.sender_id(), session, *this));
  _senders.addSender(std::move(pSender));
}
void QueueDestination::removeSender(const Session &session, const MessageDataContainer &sMessage) {
  removeSenderByID(session, sMessage.unsender().sender_id());
}
void QueueDestination::removeSenders(const Session &session) {
  _senders.closeGroups(session);
  _senders.removeSenders(session);
}
void QueueDestination::addSendersFromCache(const Session &session, const MessageDataContainer &sMessage, Subscription &subscription) {
  UNUSED_VAR(session);
  UNUSED_VAR(sMessage);
  UNUSED_VAR(subscription);
}
void QueueDestination::removeSenderByID(const Session &session, const std::string &senderID) {
  _senders.closeGroup(senderID, session);
  _senders.removeSender(senderID);
}
}  // namespace broker
}  // namespace upmq
