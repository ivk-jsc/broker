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

#include <Exchange.h>
#include <MessagePropertyInfo.h>
#include <Poco/Hash.h>
#include <Poco/StringTokenizer.h>
#include <limits>
#include <sstream>
#include <NextBindParam.h>
#include "Broker.h"
#include "Connection.h"
#include "Exception.h"
#include "S2SProto.h"
#include "Defines.h"
#include "MessageStorage.h"

using upmq::broker::storage::DBMSConnectionPool;

namespace upmq {
namespace broker {

Storage::Storage(const std::string &messageTableID, size_t nonPersistentSize)
    : _messageTableID("\"" + messageTableID + "\""),
      _propertyTableID("\"" + messageTableID + "_property" + "\""),
      _parent(nullptr),
      _nonPersistent(nonPersistentSize) {
  log = &Poco::Logger::get(CONFIGURATION::Instance().log().name);
  TRACE(log);
  std::string mainTsql = generateSQLMainTable(messageTableID);
  auto mainTXsqlIndexes = generateSQLMainTableIndexes(messageTableID);

  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(messageTableID);

  OnError onError;
  onError.setError(Proto::ERROR_STORAGE).setSql(mainTsql).setInfo("can't init storage").setExpression([&dbSession]() { dbSession.rollbackTX(); });

  TRY_EXECUTE(([&mainTsql, &mainTXsqlIndexes, &dbSession]() {
                dbSession << mainTsql, Poco::Data::Keywords::now;
                for (const auto &index : mainTXsqlIndexes) {
                  dbSession << index, Poco::Data::Keywords::now;
                }
              }),
              onError);

  std::string propTsql = generateSQLProperties();
  onError.setSql(propTsql);
  TRY_EXECUTE(([&propTsql, &dbSession]() { dbSession << propTsql, Poco::Data::Keywords::now; }), onError);

  dbSession.commitTX();
}
Storage::~Storage() { TRACE(log); };
std::string Storage::generateSQLMainTable(const std::string &tableName) const {
  TRACE(log);
  std::stringstream sql;
  std::string autoinc = "integer primary key autoincrement not null";
  std::string currTimeType = "bigint";
  std::string currTime = "(strftime('%Y-%m-%dT%H:%M:%f', 'now'))";
  switch (STORAGE_CONFIG.connection.props.dbmsType) {
    case storage::Postgresql:
      autoinc = "bigserial primary key";
      currTimeType = "timestamp";
      currTime = "clock_timestamp()";
      break;
    default:
      break;
  }
  sql << " create table if not exists \"" << tableName << "\""
      << "("
      << "    num " << autoinc << ""
      << "   ,message_id text not null unique"
      << "   ,type text not null"
      << "   ,body_type int not null default 0"
      << "   ,priority int not null"
      << "   ,persistent boolean not null"
      << "   ,correlation_id text"
      << "   ,reply_to text"
      << "   ,client_timestamp bigint not null"
      << "   ,expiration bigint not null default 0"
      << "   ,ttl bigint not null default 0"
      << "   ,created_time " << currTimeType << " not null default " << currTime << ""
      << "   ,delivery_count int not null default 0"
      << "   ,delivery_status int not null default 0"
      << "   ,client_id text not null"
      << "   ,consumer_id text"
      << "   ,group_id text"
      << "   ,group_seq int not null default 0"
      << "   ,last_in_group bool not null default 'false'"
      << "   ,transaction_id text"
      << "); ";

  return sql.str();
}
std::vector<std::string> Storage::generateSQLMainTableIndexes(const std::string &tableName) const {
  TRACE(log);
  std::stringstream sql;
  std::vector<std::string> indexes;
  sql << "create index if not exists \"" << tableName << "_msgs_delivery_status\" on "  // if not exists
      << "\"" << tableName << "\""
      << "( delivery_status ); ";
  indexes.emplace_back(sql.str());
  sql.str("");
  sql << "create index if not exists \"" << tableName << "_msgs_consumer_id\" on "  // if not exists
      << "\"" << tableName << "\""
      << "( consumer_id ); ";
  indexes.emplace_back(sql.str());
  sql.str("");
  sql << "create index if not exists \"" << tableName << "_msgs_transaction_id\" on "  // if not exists
      << "\"" << tableName << "\""
      << "( transaction_id ); ";
  indexes.emplace_back(sql.str());
  sql.str("");

  return indexes;
}
std::string Storage::generateSQLProperties() const {
  TRACE(log);
  std::stringstream sql;
  std::string idx = _propertyTableID;
  idx.replace(_propertyTableID.find_last_of('\"'), 1, "_");
  idx.append("midpn\"");
  std::string blob = "blob";
  switch (STORAGE_CONFIG.connection.props.dbmsType) {
    case storage::Postgresql:
      blob = "bytea";
      break;
    default:
      break;
  }
  sql << " create table if not exists " << _propertyTableID << "("
      << "    value_char int"
      << "   ,value_bool boolean"
      << "   ,value_byte int"
      << "   ,value_short int"
      << "   ,value_int int"
      << "   ,value_long bigint"
      << "   ,value_float float"
      << "   ,value_double double precision"
      << "   ,value_string text"
      << "   ,message_id text not null"
      << "   ,property_name text not null"
      << "   ,property_type int not null"
      << "   ,value_bytes " << blob << ""
      << "   ,value_object " << blob << ""
      << "   ,is_null boolean not null default false"
      << "   ,constraint " << idx << " unique (message_id, property_name)"
      << ");";
  return sql.str();
}
void Storage::removeMessagesBySession(const upmq::broker::Session &session) {
  TRACE(log);
  std::stringstream sql;
  sql << "select * from " << _messageTableID << " where consumer_id like \'%" << session.id() << "%\'";
  if (session.isTransactAcknowledge()) {
    sql << " and transaction_id = \'" << session.txName() << "\'";
  }
  sql << ";" << non_std_endl;

  MessageInfo messageInfo;

  OnError onError;
  onError.setError(Proto::ERROR_ON_SAVE_MESSAGE).setSql(sql.str()).setInfo("can't remove all messages in session");

  TRY_EXECUTE(([&session, &sql, &messageInfo, this]() {
                Poco::Data::Statement select((*session.currentDBSession)());
                select << sql.str(), Poco::Data::Keywords::into(messageInfo.tuple), Poco::Data::Keywords::range(0, 1);
                while (!select.done()) {
                  select.execute();

                  auto &fieldMessageId = messageInfo.tuple.get<message::field::MessageId::POSITION>();
                  if (!fieldMessageId.empty()) {
                    auto &fieldGroupId = messageInfo.tuple.get<message::field::GroupId::POSITION>();
                    if (!fieldGroupId.isNull() && !fieldGroupId.value().empty()) {
                      if (messageInfo.tuple.get<message::field::LastInGroup::POSITION>()) {
                        removeGroupMessage(fieldGroupId.value(), session);
                      }
                    } else {
                      removeMessage(fieldMessageId, *session.currentDBSession);
                    }
                  }
                }
              }),
              onError);
}
void Storage::resetMessagesBySession(const upmq::broker::Session &session) {
  TRACE(log);
  std::stringstream sql;

  std::string transactionExclusion = std::string(" or  delivery_status = ").append(std::to_string(message::WAS_SENT));

  sql << "update " << _messageTableID << " set delivery_status = " << message::NOT_SENT << " , consumer_id = '' "
      << " where consumer_id like \'%" << session.id() << "%\'";
  if (session.isTransactAcknowledge()) {
    sql << " and transaction_id = \'" << session.txName() << "\'";
  }
  sql << " and (delivery_status = " << message::DELIVERED << transactionExclusion << ")"
      << ";" << non_std_endl;

  *session.currentDBSession << sql.str(), Poco::Data::Keywords::now;
}
void Storage::removeGroupMessage(const std::string &groupID, const upmq::broker::Session &session) {
  TRACE(log);
  std::vector<std::string> result;
  std::stringstream sql;
  sql << "select message_id from " << _messageTableID << " where group_id = \'" << groupID << "\'"
      << ";";
  bool externConnection = (session.currentDBSession != nullptr);
  std::unique_ptr<storage::DBMSSession> tempDBMSSession;
  if (!externConnection) {
    tempDBMSSession = std::make_unique<storage::DBMSSession>(dbms::Instance().dbmsSession());
    tempDBMSSession->beginTX(groupID);
  }
  storage::DBMSSession &dbSession = externConnection ? *session.currentDBSession : *tempDBMSSession;

  OnError onError;
  onError.setError(Proto::ERROR_ON_ACK_MESSAGE).setSql(sql.str()).setInfo("can't get message group for ack");

  TRY_EXECUTE(([&dbSession, &sql, &result]() { dbSession << sql.str(), Poco::Data::Keywords::into(result), Poco::Data::Keywords::now; }), onError);

  for (const auto &msgID : result) {
    removeMessage(msgID, dbSession);
  }
  if (tempDBMSSession) {
    tempDBMSSession->commitTX();
  }
}
message::GroupStatus Storage::checkIsGroupClosed(const MessageDataContainer &sMessage, const Session &session) const {
  TRACE(log);
  int result = -1;
  std::stringstream sql;
  sql << "select count(last_in_group) from " << _messageTableID << " where last_in_group = \'TRUE\' and group_id in "
      << "(select group_id from " << _messageTableID << " where message_id = \'" << sMessage.ack().message_id() << "\'"
      << " and delivery_status <> " << message::DELIVERED << ")"
      << ";";
  bool externConnection = (session.currentDBSession != nullptr);
  std::unique_ptr<storage::DBMSSession> tempDBMSSession;
  if (!externConnection) {
    tempDBMSSession = std::make_unique<storage::DBMSSession>(dbms::Instance().dbmsSession());
  }
  storage::DBMSSession &dbSession = externConnection ? *session.currentDBSession : *tempDBMSSession;

  OnError onError;
  onError.setError(Proto::ERROR_ON_ACK_MESSAGE).setInfo("can't check last in group for ack").setSql(sql.str());

  TRY_EXECUTE(([&dbSession, &sql, &result]() { dbSession << sql.str(), Poco::Data::Keywords::into(result), Poco::Data::Keywords::now; }), onError);

  switch (result) {
    case 0:
      return message::GroupStatus::ONE_OF_GROUP;
    case 1:
      return message::GroupStatus::LAST_IN_GROUP;
    default:
      return message::GroupStatus::NOT_IN_GROUP;
  }
}
bool Storage::deleteMessageHeader(storage::DBMSSession &dbSession, const std::string &messageID) {
  TRACE(log);
  std::stringstream sql;
  bool persistent = !_nonPersistent.contains(messageID);
  sql << "delete from " << _messageTableID << " where message_id = \'" << messageID << "\'"
      << ";" << non_std_endl;
  OnError onError;
  onError.setError(Proto::ERROR_UNKNOWN).setInfo("can't erase message").setSql(sql.str());

  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);

