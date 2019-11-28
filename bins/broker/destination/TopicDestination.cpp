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

#include "TopicDestination.h"
#include "MiscDefines.h"
#include "TopicSender.h"
#include <Poco/StringTokenizer.h>
#include <Poco/Hash.h>

namespace upmq {
namespace broker {

TopicDestination::TopicDestination(const Exchange &exchange, const std::string &uri, Destination::Type type) : Destination(exchange, uri, type) {
  loadDurableSubscriptions();
}
void TopicDestination::save(const Session &session, const MessageDataContainer &sMessage) {
  session.currentDBSession->commitTX();

  TRY_POCO_DATA_EXCEPTION {
    std::string routingK = routingKey(sMessage.message().destination_uri());
    ParentTopics parentTopics = generateParentTopics(routingK);
    bool needRemoveBody = true;
    for (const auto &topic : parentTopics) {
      if (notifySubscription(topic, session, sMessage)) {
        needRemoveBody = false;
      }
    }
    if (needRemoveBody) {
      const_cast<MessageDataContainer &>(sMessage).removeLinkedFile();
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't save message", "", ERROR_ON_SAVE_MESSAGE)
}
void TopicDestination::ack(const Session &session, const MessageDataContainer &sMessage) {
  const Proto::Ack &ack = sMessage.ack();
  const std::string &messageID = ack.message_id();
  const std::string &subscriptionName = ack.subscription_name();

  if (session.isClientAcknowledge()) {
    _subscriptions.changeForEach([this, &session, &messageID, &sMessage](SubscriptionsList::ItemType::KVPair &pair) {
      std::vector<MessageInfo> sentMsgs = pair.second.storage().getMessagesBelow(session, messageID);
      Destination::doAck(session, sMessage, pair.second.storage(), false, sentMsgs);
    });
  } else {
    auto it = _subscriptions.find(subscriptionName);
    if (it.hasValue()) {
      auto &subs = *it;
      std::vector<MessageInfo> sentMsgs = subs.storage().getMessagesBelow(session, messageID);
      Destination::doAck(session, sMessage, subs.storage(), false, sentMsgs);
    }
  }
}
void TopicDestination::commit(const Session &session) {
  Destination::commit(session);
  _subscriptions.changeForEach([&session](SubscriptionsList::ItemType::KVPair &pair) { pair.second.commit(session); });
}
void TopicDestination::abort(const Session &session) {
  Destination::abort(session);
  _subscriptions.changeForEach([&session](SubscriptionsList::ItemType::KVPair &pair) { pair.second.abort(session); });
}

Subscription TopicDestination::createSubscription(const std::string &name, const std::string &routingKey, Subscription::Type type) {
  return Subscription(*this, "", name, routingKey, type);
}
void TopicDestination::begin(const Session &session) {
  Destination::begin(session);
  _subscriptions.changeForEach([&session](SubscriptionsList::ItemType::KVPair &pair) { pair.second.storage().begin(session, pair.second.id()); });
}
void TopicDestination::addSender(const Session &session, const MessageDataContainer &sMessage) {
  const Proto::Sender &sender = sMessage.sender();
  std::string routingKey = Destination::routingKey(sender.destination_uri());
  {
    upmq::ScopedReadRWLock readRWLock(_routingLock);
    auto item = _routing.find(routingKey);
    if (item != _routing.end()) {
      const MessageDataContainer *dc = &sMessage;
      item->second->notify(&session, dc);
      return;
    }
  }
  upmq::ScopedWriteRWLock writeRWLock(_senderCacheLock);
  _senderCache.insert(std::make_pair(routingKey, sender.sender_id()));
}
void TopicDestination::removeSender(const Session &session, const MessageDataContainer &sMessage) {
  const Proto::Unsender &unsender = sMessage.unsender();
  std::string routingKey = Destination::routingKey(unsender.destination_uri());
  {
    upmq::ScopedReadRWLock readRWLock(_routingLock);
    auto item = _routing.find(routingKey);
    if (item != _routing.end()) {
      const MessageDataContainer *dc = &sMessage;
      item->second->notify(&session, dc);
    }
  }

  upmq::ScopedWriteRWLock writeRWLock(_senderCacheLock);
  auto cacheItem = _senderCache.equal_range(routingKey);

  for (auto itSender = cacheItem.first; itSender != cacheItem.second;) {
    if ((*itSender).second == unsender.sender_id()) {
      itSender = _senderCache.erase(itSender);
    } else {
      ++itSender;
    }
  }
}
void TopicDestination::removeSenders(const Session &session) {
  upmq::ScopedReadRWLock readRWLock(_routingLock);
  const MessageDataContainer *dc = nullptr;
  for (const auto &item : _routing) {
    item.second->notify(&session, dc);
  }
}
void TopicDestination::removeSenderByID(const Session &session, const std::string &senderID) {
  MessageDataContainer messageDataContainer;
  Proto::Unsender &unsender = messageDataContainer.createUnsender(senderID);
  unsender.set_sender_id(senderID);
  unsender.set_session_id(session.id());
  unsender.set_destination_uri(_uri);
  const MessageDataContainer &dclink = messageDataContainer;
  const MessageDataContainer *dc = &dclink;
  upmq::ScopedReadRWLock readRWLock(_routingLock);
  for (const auto &item : _routing) {
    item.second->notify(&session, dc);
  }
}

TopicDestination::ParentTopics TopicDestination::generateParentTopics(const std::string &routingKey) {
  Poco::StringTokenizer topicLevels(routingKey, "/", Poco::StringTokenizer::TOK_TRIM);
  ParentTopics parentTopics;
  size_t levels = topicLevels.count();
  if (levels == 0) {
    return parentTopics;
  }
  while (levels > 0) {
    std::string rKey;
    for (size_t i = 0; i < levels; ++i) {
      rKey.append(topicLevels[i]);
      if (i < (levels - 1)) {
        rKey.append("/");
      }
    }
    parentTopics.emplace_back(std::move(rKey));
    --levels;
  }
  return parentTopics;
}

bool TopicDestination::notifySubscription(const std::string &routingKey, const Session &session, const MessageDataContainer &sMessage) {
  upmq::ScopedReadRWLock readRWLock(_routingLock);
  auto it = _routing.find(routingKey);
  if (it != _routing.end()) {
    const MessageDataContainer *dc = &sMessage;
    it->second->notify(&session, dc);
    return true;
  }
  return false;
}
void TopicDestination::addSendersFromCache(const Session &session, const MessageDataContainer &sMessage, Subscription &subscription) {
  upmq::ScopedReadRWLock readRWLock(_senderCacheLock);
  const std::string &routingKey = subscription.routingKey();
  auto cacheItem = _senderCache.equal_range(routingKey);
  for (auto itSender = cacheItem.first; itSender != cacheItem.second; ++itSender) {
    subscription.addSender(session, sMessage);
  }
}
}  // namespace broker
}  // namespace upmq
