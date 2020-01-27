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

#include "Exchange.h"
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <sstream>
#include <fake_cpp14.h>
#include "Broker.h"
#include "MiscDefines.h"

namespace upmq {
namespace broker {

Exchange::Exchange()
    : _destinations(DESTINATION_CONFIG.maxCount),
      _destinationsT("\"" + BROKER::Instance().id() + "_destinations\""),
      _mutexDestinations(THREADS_CONFIG.subscribers),
      _conditionDestinations(_mutexDestinations.size()),
      _threadPool("exchange", 1, static_cast<int>(_mutexDestinations.size()) + 1) {
  std::stringstream sql;
  sql << "create table if not exists " << _destinationsT << "("
      << " id text not null primary key"
      << ",name text not null"
      << ",type int not null"
      << ",create_time timestamp not null default current_timestamp"
      << ",subscriptions_count int not null default 0"
      << ",constraint \"" << BROKER::Instance().id() << "_destinations_index\" unique (name, type)"
      << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't init exchange", sql.str(), ERROR_STORAGE);
}
Exchange::~Exchange() {
  try {
    _destinations.clear();
  } catch (...) {
  }
};
Destination &Exchange::destination(const std::string &uri, Exchange::DestinationCreationMode creationMode) const {
  std::string mainDP;
  if (uri.find("://") != std::string::npos) {
    mainDP = mainDestinationPath(uri);
  } else {
    mainDP = uri;
  }
  switch (creationMode) {
    case DestinationCreationMode::NO_CREATE: {
      return getDestination(mainDP);
    }
    case DestinationCreationMode::CREATE: {
      auto it = _destinations.find(mainDP);
      if (it.hasValue()) {
        auto &dest = *it;
        if (dest->consumerMode() != Destination::makeConsumerMode(uri)) {
          std::string err = "current consumer mode is ";
          err.append(Destination::consumerModeName(dest->consumerMode()));
          throw EXCEPTION("destination was initiated with another consumer mode", err, ERROR_DESTINATION);
        }
        return *dest;
      }

      // FIXME: if mainDP isn't uri then createDestination throw exception
      _destinations.insert(std::make_pair(mainDP, DestinationFactory::createDestination(*this, uri)));
      it = _destinations.find(mainDP);

      return *(*it);
    }
  }
  throw EXCEPTION("invalid creation mode", std::to_string(static_cast<int>(creationMode)), ERROR_UNKNOWN);
}  // namespace broker
Destination &Exchange::getDestination(const std::string &id) const {
  auto it = _destinations.find(id);
  if (!it.hasValue()) {
    throw EXCEPTION("destination not found", id, ERROR_UNKNOWN);
  }
  return *(*it);
}
void Exchange::deleteDestination(const std::string &uri) {
  std::string mainDP = mainDestinationPath(uri);
  _destinations.erase(mainDP);
}
std::string Exchange::mainDestinationPath(const std::string &uri) {
  Poco::StringTokenizer URI(uri, ":", Poco::StringTokenizer::TOK_TRIM);
  return DestinationFactory::destinationTypePrefix(uri) + DestinationFactory::destinationName(uri);
}
void Exchange::saveMessage(const Session &session, const MessageDataContainer &sMessage) {
  std::stringstream sql;
  const Proto::Message &message = sMessage.message();
  Destination &dest = destination(message.destination_uri(), DestinationCreationMode::NO_CREATE);
  sql << "insert into " << STORAGE_CONFIG.messageJournal(dest.name()) << "("
      << "message_id, uri, body_type, subscribers_count"
      << ")"
      << " values "
      << "("
      << " \'" << message.message_id() << "\'"
      << ",\'" << dest.name() << "\'"
      << "," << message.body_type() << "," << dest.subscriptionsCount() << ")"
      << ";";
  session.currentDBSession = dbms::Instance().dbmsSessionPtr();
  session.currentDBSession->beginTX(message.message_id());
  TRY_POCO_DATA_EXCEPTION { (*session.currentDBSession) << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION("can't save message", sql.str(), session.currentDBSession.reset(nullptr), ERROR_ON_SAVE_MESSAGE)
  dest.save(session, sMessage);
}
const std::string &Exchange::destinationsT() const { return _destinationsT; }
void Exchange::removeConsumer(const std::string &sessionID, const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum) {
  Destination &destination = getDestination(destinationID);
  destination.removeConsumer(sessionID, subscriptionID, tcpNum);
}
void Exchange::removeConsumer(const MessageDataContainer &sMessage, size_t tcpNum) {
  const Unsubscription &unsubscription = sMessage.unsubscription();
  const std::string destinationID = Exchange::mainDestinationPath(unsubscription.destination_uri());
  removeConsumer(unsubscription.session_id(), destinationID, unsubscription.subscription_name(), tcpNum);
}
void Exchange::begin(const upmq::broker::Session &session, const std::string &destinationID) {
  Destination &dest = destination(destinationID, DestinationCreationMode::NO_CREATE);
  dest.begin(session);
}
void Exchange::commit(const upmq::broker::Session &session, const std::string &destinationID) {
  Destination &destination = getDestination(destinationID);
  destination.commit(session);
  destination.postNewMessageEvent();
}
void Exchange::abort(const upmq::broker::Session &session, const std::string &destinationID) {
  Destination &destination = getDestination(destinationID);
  destination.abort(session);
  destination.postNewMessageEvent();
}
bool Exchange::isDestinationTemporary(const std::string &id) {
  Destination &destination = getDestination(id);
  return destination.isTemporary();
}

void Exchange::dropDestination(const std::string &id, DestinationOwner *owner) {
  bool needErase = false;
  {
    auto it = _destinations.find(id);
    if (it.hasValue()) {
      auto &dest = *it;
      needErase = ((owner == nullptr) || (dest->hasOwner() && owner->clientID == dest->owner().clientID));
    }
  }
  if (needErase) {
    _destinations.erase(id);
  }
}

void Exchange::dropOwnedDestination(const std::string &clientId) {
  DestinationsList::ItemType::KeyType key;
  bool needErase = false;
  {
    auto dest = _destinations.findIf([&clientId](const DestinationsList::ItemType::KVPair &pair) {
      if (pair.second->isTemporary() && pair.second->hasOwner()) {
        return pair.second->owner().clientID == clientId;
      }
      return false;
    });
    if (dest.hasValue()) {
      needErase = true;
      key = dest.key();
    }
  }
  if (needErase) {
    _destinations.erase(key);
  }
}

void Exchange::addSubscription(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  Destination &dest = destination(sMessage.subscription().destination_uri(), DestinationCreationMode::NO_CREATE);
  if (dest.isBindToSubscriber(sMessage.clientID)) {
    dest.subscription(session, sMessage);
    //    std::stringstream sql;
    //    sql << "update " << _destinationsT << " set subscriptions_count = " << dest.subscriptionsTrueCount() << ";";
    //    TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
    //    CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't update subscriptions count", sql.str(), ERROR_ON_SUBSCRIPTION)
  } else {
    throw EXCEPTION("this destination was bound to another subscriber", dest.name() + " : " + sMessage.clientID, ERROR_ON_SUBSCRIPTION);
  }
}
void Exchange::addSender(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  Destination &dest = destination(sMessage.sender().destination_uri(), DestinationCreationMode::NO_CREATE);
  if (dest.isBindToPublisher(sMessage.clientID)) {
    dest.addSender(session, sMessage);
  } else {
    throw EXCEPTION("this destination was bound to another publisher", dest.name() + " : " + sMessage.clientID, ERROR_ON_SUBSCRIPTION);
  }
}
void Exchange::removeSender(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  const Unsender &unsender = sMessage.unsender();
  if (unsender.destination_uri().empty()) {
    removeSenderFromAnyDest(session, unsender.sender_id());
  } else {
    try {
      Destination &dest = destination(unsender.destination_uri(), DestinationCreationMode::NO_CREATE);
      dest.removeSender(session, sMessage);
    } catch (...) {  // -V565 do nothing
    }
  }
}
void Exchange::removeSenders(const upmq::broker::Session &session) {
  _destinations.changeForEach([&session](DestinationsList::ItemType::KVPair &dest) { dest.second->removeSenders(session); });
}
void Exchange::removeSenderFromAnyDest(const upmq::broker::Session &session, const std::string &senderID) {
  _destinations.changeForEach([&session, &senderID](DestinationsList::ItemType::KVPair &dest) { dest.second->removeSenderByID(session, senderID); });
}
void Exchange::start() {
  _threadAdapter = std::make_unique<Poco::RunnableAdapter<Exchange>>(*this, &Exchange::run);
  int count = _threadPool.capacity() - 1;
  _isRunning = true;
  for (int i = 0; i < count; ++i) {
    _threadPool.start(*_threadAdapter);
  }
}
void Exchange::stop() {
  if (_isRunning) {
    _isRunning = false;
    _threadPool.joinAll();
  }
}
void Exchange::postNewMessageEvent(const std::string &name) const {
  const int count = _threadPool.capacity() - 1;

  addNewMessageEvent(name);

  for (size_t i = 0; i < static_cast<size_t>(count); ++i) {
    _conditionDestinations[i].notify_one();
  }
}

void Exchange::addNewMessageEvent(const std::string &name) const {
  if (!name.empty()) {
    _destinationEvents.enqueue(name);
  }
}

void Exchange::run() {
  const size_t num = _thrNum++;

  std::string queueId;
  while (_isRunning) {
    do {
      queueId.clear();
      if (_destinationEvents.try_dequeue(queueId)) {
        if (!queueId.empty()) {
          auto item = _destinations.find(queueId);
          if (item.hasValue()) {
            try {
              if ((*item)->getNexMessageForAllSubscriptions()) {
                _destinationEvents.enqueue(queueId);
                break;
              }
            } catch (Poco::Exception &pex) {
              std::cerr << "!!! " << pex.message() << " " << pex.className() << " " << pex.code() << std::endl;
            }
          }
        }
      }
    } while (!queueId.empty());

    auto &mut = _mutexDestinations[num];

    std::unique_lock<std::mutex> lock(mut);
    _conditionDestinations[num].wait_for(lock, std::chrono::milliseconds(1000));
  }
}
std::vector<Destination::Info> Exchange::info() const {
  std::vector<Destination::Info> infos;
  std::map<size_t, std::vector<Destination::Info>> infosGroup;

  auto containDigit = [](const std::string &s) {
    bool has = false;
    std::for_each(s.begin(), s.end(), [&](const char &c) {
      if (std::isdigit(c)) has = true;
    });
    return has;
  };

  _destinations.applyForEach([&containDigit, &infosGroup](const DestinationsList::ItemType::KVPair &dest) {
    auto info = dest.second->info();
    size_t sz = 0;
    if (containDigit(info.name)) {
      sz = info.name.size();
    }
    infosGroup[sz].emplace_back(std::move(info));
  });
  std::stringstream sql;
  sql << "select id, name, type, create_time from " << _destinationsT;
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  Poco::Data::Statement select(dbSession());
  Destination::Info destInfo;
  TRY_POCO_DATA_EXCEPTION {
    select << sql.str(), Poco::Data::Keywords::into(destInfo.id), Poco::Data::Keywords::into(destInfo.name),
        Poco::Data::Keywords::into(*((int *)&destInfo.type)), Poco::Data::Keywords::into(destInfo.created), Poco::Data::Keywords::range(0, 1);
    while (!select.done()) {
      select.execute();
      if (!destInfo.name.empty() && !destInfo.id.empty()) {
        if (destInfo.name.find(TEMP_QUEUE_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, TEMP_QUEUE_PREFIX "/", "");
        } else if (destInfo.name.find(TEMP_TOPIC_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, TEMP_TOPIC_PREFIX "/", "");
        } else if (destInfo.name.find(QUEUE_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, QUEUE_PREFIX "/", "");
        } else if (destInfo.name.find(TOPIC_PREFIX "/") != std::string::npos) {
          Poco::replaceInPlace(destInfo.name, TOPIC_PREFIX "/", "");
        }

        destInfo.uri = Poco::toLower(Destination::typeName(destInfo.type)) + "://" + destInfo.name;
        destInfo.dataPath = Exchange::mainDestinationPath(destInfo.uri);
        size_t sz = 0;
        if (containDigit(destInfo.name)) {
          sz = destInfo.name.size();
        }
        auto resultInfo = std::find_if(
            infosGroup[sz].begin(), infosGroup[sz].end(), [&destInfo](const Destination::Info &info) { return info.name == destInfo.name; });
        if (resultInfo == infosGroup[sz].end()) {
          infosGroup[sz].emplace_back(destInfo);
        }
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't get destinations info", sql.str(), ERROR_STORAGE)

  for (auto &item : infosGroup) {
    std::sort(item.second.begin(), item.second.end(), [](const Destination::Info &l, const Destination::Info &r) { return (l.name < r.name); });
    std::for_each(item.second.begin(), item.second.end(), [&](const Destination::Info &info) { infos.emplace_back(info); });
  }

  return infos;
}
}  // namespace broker
}  // namespace upmq
