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

#include "Subscription.h"
#include <Poco/UUIDGenerator.h>
#include <protocol.pb.h>
#include <NextBindParam.h>
#include "AsyncHandlerRegestry.h"
#include "Broker.h"
#include "Connection.h"
#include "Destination.h"
#include "Exception.h"
#include "Exchange.h"
#include "MessageStorage.h"
#include "MiscDefines.h"
#include "TopicSender.h"
#include <fake_cpp14.h>

namespace upmq {
namespace broker {
// NOTE: for queues id must be same as in destination
Subscription::Subscription(const Destination &destination, const std::string &id, std::string name, std::string routingKey, Subscription::Type type)
    : _id(id.empty() ? Poco::UUIDGenerator::defaultGenerator().createRandom().toString() : id),
      _name(std::move(name)),
      _type(type),
      _routingKey(std::move(routingKey)),
      _storage(_id, STORAGE_CONFIG.messages.nonPresistentSize),
      _destination(destination),
      _isRunning(new std::atomic_bool(false)),
      _currentConsumerNumber(0),
      _consumersT("\"" + _id + "_subscription\""),
      _messageCounter(0),
      log(&Poco::Logger::get(CONFIGURATION::Instance().log().name)),
      _isSubsNotify(false),
      _isDestroyed(false),
      _isInited(true),
      _hasSnapshot(false),
      _roundRobinCache(new std::deque<std::shared_ptr<MessageDataContainer>>) {
  _storage.setParent(&destination);

  std::stringstream sql;
  sql << "drop table if exists " << _consumersT << ";" << non_std_endl;
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't init consumers table for subscription", sql.str(), ERROR_UNKNOWN)
  sql.str("");
  sql << "create table if not exists " << _consumersT << "("
      << " client_id text not null"
      << ",tcp_id int not null"
      << ",selector text"
      << ",object_id text unique"
      << ",session text"
      << ",nolocal boolean"
      << ",create_time timestamp not null default current_timestamp"
      << ",constraint \"" << _id << "_tcp_index\" unique (client_id, tcp_id, session, selector)"
      << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't init destination", sql.str(), ERROR_DESTINATION)
  sql.str("");
  std::string ignorsert = "insert or ignore";
  std::string postfix;
  if (STORAGE_CONFIG.connection.props.dbmsType == storage::Postgresql) {
    ignorsert = "insert";
    postfix = " on conflict do nothing";
  }

  NextBindParam nextParam;

  sql << ignorsert << " into " << _destination.subscriptionsT() << "("
      << "id, name, type, routing_key"
      << ")"
      << " values "
      << "("
      << " \'" << _id << "\'"
      << "," << nextParam();
  sql << "," << static_cast<int>(_type) << "," << nextParam() << ")" << postfix << ";";

  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(_id);
  TRY_POCO_DATA_EXCEPTION {
    dbSession << sql.str(), Poco::Data::Keywords::useRef(_name), Poco::Data::Keywords::useRef(_routingKey), Poco::Data::Keywords::now;
  }
  CATCH_POCO_DATA_EXCEPTION_NO_INVALID_SQL("can't create subscription", sql.str(), ;, ERROR_ON_SUBSCRIPTION)
  dbSession.commitTX();
}
Subscription::~Subscription() noexcept {
  try {
    if (_isSubsNotify) {
      _destination.unsubscribeFromNotify(*this);
    }
    if (!_consumersLock.isValid()) {
      return;
    }
    if (!_isDestroyed) {
      destroy();
    }
  } catch (...) {
  }
}
void Subscription::save(const Session &session, const MessageDataContainer &sMessage) {
  if (isBrowser()) {
    return;
  }
  if (!_destination.isQueueFamily()) {
    const Message &message = sMessage.message();
    if (session.isTransactAcknowledge() && !_storage.hasTransaction(session)) {
      _storage.begin(session);
    }
    if (!session.currentDBSession.exists()) {
      session.currentDBSession = dbms::Instance().dbmsSessionPtr();
    }
    session.currentDBSession->beginTX(message.sender_id());
    _senders.fixMessageInGroup(message.sender_id(), session, sMessage);
    session.currentDBSession->commitTX();

    session.currentDBSession->beginTX(_id + message.message_id());
    TRY_POCO_DATA_EXCEPTION { _storage.save(session, sMessage); }
    CATCH_POCO_DATA_EXCEPTION_PURE("can't save message", "", ERROR_ON_SAVE_MESSAGE)
    session.currentDBSession->commitTX();
    _destination.postNewMessageEvent();
  }
}
void Subscription::commit(const Session &session) {
  _storage.commit(session);
  _destination.postNewMessageEvent();
}
void Subscription::abort(const Session &session) {
  _storage.abort(session);
  _destination.postNewMessageEvent();
}

void Subscription::addClient(
    const Session &session, size_t tcpConnectionNum, const std::string &objectID, const std::string &selector, Subscription::LocalMode localMode) {
  std::stringstream sql;
  // NOTE: if subscription is browser then make client_id more unique
  std::string clientID = session.connection().clientID();

  bool nolocal = (localMode == LocalMode::IS_NO_LOCAL);
  if (isBrowser()) {
    clientID.append("-browser");
  }
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  if (!selector.empty()) {
    sql << "select count(*) from " << _consumersT << " where "
        << " client_id = \'" << clientID << "\'"
        << " and "
        << " tcp_id = " << tcpConnectionNum << ""
        << " and "
        << " session = \'" << session.id() << "\'"
        << " and "
        << " (selector = '' or selector is null)"
        << ";";

    size_t count = 0;
    dbSession.beginTX(objectID);
    TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::into(count), Poco::Data::Keywords::now; }
    CATCH_POCO_DATA_EXCEPTION_PURE("can't add consumer", sql.str(), ERROR_ON_SUBSCRIPTION)

    if (count > 0) {
      throw EXCEPTION("client already exists", sql.str(), ERROR_ON_SUBSCRIPTION);
    }
  }

