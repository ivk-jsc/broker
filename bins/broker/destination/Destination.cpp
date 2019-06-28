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

#include "Destination.h"
#include <Poco/Delegate.h>
#include <Poco/StringTokenizer.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/URI.h>
#include "Connection.h"
#include "Exchange.h"
#include "MiscDefines.h"
#include "fake_cpp14.h"
#include "NextBindParam.h"

namespace upmq {
namespace broker {

Destination::Destination(const Exchange &exchange, const std::string &uri, Type type)
    : _id(getStoredDestinationID(exchange, Exchange::mainDestinationPath(uri), type)),
      _uri(uri),
      _name(Exchange::mainDestinationPath(uri)),
      _storage(_id),
      _type(type),
      _exchange(exchange),
      _subscriptionsT("\"" + _id + "_subscriptions\"") {
  _storage.setParent(this);
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(_id);

  createSubscriptionsTable(dbSession);

  dbSession.commitTX();
}
Destination::~Destination() {
  try {
    std::stringstream sql;
    if (!isTemporary()) {
      sql << "update " << _exchange.destinationsT() << " set subscriptions_count = 0"
          << " where id = \'" << _id << "\'"
          << ";";
      TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
      CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't update subscription count", sql.str(), ERROR_UNKNOWN)
    }
    {
      upmq::ScopedWriteRWLock writeRWLock(_subscriptionsLock);
      for (auto &subs : _subscriptions) {
        unsubscribeFromNotify((subs.second));
        subs.second.destroy();
      }
    }
    if (isTemporary()) {
      sql << "drop table if exists " << _subscriptionsT << ";" << non_std_endl;
      TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
      CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't update subscription count", sql.str(), ERROR_UNKNOWN)
      sql.str("");
      sql << "delete from " << _exchange.destinationsT() << " where id = \'" << _id << "\'"
          << ";";
      TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
      CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't update subscription count", sql.str(), ERROR_UNKNOWN)
      _storage.dropTables();
    }
  } catch (...) {
    // TODO : make log
  }
}
void Destination::createSubscriptionsTable(storage::DBMSSession &dbSession) {
  std::stringstream sql;
  sql << "create table if not exists " << _subscriptionsT << "("
      << " id text not null primary key"
      << ",name text not null unique"
      << ",type int not null"
      << ",routing_key text"
      << ",create_time timestamp not null default current_timestamp"
      << ")"
      << ";";

  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION("can't init destination", sql.str(), dbSession.close();, ERROR_DESTINATION)
}
std::string Destination::getStoredDestinationID(const Exchange &exchange, const std::string &name, Destination::Type type) {
  std::string id = Poco::UUIDGenerator::defaultGenerator().createRandom().toString();
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(id);

  NextBindParam nextParam;

  std::stringstream sql;
  sql << "select id from " << exchange.destinationsT() << " where name = " << nextParam() << " and type = " << static_cast<int>(type) << ";";

  std::string tempId;
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::useRef(name), Poco::Data::Keywords::into(tempId), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_NO_INVALID_SQL("can't init destination", sql.str(), ;, ERROR_DESTINATION)

  if (tempId.empty()) {
    saveDestinationId(id, dbSession, exchange, name, type);
  } else {
    id = tempId;
  }
  dbSession.commitTX();

  return id;
}
void Destination::saveDestinationId(const std::string &id, storage::DBMSSession &dbSession, const Exchange &exchange, const std::string &name, Destination::Type type) {
  NextBindParam nextParam;

  std::stringstream sql;
  sql << "insert into " << exchange.destinationsT() << " ("
      << "id, name, type"
      << ")"
      << " values "
      << "("
      << "\'" << id << "\'"
      << "," << nextParam() << "," << static_cast<int>(type) << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::useRef(name), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't init destination", sql.str(), ERROR_DESTINATION)
}
void Destination::subscribe(const MessageDataContainer &sMessage) {
  const Proto::Subscribe &subscribe = sMessage.subscribe();
  const std::string &name = subscribe.subscription_name();
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  auto it = _subscriptions.find(name);
  if (it != _subscriptions.end()) {
    Consumer consumer = Consumer::makeFakeConsumer();
    consumer.clientID = sMessage.clientID;
    consumer.tcpNum = sMessage.handlerNum;
    consumer.session.id = subscribe.session_id();
    consumer.id = Consumer::genConsumerID(consumer.clientID, std::to_string(consumer.tcpNum), consumer.session.id, "");
    it->second.start(consumer);
  } else {
    throw EXCEPTION("subscription not found", name, ERROR_ON_SUBSCRIBE);
  }
}
void Destination::unsubscribe(const MessageDataContainer &sMessage) {
  const Proto::Unsubscribe &unsubscribe = sMessage.unsubscribe();
  const std::string &name = unsubscribe.subscription_name();
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  auto it = _subscriptions.find(name);
  if (it != _subscriptions.end()) {
    Consumer consumer = Consumer::makeFakeConsumer();
    consumer.clientID = sMessage.clientID;
    consumer.tcpNum = sMessage.handlerNum;
    consumer.session.id = unsubscribe.session_id();
    consumer.id = Consumer::genConsumerID(consumer.clientID, std::to_string(consumer.tcpNum), consumer.session.id, "");
    it->second.stop(consumer);
    it->second.recover(consumer);
  } else {
    //    throw EXCEPTION("subscription not found", name, ERROR_ON_UNSUBSCRIBE);
  }
}
void Destination::subscribeOnNotify(Subscription &subscription) const {
  _routing.insert(std::make_pair(subscription.routingKey(), std::make_unique<Poco::FIFOEvent<const MessageDataContainer *>>()));
  *_routing[subscription.routingKey()] += Poco::delegate(&subscription, &Subscription::onEvent);
  subscription.setHasNotify(true);
}
void Destination::unsubscribeFromNotify(Subscription &subscription) const {
  auto item = _routing.find(subscription.routingKey());
  if (item != _routing.end()) {
    *item->second -= Poco::delegate(&subscription, &Subscription::onEvent);
    subscription.setHasNotify(false);
    if (!item->second->hasDelegates()) {
      _routing.erase(item);
    }
  }
}
void Destination::eraseSubscription(SubscriptionsList::iterator &it) {
  if (it != _subscriptions.end()) {
    _subscriptions.erase(it);
  }
}
const std::string &Destination::id() const { return _id; }
const std::string &Destination::uri() const { return _uri; }
bool Destination::isSubscriptionExists(const std::string &name) const {
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  auto it = _subscriptions.find(name);
  return it != _subscriptions.end();
}
bool Destination::isSubscriptionBrowser(const std::string &name) const {
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  auto it = _subscriptions.find(name);
  if (it != _subscriptions.end()) {
    return it->second.isBrowser();
  }
  return false;
}
Subscription &Destination::subscription(const Session &session, const MessageDataContainer &sMessage) {
  Subscription::LocalMode localMode = Subscription::LocalMode::DEFAULT;
  const Proto::Subscription &subscription = sMessage.subscription();
  if (subscription.no_local()) {
    localMode = Subscription::LocalMode::IS_NO_LOCAL;
  }
  Subscription::Type type = Subscription::Type::SIMPLE;
  if (subscription.durable()) {
    type = Subscription::Type::DURABLE;
  } else if (subscription.browse()) {
    type = Subscription::Type::BROWSER;
  }
  std::string name = subscription.subscription_name();
  if (name.empty()) {
    throw EXCEPTION("subscription name is empty", "subscription", ERROR_ON_SUBSCRIPTION);
  }
  const std::string &uri = subscription.destination_uri();

  upmq::ScopedWriteRWLock writeRWLock(_subscriptionsLock);
  auto it = _subscriptions.find(name);
  if (it == _subscriptions.end()) {
    std::string routingK = routingKey(uri);

    auto item = _subscriptions.emplace(name, createSubscription(name, routingK, type));
    item.first->second.setConsumerMode(getConsumerMode(uri));
    subscribeOnNotify(item.first->second);
    addSendersFromCache(session, sMessage, item.first->second);

    addS2Subs(session.id(), name);
    it = _subscriptions.find(name);
  }
  if (!it->second.isInited()) {
    addS2Subs(session.id(), name);
    it->second.setInited(true);
  }
  if (isTopicFamily() && session.isTransactAcknowledge()) {
    it->second.storage().begin(session, it->second.id());
  }
  it->second.addClient(session, sMessage.handlerNum, sMessage.objectID(), subscription.selector(), localMode);
  return it->second;
}
Subscription::ConsumerMode Destination::getConsumerMode(const std::string &uri) {
  Poco::URI tURI(uri);
  Poco::URI::QueryParameters parameters = tURI.getQueryParameters();
  if (!parameters.empty()) {
    auto it = std::find_if(parameters.begin(), parameters.end(), [](const Poco::URI::QueryParameters::value_type &pair) { return (pair.first == "subs-mode" && pair.second == "round-robin"); });
    if (it != parameters.end()) {
      return Subscription::ConsumerMode::ROUND_ROBIN;
    }
  }
  return Subscription::ConsumerMode::EXCLUSIVE;
}
bool Destination::isTemporary() const { return (_type == Type::TEMPORARY_QUEUE) || (_type == Type::TEMPORARY_TOPIC); }
bool Destination::isTopic() const { return _type == Type::TOPIC; }
bool Destination::isQueue() const { return _type == Type::QUEUE; }
bool Destination::isTempTopic() const { return _type == Type::TEMPORARY_TOPIC; }
bool Destination::isTempQueue() const { return _type == Type::TEMPORARY_QUEUE; }
bool Destination::isTopicFamily() const { return isTopic() || isTempTopic(); }
bool Destination::isQueueFamily() const { return isQueue() || isTempQueue(); }
std::string Destination::routingKey(const std::string &uri) {
  Poco::StringTokenizer URI(uri, ":", Poco::StringTokenizer::TOK_TRIM);
  std::string routingKey = URI[1];
  routingKey = Poco::replace(routingKey, "//", " ");
  DestinationFactory::removeParamsAndFragmentFromURI(routingKey);
  Poco::trimInPlace(routingKey);
  return routingKey;
}
void Destination::save(const Session &session, const MessageDataContainer &sMessage) {
  UNUSED_VAR(session);
  UNUSED_VAR(sMessage);
}
void Destination::ack(const Session &session, const MessageDataContainer &sMessage) {
  UNUSED_VAR(session);
  UNUSED_VAR(sMessage);
}
const std::string &Destination::subscriptionsT() const { return _subscriptionsT; }
void Destination::begin(const Session &session) {
  if (session.stateStack().front() == Session::State::BEGIN) {
    return;
  }
}
void Destination::commit(const Session &session) {
  if (session.stateStack().front() == Session::State::COMMIT) {
    return;
  }
}

void Destination::abort(const Session &session) {
  if (session.stateStack().front() == Session::State::ABORT) {
    return;
  }
  resetConsumersCache();
}

void Destination::resetConsumersCache() {
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  for (auto &subs : _subscriptions) {
    subs.second.resetConsumersCache();
  }
}

size_t Destination::subscriptionsCount() const {
  size_t result = 0;
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  if (isQueueFamily()) {
    result = _subscriptions.size();
  } else {
    for (const auto &subs : _subscriptions) {
      TopicDestination::ParentTopics parentTopics = TopicDestination::generateParentTopics(subs.second.routingKey());  //_uri
      for (const auto &routing : _routing) {
        for (const auto &topic : parentTopics) {
          if (routing.first.find(topic) != std::string::npos) {
            ++result;
          }
        }
      }
    }
  }
  return result;
}
size_t Destination::subscriptionsTrueCount(bool noLock) const {
  if (noLock) {
    return _subscriptions.size();
  }
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  return _subscriptions.size();
}
const std::string &Destination::name() const { return _name; }
void Destination::removeMessageOrGroup(const Session &session, Storage &storage, const MessageInfo &msg, message::GroupStatus groupStatus) {
  if (groupStatus == message::LAST_IN_GROUP) {
    storage.removeGroupMessage(msg.tuple.get<message::field_group_id.position>().value(), session);
  }

  if (session.currentDBSession == nullptr) {
    storage::DBMSSession dbmsSession = dbms::Instance().dbmsSession();
    storage.removeMessage(msg.tuple.get<message::field_message_id.position>(), dbmsSession);
  } else {
    storage.removeMessage(msg.tuple.get<message::field_message_id.position>(), *session.currentDBSession);
  }
}
void Destination::doAck(const Session &session, const MessageDataContainer &sMessage, Storage &storage, bool browser, const std::vector<MessageInfo> &messages) {
  message::GroupStatus groupStatus = message::NOT_IN_GROUP;
  for (const auto &msg : messages) {
    groupStatus = getMsgGroupStatus(msg);
    if (session.isClientAcknowledge()) {
      removeMessageOrGroup(session, storage, msg, groupStatus);
    } else if (session.isTransactAcknowledge() || browser) {
      storage.setMessageToDelivered(msg.tuple.get<message::field_message_id.position>());
    } else {
      if (groupStatus == message::ONE_OF_GROUP) {
        storage.setMessageToDelivered(msg.tuple.get<message::field_message_id.position>());
      } else {
        removeMessageOrGroup(session, storage, msg, groupStatus);
      }
    }

    if (!session.isClientAcknowledge()) {
      increaseNotAcknowledged(sMessage.objectID());
    } else {
      increaseNotAcknowledgedAll();
    }
  }
  postNewMessageEvent();
}
message::GroupStatus Destination::getMsgGroupStatus(const MessageInfo &msg) const {
  message::GroupStatus groupStatus = message::NOT_IN_GROUP;
  if (!msg.tuple.get<message::field_group_id.position>().isNull() && !msg.tuple.get<message::field_group_id.position>().value().empty()) {
    if (msg.tuple.get<message::field_last_in_group.position>()) {
      groupStatus = message::LAST_IN_GROUP;
    } else {
      groupStatus = message::ONE_OF_GROUP;
    }
  }
  return groupStatus;
}
void Destination::increaseNotAcknowledged(const std::string &objectID) {
  upmq::ScopedReadRWLock readRWLock(_notAckLock);
  auto it = _notAckList.find(objectID);
  if (it != _notAckList.end()) {
    ++(it->second);
  }
}
void Destination::increaseNotAcknowledgedAll() {
  upmq::ScopedReadRWLock readRWLock(_notAckLock);
  for (auto &it : _notAckList) {
    ++(it.second);
  }
}
bool Destination::canSendNextMessages(const std::string &objectID) const {
  upmq::ScopedReadRWLock readRWLock(_notAckLock);
  const auto it = _notAckList.find(objectID);
  if (it != _notAckList.end()) {
    return (it->second > 0);
  }
  return false;
}
void Destination::decreesNotAcknowledged(const std::string &objectID) const {
  upmq::ScopedReadRWLock readRWLock(_notAckLock);
  auto it = _notAckList.find(objectID);
  if (it != _notAckList.end()) {
    --(it->second);
  }
}
void Destination::addToNotAckList(const std::string &objectID, int count) const {
  upmq::ScopedWriteRWLock writeRWLock(_notAckLock);
  _notAckList.insert(std::make_pair(objectID, count));
}
void Destination::remFromNotAck(const std::string &objectID) const {
  upmq::ScopedWriteRWLock writeRWLock(_notAckLock);
  _notAckList.erase(objectID);
}
void Destination::postNewMessageEvent() const {
  if (_subscriptionsLock.tryReadLock()) {
    try {
      for (const auto &item : _subscriptions) {
        item.second.postNewMessageEvent();
      }
    } catch (...) {
      _subscriptionsLock.unlockRead();
      throw;
    }
    _subscriptionsLock.unlockRead();
  }
}
bool Destination::removeConsumer(const std::string &sessionID, const std::string &subscriptionID, size_t tcpNum) {
  std::string toerase;
  bool result = false;
  {
    upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
    const auto it = _subscriptions.find(subscriptionID);
    if (it != _subscriptions.end()) {
      result = it->second.removeClient(tcpNum, sessionID);
      if (!it->second.isRunning()) {
        if (!it->second.isDurable()) {
          toerase = it->first;
        }
        remS2Subs(sessionID, it->first);
      }
    }
  }
  if (!toerase.empty()) {
    upmq::ScopedWriteRWLock writeRWLock(_subscriptionsLock);
    auto it = _subscriptions.find(toerase);
    eraseSubscription(it);
  }
  return result;
}
Storage &Destination::storage() const { return _storage; }
int64_t Destination::initBrowser(const std::string &subscriptionName) {
  auto it = _subscriptions.end();

  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  it = _subscriptions.find(subscriptionName);
  if (it == _subscriptions.end()) {
    throw EXCEPTION("subscription not found", subscriptionName, ERROR_ON_BROWSER);
  }

  if (!it->second.hasSnapshot()) {
    copyMessagesTo(it->second);
    it->second.setHasSnapshot(true);
  }
  int64_t result = it->second.storage().size();
  it->second.start();
  it->second.postNewMessageEvent();
  return result;
}
// NOTE: browser subscription has only one consumer
void Destination::copyMessagesTo(Subscription &subscription) {
  const Consumer *consumer = subscription.at(0);
  if (consumer != nullptr) {
    _storage.copyTo(subscription.storage(), *consumer);
  }
}
void Destination::addS2Subs(const std::string &sesionID, const std::string &subsID) {
  upmq::ScopedWriteRWLock writeRWLock(_s2subsLock);
  _s2subsList.insert(std::make_pair(sesionID, subsID));
}
void Destination::remS2Subs(const std::string &sessionID, const std::string &subsID) {
  //  upmq::ScopedWriteRWLock writeRWLock(_s2subsLock);
  auto item = _s2subsList.equal_range(sessionID);
  for (auto itRg = item.first; itRg != item.second; ++itRg) {
    if ((*itRg).second == subsID) {
      _s2subsList.erase(itRg);
      return;
    }
  }
}
void Destination::closeAllSubscriptions(const Session &session, size_t tcpNum) {
  upmq::ScopedWriteRWLock writeRWLock(_s2subsLock);
  auto item = _s2subsList.equal_range(session.id());
  bool removed = false;
  do {
    if (item.first != item.second) {
      auto itRg = item.first;
      removed = removeConsumer(session.id(), (*itRg).second, tcpNum);
      break;
    }
    if (!removed) {
      break;
    }
    item = _s2subsList.equal_range(session.id());
  } while (item.first != item.second);
  postNewMessageEvent();
}
bool Destination::getNexMessageForAllSubscriptions() {
  bool result = false;
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  for (auto &subscription : _subscriptions) {
    if (subscription.second.isRunning()) {
      if (subscription.second.getNextMessage() && !result) {
        result = true;
      }
    }
  }
  return result;
}
void Destination::loadDurableSubscriptions() {
  std::stringstream sql;
  std::string id;
  std::string name;
  int type = static_cast<int>(Subscription::Type::DURABLE);
  Poco::Nullable<std::string> routingKey;
  sql << "select "
      << "id, name, routing_key"
      << " from " << subscriptionsT() << " where type = " << type << ";";
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  TRY_POCO_DATA_EXCEPTION {
    Poco::Data::Statement select(dbSession());
    select << sql.str(), Poco::Data::Keywords::into(id), Poco::Data::Keywords::into(name), Poco::Data::Keywords::into(routingKey), Poco::Data::Keywords::range(0, 1);
    while (!select.done()) {
      select.execute();
      if (!id.empty() && !name.empty()) {
        upmq::ScopedWriteRWLock writeRWLock(_subscriptionsLock);
        const std::string &rKey = (routingKey.isNull() ? emptyString : routingKey.value());

        auto item = _subscriptions.emplace(name, createSubscription(name, rKey, static_cast<Subscription::Type>(type)));
        subscribeOnNotify(item.first->second);
        item.first->second.setInited(false);
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't create subscription", sql.str(), ERROR_ON_SUBSCRIPTION)
}
void Destination::bindWithSubscriber(const std::string &clientID, bool useFileLink) {
  upmq::ScopedWriteRWLock writeRWLock(_predefSubscribersLock);
  _predefinedSubscribers.insert({clientID, useFileLink});
}
void Destination::unbindFromSubscriber(const std::string &clientID) {
  upmq::ScopedWriteRWLock writeRWLock(_predefSubscribersLock);
  _predefinedSubscribers.erase(clientID);
}
bool Destination::isBindToSubscriber(const std::string &clientID) const {
  upmq::ScopedReadRWLock readRWLock(_predefSubscribersLock);
  if (_predefinedSubscribers.empty()) {
    return true;
  }
  return (_predefinedSubscribers.count(clientID) > 0);
}
bool Destination::isSubscriberUseFileLink(const std::string &clientID) const {
  upmq::ScopedReadRWLock readRWLock(_predefSubscribersLock);
  auto it = _predefinedSubscribers.find(clientID);
  if (it != _predefinedSubscribers.end()) {
    return it->second;
  }
  return false;
}
void Destination::bindWithPublisher(const std::string &clientID, bool useFileLink) {
  upmq::ScopedWriteRWLock writeRWLock(_predefPublishersLock);
  _predefinedPublisher.insert({clientID, useFileLink});
}
void Destination::unbindFromPublisher(const std::string &clientID) {
  upmq::ScopedWriteRWLock writeRWLock(_predefPublishersLock);
  _predefinedPublisher.erase(clientID);
}
bool Destination::isBindToPublisher(const std::string &clientID) const {
  upmq::ScopedReadRWLock readRWLock(_predefPublishersLock);
  if (_predefinedPublisher.empty()) {
    return true;
  }
  return (_predefinedPublisher.count(clientID) > 0);
}
bool Destination::isPublisherUseFileLink(const std::string &clientID) const {
  upmq::ScopedReadRWLock readRWLock(_predefPublishersLock);
  auto it = _predefinedPublisher.find(clientID);
  if (it != _predefinedPublisher.end()) {
    return it->second;
  }
  return false;
}
void Destination::setOwner(const std::string &clientID, size_t tcpID) {
  if (_owner == nullptr) {
    _owner = std::make_unique<upmq::broker::DestinationOwner>(clientID, tcpID);
  }
}
const DestinationOwner &Destination::owner() const { return *_owner; }
bool Destination::hasOwner() const { return _owner != nullptr; }
Destination::Info Destination::info() const {
  Destination::Info dinfo(_uri,
                          _id,
                          DestinationFactory::destinationName(_uri),
                          _type,
                          Poco::DateTimeFormatter::format(*_created, DT_FORMAT_SIMPLE),
                          Exchange::mainDestinationPath(_uri),
                          static_cast<uint64_t>(storage().size()));
  upmq::ScopedReadRWLock readRWLock(_subscriptionsLock);
  for (const auto &subscription : _subscriptions) {
    dinfo.subscriptions.emplace_back(subscription.second.info());
  }
  return dinfo;
}
std::string Destination::typeName(Destination::Type type) {
  switch (type) {
    case Type::NONE:
      return MakeStringify(NONE);
    case Type::QUEUE:
      return MakeStringify(QUEUE);
    case Type::TOPIC:
      return MakeStringify(TOPIC);
    case Type::TEMPORARY_QUEUE:
      return MakeStringify(TEMPORARY_QUEUE);
    case Type::TEMPORARY_TOPIC:
      return MakeStringify(TEMPORARY_TOPIC);
  }
  return MakeStringify(NONE);
}

Destination::Type Destination::type(const std::string &typeName) {
  if (typeName == MakeStringify(QUEUE)) {
    return Destination::Type::QUEUE;
  }
  if (typeName == MakeStringify(TOPIC)) {
    return Destination::Type::TOPIC;
  }
  if (typeName == MakeStringify(TEMPORARY_QUEUE)) {
    return Destination::Type::TEMPORARY_QUEUE;
  }
  if (typeName == MakeStringify(TEMPORARY_TOPIC)) {
    return Destination::Type::TEMPORARY_TOPIC;
  }
  return Destination::Type::NONE;
}
}  // namespace broker
}  // namespace upmq