  return persistent;
}
void Storage::deleteMessageProperties(storage::DBMSSession &dbSession, const std::string &messageID) {
  TRACE(log);
  std::stringstream sql;
  sql << "delete from " << _propertyTableID << " where message_id = \'" << messageID << "\'"
      << ";" << non_std_endl;
  OnError onError;
  onError.setError(Proto::ERROR_UNKNOWN).setInfo("can't erase message properties").setSql(sql.str());

  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);
}
int Storage::getSubscribersCount(storage::DBMSSession &dbSession, const std::string &messageID) {
  TRACE(log);
  int subscribersCount = 1;
  std::stringstream sql;
  sql << "select subscribers_count from " << STORAGE_CONFIG.messageJournal(_parent->name()) << " where message_id = \'" << messageID << "\';";
  OnError onError;
  onError.setError(Proto::ERROR_UNKNOWN).setInfo("can't get subscribers count").setSql(sql.str());

  TRY_EXECUTE(
      ([&dbSession, &sql, &subscribersCount]() { dbSession << sql.str(), Poco::Data::Keywords::into(subscribersCount), Poco::Data::Keywords::now; }),
      onError);

  return (--subscribersCount);
}
void Storage::updateSubscribersCount(storage::DBMSSession &dbSession, const std::string &messageID) {
  TRACE(log);
  std::stringstream sql;
  sql << "update " << STORAGE_CONFIG.messageJournal(_parent->name()) << " set subscribers_count = subscribers_count - 1 where message_id =\'"
      << messageID << "\';" << non_std_endl;

  OnError onError;
  onError.setError(Proto::ERROR_UNKNOWN).setInfo("can't update subscribers count").setSql(sql.str());

  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);
}
void Storage::deleteMessageInfoFromJournal(storage::DBMSSession &dbSession, const std::string &messageID) {
  TRACE(log);
  std::stringstream sql;
  sql << "delete from " << STORAGE_CONFIG.messageJournal(_parent->name()) << " where message_id = \'" << messageID << "\';";
  OnError onError;
  onError.setError(Proto::ERROR_UNKNOWN).setInfo("can't delete message from journal").setSql(sql.str());

  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);
}
void Storage::deleteMessageDataIfExists(const std::string &messageID, bool persistent) {
  TRACE(log);
  if (persistent) {
    std::string mID = Poco::replace(messageID, ":", "_");
    Poco::Path msgFile = STORAGE_CONFIG.data.get();
    msgFile.append(_parent->name());
    msgFile.append(mID).makeFile();
    ::remove(msgFile.toString().c_str());
  } else {
    _nonPersistent.erase(messageID);
  }
}
void Storage::removeMessage(const std::string &messageID, storage::DBMSSession &extDBSession) {
  TRACE(log);
  bool externConnection = extDBSession.isValid();
  std::unique_ptr<storage::DBMSSession> tempDBMSSession;
  if (!externConnection) {
    tempDBMSSession = dbms::Instance().dbmsSessionPtr();
  }
  storage::DBMSSession &dbSession = externConnection ? extDBSession : *tempDBMSSession;
  if (!externConnection) {
    dbSession.beginTX(messageID);
  }

  const bool wasPersistent = deleteMessageHeader(dbSession, messageID);
  deleteMessageProperties(dbSession, messageID);
  const int subscribersCount = getSubscribersCount(dbSession, messageID);
  if (subscribersCount <= 0) {
    deleteMessageInfoFromJournal(dbSession, messageID);
    deleteMessageDataIfExists(messageID, wasPersistent);
  } else {
    updateSubscribersCount(dbSession, messageID);
  }

  if (!externConnection) {
    dbSession.commitTX();
  }
}
const std::string &Storage::messageTableID() const { return _messageTableID; }
const std::string &Storage::propertyTableID() const { return _propertyTableID; }
void Storage::saveMessageHeader(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  TRACE(log);
  storage::DBMSSession &dbs = *session.currentDBSession;
  const Proto::Message &message = sMessage.message();
  bool persistent = message.persistent();
  int bodyType = message.body_type();
  int priority = message.priority();
  Poco::Int64 timestamp = message.timestamp();
  Poco::Int64 expiration = message.expiration();
  Poco::Int64 ttl = message.timetolive();
  int groupSeq = message.group_seq();

  NextBindParam nextParam;

  std::stringstream sql;
  sql << "insert into " << saveTableName(session)
      << " (message_id, priority, persistent, correlation_id, reply_to, type, "
         "client_timestamp, ttl, expiration, "
         "body_type, client_id, group_id, group_seq)"
      << " values "
      << "(" << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam();
  sql << "," << nextParam() << ");";

  OnError onError;
  onError.setError(Proto::ERROR_ON_SAVE_MESSAGE).setSql(sql.str()).setInfo("failed to save message header id : " + message.message_id());
  TRY_EXECUTE(([&dbs, &message, &priority, &persistent, &timestamp, &ttl, &expiration, &bodyType, &sMessage, &groupSeq, &sql]() {
                Poco::Data::Statement insert(dbs());
                insert.addBind(Poco::Data::Keywords::useRef(message.message_id()))
                    .addBind(Poco::Data::Keywords::use(priority))
                    .addBind(Poco::Data::Keywords::use(persistent))
                    .addBind(Poco::Data::Keywords::useRef(message.correlation_id()))
                    .addBind(Poco::Data::Keywords::useRef(message.reply_to()))
                    .addBind(Poco::Data::Keywords::useRef(message.type()))
                    .addBind(Poco::Data::Keywords::use(timestamp))
                    .addBind(Poco::Data::Keywords::use(ttl))
                    .addBind(Poco::Data::Keywords::use(expiration))
                    .addBind(Poco::Data::Keywords::use(bodyType))
                    .addBind(Poco::Data::Keywords::useRef(sMessage.clientID))
                    .addBind(Poco::Data::Keywords::useRef(message.group_id()))
                    .addBind(Poco::Data::Keywords::use(groupSeq));

                insert << sql.str();
                insert.execute();
              }),
              onError);
}
void Storage::save(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
  TRACE(log);
  const Proto::Message &message = sMessage.message();
  const std::string &messageID = message.message_id();

  if (!message.persistent()) {
    _nonPersistent.insert(std::make_pair(messageID, std::shared_ptr<MessageDataContainer>(sMessage.clone())));
  }
  try {
    saveMessageProperties(session, message);
    saveMessageHeader(session, sMessage);
  } catch (PDSQLITE::InvalidSQLStatementException &ioex) {
    _nonPersistent.erase(messageID);
    if (ioex.message().find("no such table") != std::string::npos) {
      throw EXCEPTION(ioex.message(), _messageTableID + " or " + _propertyTableID, Proto::ERROR_ON_SAVE_MESSAGE);
    }
    ioex.rethrow();
  } catch (Poco::Exception &pex) {
    _nonPersistent.erase(messageID);
    pex.rethrow();
  } catch (...) {
    _nonPersistent.erase(messageID);
    throw;
  }
}
bool Storage::checkTTLIsOut(const Poco::DateTime &messageTime, Poco::Int64 ttl) {
  TRACE(log);
  if (ttl <= 0) {
    return false;
  }
  Poco::DateTime currentDateTime;
  Poco::Timespan ttlTimespan(ttl * Poco::Timespan::MILLISECONDS);
  return ((messageTime + ttlTimespan).timestamp() < currentDateTime.timestamp());
}
std::shared_ptr<MessageDataContainer> Storage::get(const Consumer &consumer, bool useFileLink) {
  TRACE(log);
  std::stringstream sql;
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();

  if (consumer.abort) {
    consumer.select->clear();
    consumer.abort = false;
  }

  bool needFiltered = false;
  if (consumer.select->empty()) {
    sql.str("");
    sql << "select * "
        << " FROM " << _messageTableID << " as msgs"
        << " where delivery_status = " << message::NOT_SENT;
    if (consumer.noLocal) {
      sql << " and msgs.client_id <> \'" << consumer.clientID << "\'";
    }
    sql << " order by msgs.priority desc, msgs.num";

    if (consumer.selector && !consumer.browser) {
      needFiltered = true;
      sql << ";";
    } else {
      sql << " limit ";
      sql << consumer.maxNotAckMsg << ";";
    }

    MessageInfo messageInfo;

    OnError onError;
    onError.setError(Proto::ERROR_ON_GET_MESSAGE).setInfo("can't get message").setSql(sql.str());

    dbSession.beginTX(_extParentID);
    TRY_EXECUTE(([&dbSession, &sql, &messageInfo, &consumer, &needFiltered, &useFileLink, this]() {
                  Poco::Data::Statement select(dbSession());

                  select << sql.str(), Poco::Data::Keywords::into(messageInfo.tuple), Poco::Data::Keywords::range(0, 1);

                  while (!select.done()) {
                    messageInfo.clear();
                    select.execute();
                    if (!messageInfo.messageId().empty()) {
                      auto sMessage = makeMessage(dbSession, messageInfo, consumer, useFileLink);
                      if (sMessage) {
                        bool isOk = true;
                        if (needFiltered) {
                          isOk = consumer.selector->filter(*sMessage);
                        }
                        if (isOk) {
                          consumer.select->push_back(std::move(sMessage));
                        }
                      }
                    }
                  }
                  setMessagesToWasSent(dbSession, consumer);
                }),
                onError);
    dbSession.commitTX();
  }

  std::shared_ptr<MessageDataContainer> msgResult;
  if (!consumer.select->empty()) {
    msgResult = std::move(consumer.select->front());
    consumer.select->pop_front();
  }

  return msgResult;
}
void Storage::setParent(const broker::Destination *parent) { _parent = parent; }
const std::string &Storage::uri() const { return _parent ? _parent->uri() : emptyString; }
void Storage::saveMessageProperties(const upmq::broker::Session &session, const Proto::Message &message) {
  TRACE(log);
  std::vector<MessagePropertyInfo::MsgTuple> messageProperties(message.property_size());
  size_t i = 0;
  for (auto it = message.property().begin(); it != message.property().end(); ++it) {
    MessagePropertyInfo messagePropertyInfo;
    messagePropertyInfo.setValueNull(it->second.is_null());
    messagePropertyInfo.setMessageID(message.message_id());
    messagePropertyInfo.setPropertyName(it->first);
    messagePropertyInfo.setPropertyType(it->second.PropertyValue_case());
    switch (it->second.PropertyValue_case()) {
      case Proto::Property::kValueString: {
        messagePropertyInfo.setValueString(it->second.value_string());
      } break;

      case Proto::Property::kValueChar: {
        messagePropertyInfo.setValueChar(it->second.value_char());
      } break;

      case Proto::Property::kValueBool: {
        messagePropertyInfo.setValueBool(it->second.value_bool());
      } break;

      case Proto::Property::kValueByte: {
        messagePropertyInfo.setValueByte(it->second.value_byte());
      } break;

      case Proto::Property::kValueShort: {
        messagePropertyInfo.setValueShort(it->second.value_short());
      } break;

      case Proto::Property::kValueInt: {
        messagePropertyInfo.setValueInt(it->second.value_int());
      } break;

      case Proto::Property::kValueLong: {
        messagePropertyInfo.setValueLong(it->second.value_long());
      } break;

      case Proto::Property::kValueFloat: {
        messagePropertyInfo.setValueFloat(it->second.value_float());
      } break;

      case Proto::Property::kValueDouble: {
        messagePropertyInfo.setValueDouble(it->second.value_double());
      } break;

      case Proto::Property::kValueBytes: {
        messagePropertyInfo.setValueBytes(Poco::Data::BLOB((const unsigned char *)it->second.value_bytes().c_str(), it->second.value_bytes().size()));
      } break;

      case Proto::Property::kValueObject: {
        messagePropertyInfo.setValueObject(
            Poco::Data::BLOB((const unsigned char *)it->second.value_object().c_str(), it->second.value_object().size()));
      } break;

      default:
        break;
    }
    messageProperties[i] = std::move(messagePropertyInfo.tuple);
    i++;
  }
  if (!messageProperties.empty()) {
    storage::DBMSSession &dbSession = *session.currentDBSession;
    std::stringstream sql;
    std::string upsert = "insert or replace";
    std::string postfix;
    if (STORAGE_CONFIG.connection.props.dbmsType == storage::Postgresql) {
      upsert = "insert";
    }
    NextBindParam nextParam;
    sql << upsert << " into " << _propertyTableID
        << " (message_id,"
           "  property_name,"
           "  property_type, "
           "  value_string,"
           "  value_char,"
           "  value_bool,"
           "  value_byte,"
           "  value_short,"
           "  value_int,"
           "  value_long,"
           "  value_float,"
           "  value_double,"
           "  value_bytes,"
           "  value_object,"
           "  is_null) values (";
    sql << " " << nextParam();  // 1
    sql << "," << nextParam();  // 2
    sql << "," << nextParam();  // 3
    sql << "," << nextParam();  // 4
    sql << "," << nextParam();  // 5
    sql << "," << nextParam();  // 6
    sql << "," << nextParam();  // 7
    sql << "," << nextParam();  // 8
    sql << "," << nextParam();  // 9
    sql << "," << nextParam();  // 10
    sql << "," << nextParam();  // 11
    sql << "," << nextParam();  // 12
    sql << "," << nextParam();  // 13
    sql << "," << nextParam();  // 14
    sql << "," << nextParam();  // 15
    sql << ");";

    auto dumpInfo = [&messageProperties]() -> std::string {
      std::string result;
      result += non_std_endl;
      for (const auto &item : messageProperties) {
        result.append(MessagePropertyInfo::dump(item));
        result += non_std_endl;
      }
      return result;
    };
    OnError onError;
    onError.setError(Proto::ERROR_ON_SAVE_MESSAGE)
        .setSql(sql.str())
        .setInfo("failed to save message properties for msg id : " + message.message_id() + " " + dumpInfo());
    TRY_EXECUTE(([&dbSession, &messageProperties, &sql]() {
                  dbSession << sql.str(), Poco::Data::Keywords::use(messageProperties), Poco::Data::Keywords::now;
                }),
                onError);
  }
}
void Storage::begin(const Session &session, const std::string &extParentId) {
  TRACE(log);
  {
    upmq::ScopedWriteRWLock writeRWLock(_txSessionsLock);
    if (_txSessions.find(session.id()) == _txSessions.end()) {
      _txSessions.insert(session.id());
    }
  }
  _extParentID = _parent->id() + extParentId;
  std::string mainTXTable = std::to_string(Poco::hash(_extParentID + "_" + session.txName()));
  std::string mainTXsql = generateSQLMainTable(mainTXTable);
  auto mainTXsqlIndexes = generateSQLMainTableIndexes(mainTXTable);
  OnError onError;
  onError.setError(Proto::ERROR_ON_BEGIN).setInfo("can't create tx_table");
  std::unique_ptr<storage::DBMSSession> dbSessionPtr;

  auto getRef = [&session, &dbSessionPtr, &mainTXTable]() -> storage::DBMSSession & {
    if (session.currentDBSession == nullptr) {
      dbSessionPtr = dbms::Instance().dbmsSessionPtr();
      dbSessionPtr->beginTX(mainTXTable);
      return *dbSessionPtr;
    }
    return *session.currentDBSession;
  };

  storage::DBMSSession &dbSession = getRef();

  TRY_EXECUTE(([&mainTXsql, &mainTXsqlIndexes, &dbSession, &onError]() {
                onError.setSql(mainTXsql);
                dbSession << mainTXsql, Poco::Data::Keywords::now;
                for (const auto &index : mainTXsqlIndexes) {
                  onError.setSql(mainTXsql);
                  dbSession << index, Poco::Data::Keywords::now;
                }
              }),
              onError);
  if (dbSessionPtr != nullptr) {
    dbSessionPtr->commitTX();
  }
}
void Storage::commit(const Session &session) {
  TRACE(log);
  std::string mainTXTable = "\"" + std::to_string(Poco::hash(_extParentID + "_" + session.txName())) + "\"";
  std::stringstream sql;
  sql << "insert into " << _messageTableID
      << " (message_id, priority, persistent, correlation_id, reply_to, type, "
         "client_timestamp, ttl, expiration, "
         "body_type, client_id, consumer_id, group_id, group_seq)"
      << " select message_id, priority, persistent, correlation_id, reply_to, "
         "type, client_timestamp, ttl, "
         "expiration, body_type, client_id, consumer_id, group_id, group_seq from "
      << mainTXTable << " order by num asc;";
  std::unique_ptr<storage::DBMSSession> dbSession = dbms::Instance().dbmsSessionPtr();
  OnError onError;
  onError.setError(Proto::ERROR_ON_COMMIT).setInfo("can't commit");

  dbSession->beginTX(session.id());

  onError.setSql(sql.str());
  TRY_EXECUTE(([&dbSession, &sql]() { *dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);

  TRY_EXECUTE(([&dbSession, &mainTXTable, this]() { dropTXTable(*dbSession, mainTXTable); }), onError);

  onError.setInfo("can't commit (removeMessagesBySession) for session : " + session.id()).setExpression([&session]() {
    session.currentDBSession.reset(nullptr);
  });

  TRY_EXECUTE(([&session, &dbSession, this]() {
                session.currentDBSession = std::move(dbSession);
                removeMessagesBySession(session);
              }),
              onError);

  session.currentDBSession->commitTX();
  session.currentDBSession.reset(nullptr);

  _parent->postNewMessageEvent();
}
void Storage::abort(const Session &session) {
  TRACE(log);
  std::string mainTXTable = "\"" + std::to_string(Poco::hash(_extParentID + "_" + session.txName())) + "\"";
  std::stringstream sql;
  bool tbExist = false;
  sql << "select 1 from " << mainTXTable << ";";
  std::unique_ptr<storage::DBMSSession> dbSession = dbms::Instance().dbmsSessionPtr();
  dbSession->beginTX(session.id());

  OnError onError;
  onError.setError(Proto::ERROR_ON_ABORT).setSql(sql.str()).setInfo("can't check table on existence");

  TRY_EXECUTE(([&dbSession, &sql, &tbExist]() {
                try {
                  *dbSession << sql.str(), Poco::Data::Keywords::now;
                  tbExist = true;
                } catch (PDSQLITE::InvalidSQLStatementException &issex) {
                  UNUSED_VAR(issex);
                  tbExist = false;
                }
              }),
              onError);

  if (tbExist) {
    sql.str("");
    sql << "delete from " << STORAGE_CONFIG.messageJournal(_parent->name()) << " where message_id in ("
        << " select message_id from " << mainTXTable << ");";

    onError.setSql(sql.str()).setInfo("can't abort");
    TRY_EXECUTE(([&dbSession, &sql]() { *dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);

    sql.str("");
    sql << "delete from " << _propertyTableID << " where message_id in ("
        << " select message_id from " << mainTXTable << ");";

    onError.setSql(sql.str());
    TRY_EXECUTE(([&dbSession, &sql]() { *dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);

    onError.setSql("drop table if exists " + mainTXTable);
    TRY_EXECUTE(([&dbSession, &mainTXTable, this]() { dropTXTable(*dbSession, mainTXTable); }), onError);
  }

  onError.setSql("func:resetMessagesBySession").setInfo("can't abort").setExpression([&session]() { session.currentDBSession.reset(nullptr); });
  TRY_EXECUTE(([&dbSession, &session, this]() {
                session.currentDBSession = std::move(dbSession);
                resetMessagesBySession(session);
              }),
              onError);

  session.currentDBSession->commitTX();
  session.currentDBSession.reset(nullptr);
  _parent->postNewMessageEvent();
}
void Storage::dropTXTable(storage::DBMSSession &dbSession, const std::string &mainTXTable) const {
  TRACE(log);
  std::stringstream sql;
  sql << "drop table if exists " << mainTXTable << ";";
  dbSession << sql.str(), Poco::Data::Keywords::now;
}
std::vector<MessageInfo> Storage::getMessagesBelow(const Session &session, const std::string &messageID) const {
  if (!session.isClientAcknowledge()) {
    return std::vector<MessageInfo>{MessageInfo(messageID)};
  }
  std::vector<MessageInfo> result;
  std::stringstream sql;
  sql << "select * from " << _messageTableID << " where delivery_count > 0 "
      << " and consumer_id like \'%" << session.id() << "%'"
      << ";" << non_std_endl;

  OnError onError;
  onError.setError(Proto::ERROR_ON_ACK_MESSAGE).setSql(sql.str()).setInfo("can't get all messages below id");

  storage::DBMSSession &dbSession = session.currentDBSession.get();
  TRY_EXECUTE(([&dbSession, &sql, &result]() {
                Poco::Data::Statement select(dbSession());
                MessageInfo messageInfo;
                select << sql.str(), Poco::Data::Keywords::into(messageInfo.tuple), Poco::Data::Keywords::range(0, 1);
                while (!select.done()) {
                  select.execute();
                  if (!(messageInfo.tuple.get<message::field::MessageId::POSITION>()).empty()) {
                    result.push_back(messageInfo);
                  }
                }
              }),
              onError);

  return result;
}
void Storage::setMessageToWasSent(const std::string &messageID, storage::DBMSSession &dbSession, const Consumer &consumer) {
  TRACE(log);
  std::stringstream sql;
  sql << "update " << _messageTableID << " set delivery_status = " << message::WAS_SENT << ",    consumer_id = \'" << consumer.id << "\'"
      << ",    delivery_count  = delivery_count + 1";
  if (consumer.session.type == Proto::SESSION_TRANSACTED) {
    consumer.session.txName = BROKER::Instance().currentTransaction(consumer.clientID, consumer.session.id);
    sql << ",  transaction_id = \'" << consumer.session.txName << "\'";
  }
  sql << " where message_id = \'" << messageID << "\'"
      << ";";

  OnError onError;
  onError.setError(Proto::ERROR_STORAGE).setSql(sql.str()).setInfo("can't set message to was_sent");

  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);
}
void Storage::setMessagesToWasSent(storage::DBMSSession &dbSession, const Consumer &consumer) {
  TRACE(log);
  if (!consumer.select->empty()) {
    std::stringstream sql;
    sql << "update " << _messageTableID << " set delivery_status = " << message::WAS_SENT << ",    consumer_id = \'" << consumer.id << "\'"
        << ",    delivery_count  = delivery_count + 1";
    if (consumer.session.type == Proto::SESSION_TRANSACTED) {
      consumer.session.txName = BROKER::Instance().currentTransaction(consumer.clientID, consumer.session.id);
      sql << ",  transaction_id = \'" << consumer.session.txName << "\'";
    }
    sql << " where ";
    const std::deque<std::shared_ptr<MessageDataContainer>> &messages = *consumer.select;
    for (size_t i = 0; i < messages.size(); ++i) {
      if (i > 0) {
        sql << " or ";
      }
      sql << "message_id = \'" << messages[i]->message().message_id() << "\'";
    }
    sql << ";";

    OnError onError;
    onError.setError(Proto::ERROR_STORAGE).setSql(sql.str()).setInfo("can't set message to was_sent");

    TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);
  }
}
void Storage::setMessageToDelivered(const upmq::broker::Session &session, const std::string &messageID) {
  TRACE(log);
  std::stringstream sql;
  sql << "update " << _messageTableID << " set delivery_status = " << message::DELIVERED << " where message_id = \'" << messageID << "\'"
      << ";";
  storage::DBMSSession &dbSession = session.currentDBSession.get();
  OnError onError;
  onError.setError(Proto::ERROR_STORAGE).setSql(sql.str()).setInfo("can't set message to delivered");

  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);
}
void Storage::setMessageToLastInGroup(const Session &session, const std::string &messageID) {
  TRACE(log);
  std::stringstream sql;
  bool externConnection = (session.currentDBSession != nullptr);
  std::unique_ptr<storage::DBMSSession> tempDBMSSession;
  if (!externConnection) {
    tempDBMSSession = std::make_unique<storage::DBMSSession>(dbms::Instance().dbmsSession());
  }
  storage::DBMSSession &dbSession = (externConnection ? *session.currentDBSession : *tempDBMSSession);
  sql << "update " << saveTableName(session) << " set last_in_group = \'TRUE\'"
      << " where message_id = \'" << messageID << "\'"
      << ";";
  if (!externConnection) {
    dbSession.beginTX(messageID);
  }

  OnError onError;
  onError.setError(Proto::ERROR_ON_SAVE_MESSAGE).setSql(sql.str()).setInfo("can't set message to last_in_group");

  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);

  if (!externConnection) {
    dbSession.commitTX();
  }
}
std::string Storage::saveTableName(const Session &session) const {
  TRACE(log);
  std::string messageTable = _messageTableID;
  if (session.isTransactAcknowledge()) {
    messageTable = "\"" + std::to_string(Poco::hash(_extParentID + "_" + session.txName())) + "\"";
  }
  return messageTable;
}
void Storage::setMessagesToNotSent(const Consumer &consumer) {
  TRACE(log);
  OnError onError;
  onError.setError(Proto::ERROR_STORAGE).setInfo("can't set messages to not-sent");
  std::stringstream sql;
  sql << "update " << _messageTableID << " set delivery_status = " << message::NOT_SENT << " where consumer_id like \'%" << consumer.id << "%\'";
  if (consumer.session.type == Proto::SESSION_TRANSACTED) {
    sql << " and transaction_id = \'" << consumer.session.txName << "\'";
  }
  sql << ";";
  TRY_EXECUTE_NOEXCEPT([&sql]() { dbms::Instance().doNow(sql.str()); }, onError);
}
void Storage::copyTo(Storage &storage, const Consumer &consumer) {
  TRACE(log);
  storage.resetNonPersistent(_nonPersistent);

  bool withSelector = (consumer.selector && !consumer.selector->expression().empty());
  std::stringstream sql;

  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();

  sql.str("");
  if (withSelector) {
    sql << " select * from " << _messageTableID << " where delivery_status <> " << message::DELIVERED << " order by num asc;";
  } else {
    sql << "insert into " << storage.messageTableID()
        << " (message_id, priority, persistent, correlation_id, reply_to, "
           "type, client_timestamp, ttl, expiration, "
           "body_type, client_id, group_id, group_seq)"
        << " select message_id, priority, persistent, correlation_id, "
           "reply_to, type, client_timestamp, ttl, "
           "expiration, body_type, client_id, group_id, group_seq from "
        << _messageTableID << " where delivery_status <> " << message::DELIVERED << " order by num asc;";
  }

  OnError onError;
  onError.setError(Proto::ERROR_ON_BROWSER).setInfo("can't copy messages to the browser").setSql(sql.str());

  dbSession.beginTX(consumer.objectID);

  TRY_EXECUTE(([&withSelector, &dbSession, &sql, &consumer, &storage, this]() {
                if (withSelector) {
                  MessageInfo messageInfo;

                  auto &session = dbSession();
                  Poco::Data::Statement select(session);
                  Poco::Data::Statement insert(session);
                  select << sql.str(), Poco::Data::Keywords::into(messageInfo.tuple), Poco::Data::Keywords::range(0, 1);
                  NextBindParam nextParam;
                  while (!select.done()) {
                    nextParam.reset();
                    select.execute();
                    if (!messageInfo.messageId().empty()) {
                      auto sMessage = makeMessage(dbSession, messageInfo, consumer, false);  // TODO(bas524) : check use_file_link!!
                      if (sMessage && consumer.selector->filter(*sMessage)) {
                        sql.str("");
                        sql << " insert into " << storage.messageTableID() << " (message_id, priority, persistent, correlation_id, "
                            << " reply_to, type, client_timestamp, "
                            << " ttl, expiration, body_type, client_id, group_id, group_seq)"
                            << " values "
                            << " (" << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam();
                        sql << "," << nextParam() << ");";
                        insert << sql.str(), Poco::Data::Keywords::useRef(messageInfo.messageId()),
                            Poco::Data::Keywords::useRef(messageInfo.tuple.get<message::field::Priority::POSITION>()),
                            Poco::Data::Keywords::useRef(messageInfo.tuple.get<message::field::Persistent::POSITION>()),
                            Poco::Data::Keywords::use(Poco::Nullable<std::string>(messageInfo.correlationID())),
                            Poco::Data::Keywords::use(Poco::Nullable<std::string>(messageInfo.replyTo())),
                            Poco::Data::Keywords::useRef(messageInfo.type()),
                            Poco::Data::Keywords::useRef(messageInfo.tuple.get<message::field::Timestamp::POSITION>()),
                            Poco::Data::Keywords::useRef(messageInfo.tuple.get<message::field::TimeToLive::POSITION>()),
                            Poco::Data::Keywords::useRef(messageInfo.tuple.get<message::field::Expiration::POSITION>()),
                            Poco::Data::Keywords::useRef(messageInfo.tuple.get<message::field::BodyType::POSITION>()),
                            Poco::Data::Keywords::use(Poco::Nullable<std::string>(messageInfo.clientID())),
                            Poco::Data::Keywords::use(Poco::Nullable<std::string>(messageInfo.groupID())),
                            Poco::Data::Keywords::useRef(messageInfo.tuple.get<message::field::GroupSeq::POSITION>()), Poco::Data::Keywords::now;
                      }
                    }
                  }
                } else {
                  dbSession << sql.str(), Poco::Data::Keywords::now;
                }
              }),
              onError);

  sql.str("");
  sql << "update " << STORAGE_CONFIG.messageJournal(_parent->name()) << " set subscribers_count = subscribers_count + 1 where message_id in ("
      << "select message_id from " << storage.messageTableID() << ");";

  onError.setSql(sql.str());
  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);

  sql.str("");
  sql << "insert into " << storage.propertyTableID() << " select * from " << _propertyTableID << " where message_id in "
      << "("
      << " select message_id from " << storage.messageTableID() << ")"
      << ";";

  onError.setSql(sql.str());
  TRY_EXECUTE(([&dbSession, &sql]() { dbSession << sql.str(), Poco::Data::Keywords::now; }), onError);

  dbSession.commitTX();
}
void Storage::resetNonPersistent(const Storage::NonPersistentMessagesListType &nonPersistentMessagesList) {
  TRACE(log);
  _nonPersistent.clear();
  _nonPersistent = nonPersistentMessagesList;
}
int64_t Storage::size() {
  TRACE(log);
  std::stringstream sql;
  Poco::Int64 result = 0;
  OnError onError;
  onError.setError(Proto::ERROR_STORAGE).setInfo("can't get queue size");

  sql << "select count(*) from " << _messageTableID << " as msgs"
      << " where delivery_status <> " << message::DELIVERED;
  sql << ";";

  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(_messageTableID, storage::DBMSSession::TransactionMode::READ);
  TRY_EXECUTE_NOEXCEPT(([&dbSession, &sql, &result]() { dbSession << sql.str(), Poco::Data::Keywords::into(result), Poco::Data::Keywords::now; }),
                       onError);
  dbSession.commitTX();
  return result;
}
std::shared_ptr<MessageDataContainer> Storage::makeMessage(storage::DBMSSession &dbSession,
                                                           const MessageInfo &msgInfo,
                                                           const Consumer &consumer,
                                                           bool useFileLink) {
  TRACE(log);
  std::shared_ptr<MessageDataContainer> sMessage;
  if (!msgInfo.messageId().empty()) {
    Poco::DateTime messageDateTime = msgInfo.createdTime();

    bool ttlIsOut = checkTTLIsOut(messageDateTime, msgInfo.timetolive());

    if (ttlIsOut) {
      removeMessage(msgInfo.messageId(), dbSession);
      return {};
    }

    if (!msgInfo.persistent()) {
      auto item = _nonPersistent.find(msgInfo.messageId());
      if (item.hasValue()) {
        sMessage = (*item);
      } else {
        log->warning("message : %s was not found in non-persistent storage dump : %s", msgInfo.messageId(), msgInfo.dump());
        removeMessage(msgInfo.messageId(), dbSession);
        return {};
      }
      sMessage->setObjectID(consumer.objectID);
    } else {
      try {
        sMessage = std::make_shared<MessageDataContainer>(STORAGE_CONFIG.data.get().toString());
        Proto::Message &message = sMessage->createMessageHeader(consumer.objectID);

        message.set_message_id(msgInfo.messageId());
        message.set_destination_uri(_parent->uri());
        message.set_priority(msgInfo.priority());
        message.set_persistent(true);

        std::string data = sMessage->message().message_id();
        data[2] = '_';
        data = Exchange::mainDestinationPath(sMessage->message().destination_uri()) + "/" + data;
        auto &pmap = *message.mutable_property();
        if (useFileLink) {
          Poco::Path path = STORAGE_CONFIG.data.get();
          path.append(data);
          pmap[s2s::proto::upmq_data_link].set_value_string(path.toString());
          pmap[s2s::proto::upmq_data_link].set_is_null(false);

          pmap[s2s::proto::upmq_data_parts_number].set_value_int(0);
          pmap[s2s::proto::upmq_data_parts_number].set_is_null(false);

          pmap[s2s::proto::upmq_data_parts_count].set_value_int(0);
          pmap[s2s::proto::upmq_data_parts_count].set_is_null(false);

          pmap[s2s::proto::upmq_data_part_size].set_value_int(0);
          pmap[s2s::proto::upmq_data_part_size].set_is_null(false);
        } else {
          pmap.erase(s2s::proto::upmq_data_link);

          pmap.erase(s2s::proto::upmq_data_parts_number);

          pmap.erase(s2s::proto::upmq_data_parts_count);

          pmap.erase(s2s::proto::upmq_data_part_size);

          sMessage->setWithFile(true);
          sMessage->data = data;
        }
        if (!msgInfo.correlationID().isNull()) {
          message.set_correlation_id(msgInfo.correlationID().value());
        }
        if (!msgInfo.replyTo().isNull()) {
          message.set_reply_to(msgInfo.replyTo().value());
        }
        message.set_type(msgInfo.type());
        message.set_timestamp(msgInfo.timestamp());
        message.set_timetolive(msgInfo.timetolive());
        message.set_expiration(msgInfo.expiration());
        message.set_body_type(msgInfo.bodyType());

        fillProperties(dbSession, message);
      } catch (Exception &ex) {
        removeMessage(msgInfo.messageId(), dbSession);
        throw Exception(ex);
      }
    }
    Proto::Message &message = sMessage->mutableMessage();
    sMessage->setCreated(messageDateTime.timestamp().epochMicroseconds());
    sMessage->clientID = Poco::replace(consumer.clientID, "-browser", "");
    sMessage->handlerNum = consumer.tcpNum;
    message.set_sender_id(BROKER::Instance().id());
    message.set_session_id(consumer.session.id);
    if (!msgInfo.groupID().isNull()) {
      if (!msgInfo.groupID().value().empty()) {
        Poco::StringTokenizer groupIDAll(msgInfo.groupID().value(), "+", Poco::StringTokenizer::TOK_TRIM);
        message.set_group_id(groupIDAll[0]);
      } else {
        message.set_group_id(msgInfo.groupID().value());
      }
    }
    message.set_group_seq(msgInfo.groupSeq());
    sMessage->setDeliveryCount(msgInfo.deliveryCount());
  }
  return sMessage;
}
void Storage::fillProperties(storage::DBMSSession &dbSession, Proto::Message &message) {
  TRACE(log);
  std::stringstream sql;
  sql << "select "
         " message_id,"
         " property_name,"
         " property_type, "
         " value_string,"
         " value_char,"
         " value_bool,"
         " value_byte,"
         " value_short,"
         " value_int,"
         " value_long,"
         " value_float,"
         " value_double,"
         " value_bytes,"
         " value_object,"
         " is_null"
         " from "
      << _propertyTableID << " where message_id = \'" << message.message_id() << "\';";
  MessagePropertyInfo messagePropertyInfo;
  OnError onError;
  onError.setError(Proto::ERROR_ON_GET_MESSAGE).setInfo("can't fill properties").setSql(sql.str());

  TRY_EXECUTE(([&dbSession, &messagePropertyInfo, &message, &sql]() {
                Poco::Data::Statement select(dbSession());
                select << sql.str(), Poco::Data::Keywords::into(messagePropertyInfo.tuple), Poco::Data::Keywords::range(0, 1);
                while (!select.done()) {
                  select.execute();
                  if (!messagePropertyInfo.messageID().empty()) {
                    auto &pmap = *message.mutable_property();
                    const std::string &name = messagePropertyInfo.propertyName();
                    bool isNull = messagePropertyInfo.valueNull();
                    bool isNan = false;
                    if (messagePropertyInfo.isNull() && !isNull) {
                      isNan = true;
                    }

                    switch (static_cast<MessagePropertyInfo::Field>(messagePropertyInfo.propertyType())) {
                      case MessagePropertyInfo::Field::value_string:
                        pmap[name].set_value_string(messagePropertyInfo.valueString());
                        break;
                      case MessagePropertyInfo::Field::value_char:
                        pmap[name].set_value_char(messagePropertyInfo.valueChar());
                        break;
                      case MessagePropertyInfo::Field::value_bool:
                        pmap[name].set_value_bool(messagePropertyInfo.valueBool());
                        break;
                      case MessagePropertyInfo::Field::value_byte:
                        pmap[name].set_value_byte(messagePropertyInfo.valueByte());
                        break;
                      case MessagePropertyInfo::Field::value_short:
                        pmap[name].set_value_short(messagePropertyInfo.valueShort());
                        break;
                      case MessagePropertyInfo::Field::value_int:
                        pmap[name].set_value_int(messagePropertyInfo.valueInt());
                        break;
                      case MessagePropertyInfo::Field::value_long:
                        pmap[name].set_value_long(messagePropertyInfo.valueLong());
                        break;
                      case MessagePropertyInfo::Field::value_float:
                        if (isNan) {
                          pmap[name].set_value_float(std::numeric_limits<float>::quiet_NaN());
                        } else {
                          pmap[name].set_value_float(messagePropertyInfo.valueFloat());
                        }
                        break;
                      case MessagePropertyInfo::Field::value_double:
                        if (isNan) {
                          pmap[name].set_value_double(std::numeric_limits<double>::quiet_NaN());
                        } else {
                          pmap[name].set_value_double(messagePropertyInfo.valueDouble());
                        }
                        break;
                      case MessagePropertyInfo::Field::value_bytes:
                        pmap[name].set_value_bytes(messagePropertyInfo.valueBytes().rawContent(), messagePropertyInfo.valueBytes().size());
                        break;
                      case MessagePropertyInfo::Field::value_object:
                        pmap[name].set_value_object(reinterpret_cast<const char *>(messagePropertyInfo.valueObject().rawContent()),
                                                    messagePropertyInfo.valueObject().size());
                        break;
                      default:
                        pmap[name].set_is_null(true);
                    }

                    pmap[name].set_is_null(isNull);
                  }
                }
              }),
              onError);
}
void Storage::dropTables() {
  TRACE(log);
  OnError onError;
  onError.setError(Proto::ERROR_ON_UNSUBSCRIPTION).setInfo("can't drop table");
  std::stringstream sql;
  sql << "drop table if exists " << _messageTableID << ";" << non_std_endl;
  onError.setSql(sql.str());
  TRY_EXECUTE_NOEXCEPT([&sql]() { dbms::Instance().doNow(sql.str()); }, onError);

  sql.str("");
  sql << "drop table if exists " << _propertyTableID << ";";
  onError.setSql(sql.str());
  TRY_EXECUTE_NOEXCEPT([&sql]() { dbms::Instance().doNow(sql.str()); }, onError);
}
bool Storage::hasTransaction(const Session &session) const {
  TRACE(log);
  upmq::ScopedReadRWLock readRWLock(_txSessionsLock);
  return (_txSessions.find(session.id()) != _txSessions.end());
}
}  // namespace broker
}  // namespace upmq