  NextBindParam nextParam;

  sql.str("");
  sql << "insert into " << _consumersT << "(client_id, tcp_id, selector, object_id, session, nolocal)"
      << " values "
      << "(";
  sql << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam() << ");";

  TRY_POCO_DATA_EXCEPTION {
    dbSession << sql.str(), Poco::Data::Keywords::use(clientID), Poco::Data::Keywords::use(tcpConnectionNum), Poco::Data::Keywords::useRef(selector),
        Poco::Data::Keywords::useRef(objectID), Poco::Data::Keywords::useRef(session.id()), Poco::Data::Keywords::use(nolocal),
        Poco::Data::Keywords::now;
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't add consumer", sql.str(), ERROR_ON_SUBSCRIPTION)
  dbSession.commitTX();

  std::shared_ptr<std::deque<std::shared_ptr<MessageDataContainer>>> selectCache;
  if (_destination.isQueueFamily() && _destination.consumerMode() == ConsumerMode::ROUND_ROBIN) {
    selectCache = _roundRobinCache;
  } else {
    selectCache = std::make_shared<std::deque<std::shared_ptr<MessageDataContainer>>>();
  }

  _destination.addToNotAckList(objectID, session.connection().maxNotAcknowledgedMessages(tcpConnectionNum));

  upmq::ScopedWriteRWLock writeRWLock(_consumersLock);
  _consumers.emplace_back(Consumer::genConsumerID(clientID, std::to_string(tcpConnectionNum), session.id(), selector),
                          Consumer(static_cast<int>(_consumers.size() + 1),
                                   clientID,
                                   tcpConnectionNum,
                                   objectID,
                                   session.id(),
                                   session.type(),
                                   selector,
                                   localMode == LocalMode::IS_NO_LOCAL,
                                   _type == Type::BROWSER,
                                   session.connection().maxNotAcknowledgedMessages(tcpConnectionNum),
                                   selectCache));
}
const std::string &Subscription::routingKey() const { return _routingKey; }
void Subscription::onEvent(const void *pSender, const MessageDataContainer *&sMessage) {
  if (sMessage == nullptr) {
    removeSenders(*((const Session *)pSender));
  } else {
    const MessageDataContainer &dc = *sMessage;
    if (dc.isMessage()) {
      save(*((const Session *)pSender), dc);
    } else if (dc.isUnsender()) {
      removeSender(*((const Session *)pSender), dc);
    } else if (dc.isSender()) {
      addSender(*((const Session *)pSender), dc);
    }
  }
}
bool Subscription::isDurable() const { return _type == Type::DURABLE; }
bool Subscription::isBrowser() const { return _type == Type::BROWSER; }
void Subscription::start() {
  _destination.postNewMessageEvent();
  if (*_isRunning) {
    return;
  }
  *_isRunning = true;
}
void Subscription::stop() {
  if (*_isRunning) {
    *_isRunning = false;
  }
}
void Subscription::stop(const Consumer &consumer) {
  try {
    {
      upmq::ScopedReadRWLock readRWLock(_consumersLock);
      const Consumer &cons = byClientAndHandlerAndSessionIDs(consumer.clientID, consumer.tcpNum, consumer.session.id);
      cons.stop();
    }
    if (allConsumersStopped()) {
      stop();
    }
  } catch (Exception &ex) {
    UNUSED_VAR(ex);
  }
}
void Subscription::start(const Consumer &consumer) {
  try {
    upmq::ScopedReadRWLock readRWLock(_consumersLock);
    const Consumer &cons = byClientAndHandlerAndSessionIDs(consumer.clientID, consumer.tcpNum, consumer.session.id);
    cons.start();

  } catch (Exception &ex) {
    UNUSED_VAR(ex);
  }
  start();
}
void Subscription::recover() {
  upmq::ScopedReadRWLock readRWLock(_consumersLock);
  for (const auto &consumer : _consumers) {
    _storage.setMessagesToNotSent(consumer.second);
    consumer.second.select->clear();
  }
}
void Subscription::recover(const Consumer &consumer) {
  try {
    upmq::ScopedReadRWLock readRWLock(_consumersLock);
    const Consumer &cons = byClientAndHandlerAndSessionIDs(consumer.clientID, consumer.tcpNum, consumer.session.id);
    _storage.setMessagesToNotSent(cons);
    cons.select->clear();
  } catch (Exception &ex) {
    UNUSED_VAR(ex);
  }
}
Subscription::ProcessMessageResult Subscription::getNextMessage() {
  std::string groupID;
  std::string messageID;
  ScopedWriteTryLocker swTryLocker(_consumersLock, false);
  if (swTryLocker.tryLock()) {
    const Consumer *consumer = at(_currentConsumerNumber);
    if ((consumer == nullptr) || !consumer->isRunning) {
      changeCurrentConsumerNumber();
      swTryLocker.unlock();
      return ProcessMessageResult::CONSUMER_NOT_RAN;
    }
    if (!_destination.canSendNextMessages(consumer->objectID)) {
      swTryLocker.unlock();
      return ProcessMessageResult::CONSUMER_CANT_SEND;
    }

    std::shared_ptr<MessageDataContainer> sMessage;
    Storage &storage = (_destination.isQueueFamily() && !isBrowser()) ? _destination.storage() : _storage;
    const bool useFileLink = _destination.isSubscriberUseFileLink(consumer->clientID);
    size_t consumersSize = _consumers.size();
    do {
      try {
        sMessage = storage.get(*consumer, useFileLink);
      } catch (Exception &ex) {
        consumer->select->clear();
        log->error("%s", std::string(consumer->clientID).append(" ! <= [").append(std::string(__FUNCTION__)).append("] ").append(ex.message()));
        swTryLocker.unlock();
        return ProcessMessageResult::SOME_ERROR;
      } catch (std::exception &stdex) {
        consumer->select->clear();
        log->error("%s", std::string(consumer->clientID).append(" ! <= [").append(std::string(__FUNCTION__)).append("] ").append(stdex.what()));
        swTryLocker.unlock();
        return ProcessMessageResult::SOME_ERROR;
      }
      if (sMessage) {
        if (!sMessage->message().has_group_id()) {
          changeCurrentConsumerNumber();
          groupID.clear();
        } else {
          groupID = sMessage->message().group_id();
        }

        messageID = sMessage->message().message_id();
        sMessage->protoMessage().set_request_reply_id(0);

        try {
          sMessage->serialize();
          AHRegestry::Instance().put(consumer->tcpNum, std::move(sMessage));
          ++_messageCounter;
          const size_t tid = (size_t)(Poco::Thread::currentTid());
          log->information("%s",
                           std::to_string(consumer->tcpNum)
                               .append(" * <= from subs => ")
                               .append(_name)
                               .append(" : consumer [ tid(")
                               .append(std::to_string(tid))
                               .append(") ")
                               .append(std::to_string(consumer->num))
                               .append(":")
                               .append(consumer->clientID)
                               .append(":")
                               .append((useFileLink ? "use_file_link" : "standard"))
                               .append("] to client : ")
                               .append(consumer->objectID)
                               .append(" >> send message [")
                               .append(messageID)
                               .append("] (")
                               .append(std::to_string(_messageCounter))
                               .append(")"));

          _destination.decreesNotAcknowledged(consumer->objectID);
          if (consumer->session.type == Proto::Acknowledge::CLIENT_ACKNOWLEDGE || !consumer->select->empty()) {
            EXCHANGE::Instance().addNewMessageEvent(_destination.name());
          }
          if (_destination.isQueueFamily() && _destination.consumerMode() == ConsumerMode::ROUND_ROBIN) {
            for (const auto &cn : _consumers) {
              if (cn.second.objectID != consumer->objectID) {
                _destination.decreesNotAcknowledged(cn.second.objectID);
              }
            }
            changeCurrentConsumerNumber();
          }

        } catch (Exception &ex) {
          log->error("%s", std::to_string(consumer->tcpNum).append(" ! <= [").append(__FUNCTION__).append("] ").append(ex.message()));
          if (ex.error() == ERROR_CONNECTION) {
            messageID.clear();
            removeConsumers(consumer->tcpNum);
            if (_consumers.empty()) {
              *_isRunning = false;
            }
          }
          swTryLocker.unlock();
          return ProcessMessageResult::SOME_ERROR;
        }
      } else {
        messageID.clear();
        if (consumersWithSelectorsOnly()) {
          changeCurrentConsumerNumber();
        }
        swTryLocker.unlock();
        return ProcessMessageResult::NO_MESSAGE;
      }
    } while (consumersSize == 1 && !consumer->select->empty());
    swTryLocker.unlock();
    return ProcessMessageResult::OK_COMPLETE;
  }
  return ProcessMessageResult::CONSUMER_LOCKED;
}
void Subscription::changeCurrentConsumerNumber() const {
  if (_consumers.empty()) {
    _currentConsumerNumber = 0;
  } else {
    ++_currentConsumerNumber;
    _currentConsumerNumber %= _consumers.size();
  }
  EXCHANGE::Instance().addNewMessageEvent(_destination.name());
}
const Consumer *Subscription::at(size_t index) const {
  auto consEnd = _consumers.rend();
  size_t counter = 0;
  const size_t curSize = (_consumers.empty()) ? 0 : (_consumers.size() - 1);
  if (index > curSize) {
    index = curSize;
  }
  for (auto it = _consumers.rbegin(); it != consEnd; ++it) {
    if ((counter++) == index) {
      return &(it->second);
    }
  }
  return nullptr;
}
Storage &Subscription::storage() const { return _storage; }
const Consumer &Subscription::byObjectID(const std::string &objectID) {
  auto consEnd = _consumers.rend();
  for (auto it = _consumers.rbegin(); it != consEnd; ++it) {
    if (it->second.objectID == objectID) {
      return it->second;
    }
  }
  throw EXCEPTION("consumer not found", objectID, ERROR_UNKNOWN);
}
const Consumer &Subscription::byClientAndHandlerAndSessionIDs(const std::string &clientID, size_t handlerNum, const std::string &sessionID) {
  std::string tmpClientID = (isBrowser() ? clientID + "-browser" : clientID);
  auto consEnd = _consumers.rend();
  for (auto it = _consumers.rbegin(); it != consEnd; ++it) {
    if (it->second.clientID == tmpClientID && it->second.tcpNum == handlerNum && it->second.session.id == sessionID) {
      return it->second;
    }
  }
  throw EXCEPTION("consumer not found", clientID + " : " + std::to_string(handlerNum), ERROR_UNKNOWN);
}
Subscription::ConsumersListType::iterator Subscription::eraseConsumer(ConsumersListType::iterator it) {
  std::stringstream sql;
  sql << "delete from " << _consumersT << " where object_id = \'" << it->second.objectID << "\';";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT("can't remove consumer", sql.str(), ERROR_ON_UNSUBSCRIPTION)
  _destination.remFromNotAck(it->second.objectID);
  return _consumers.erase(it);
}
bool Subscription::removeClient(size_t tcpConnectionNum, const std::string &sessionID) {
  bool result = false;
  bool doStop = false;
  {
    upmq::ScopedWriteRWLock writeRWLock(_consumersLock);
    result = removeConsumer(tcpConnectionNum, sessionID);
    doStop = _consumers.empty();
  }

  if (doStop) {
    stop();
    return result;
  }

  _destination.postNewMessageEvent();

  return result;
}
bool Subscription::removeConsumer(size_t tcpConnectionNum, const std::string &sessionID) {
  bool result = false;
  for (auto it = _consumers.begin(); it != _consumers.end();) {
    if ((it->second.tcpNum == tcpConnectionNum) && (it->second.session.id == sessionID)) {
      _storage.setMessagesToNotSent(it->second);
      it = eraseConsumer(it);
      result = true;
    } else {
      ++it;
    }
  }
  return result;
}

void Subscription::removeConsumers(size_t tcpConnectionNum) {
  for (auto it = _consumers.begin(); it != _consumers.end();) {
    if (it->second.tcpNum == tcpConnectionNum) {
      _storage.setMessagesToNotSent(it->second);
      it = eraseConsumer(it);
    } else {
      ++it;
    }
  }
}
void Subscription::removeClients() {
  upmq::ScopedWriteRWLock writeRWLock(_consumersLock);
  for (auto it = _consumers.begin(); it != _consumers.end();) {
    _storage.setMessagesToNotSent(it->second);
    it = eraseConsumer(it);
  }
  stop();
}
bool Subscription::isRunning() const { return *_isRunning; }
void Subscription::setHasNotify(bool hasNotify) { _isSubsNotify = hasNotify; }
void Subscription::destroy() {
  removeClients();
  if (isBrowser() /*|| _destination.isTemporary()*/) {
    _storage.dropTables();
  }

  std::stringstream sql;
  sql << "drop table if exists " << _consumersT << ";" << non_std_endl;
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(_id);
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT("can't erase subscription", sql.str(), ERROR_ON_UNSUBSCRIPTION)
  if (!isDurable()) {
    sql.str("");
    sql << "delete from " << _destination.subscriptionsT() << " where id = \'" << _id << "\';";
    TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
    CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't erase subscription", sql.str(), ERROR_ON_UNSUBSCRIPTION)

    //    sql.str("");
    //    sql << "update " << EXCHANGE::Instance().destinationsT() << " set subscriptions_count = " << _destination.subscriptionsTrueCount();
    //    TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
    //    CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT("can't update subscriptions count", sql.str(), ERROR_ON_UNSUBSCRIPTION)
  }
  dbSession.commitTX();
  _isDestroyed = true;
}
const std::string &Subscription::id() const { return _id; }
void Subscription::setInited(bool inited) { _isInited = inited; }
bool Subscription::isInited() const { return _isInited; }
void Subscription::addSender(const Session &session, const MessageDataContainer &sMessage) {
  const Proto::Sender &sender = sMessage.sender();
  std::unique_ptr<Sender> pSender = std::make_unique<TopicSender>(sender.sender_id(), session, *this);
  _senders.addSender(std::move(pSender));
}
void Subscription::removeSender(const Session &session, const MessageDataContainer &sMessage) {
  const std::string &senderID = sMessage.unsender().sender_id();
  _senders.closeGroup(senderID, session);
  _senders.removeSender(senderID);
}
void Subscription::removeSenders(const Session &session) {
  _senders.closeGroups(session);
  _senders.removeSenders(session);
}
bool Subscription::allConsumersStopped() {
  size_t stoppedCount = 0;
  for (const auto &item : _consumers) {
    if (!item.second.isRunning) {
      ++stoppedCount;
    }
  }
  return (stoppedCount == _consumers.size());
}
bool Subscription::consumersWithSelectorsOnly() const {
  for (const auto &consumer : _consumers) {
    if (consumer.second.selector != nullptr) {
      return !consumer.second.selector->expression().empty();
    }
  }
  return false;
}
bool Subscription::hasSnapshot() const { return _hasSnapshot; }
void Subscription::setHasSnapshot(bool hasSnapshot) { _hasSnapshot = hasSnapshot; }
Subscription::Info Subscription::info() const {
  upmq::ScopedReadRWLock readRWLock(_consumersLock);
  return Subscription::Info(_id, _name, _type, static_cast<int>(_consumers.size()), _messageCounter, *_isRunning);
}

void Subscription::resetConsumersCache() {
  upmq::ScopedReadRWLock readRWLock(_consumersLock);
  for (auto &consumer : _consumers) {
    consumer.second.abort = true;
  }
}

}  // namespace broker
}  // namespace upmq
