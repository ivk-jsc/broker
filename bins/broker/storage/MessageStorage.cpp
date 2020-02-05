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
#include <Poco/File.h>
#include <Poco/Hash.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Timezone.h>
#include <limits>
#include <sstream>
#include <fake_cpp14.h>
#include <NextBindParam.h>
#include "Broker.h"
#include "Connection.h"
#include "MappedDBMessage.h"
#include "MiscDefines.h"
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
  std::string mainTsql = generateSQLMainTable(messageTableID);
  auto mainTXsqlIndexes = generateSQLMainTableIndexes(messageTableID);
  TRY_POCO_DATA_EXCEPTION {
    storage::DBMSConnectionPool::doNow(mainTsql);
    for (const auto &index : mainTXsqlIndexes) {
      storage::DBMSConnectionPool::doNow(index);
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't init storage", mainTsql, ERROR_STORAGE)
  std::string propTsql = generateSQLProperties();
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(propTsql); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't init storage", mainTsql, ERROR_STORAGE)
}
Storage::~Storage() = default;
std::string Storage::generateSQLMainTable(const std::string &tableName) const {
  std::stringstream sql;
  std::string autoinc = "integer primary key autoincrement not null";
  std::string currTimeType = "text";
  std::string currTime = "(strftime('%Y-%m-%d %H:%M:%f', 'now'))";
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
      << "    num " << autoinc << "   ,message_id text not null unique"
      << "   ,type text not null"
      << "   ,body_type int not null default 0"
      << "   ,priority int not null"
      << "   ,persistent int not null"
      << "   ,correlation_id text"
      << "   ,reply_to text"
      << "   ,client_timestamp bigint not null"
      << "   ,expiration bigint not null default 0"
      << "   ,ttl bigint not null default 0"
      << "   ,created_time " << currTimeType << " not null default " << currTime << "   ,delivery_count int not null default 0"
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
      << "   ,value_bytes " << blob << "   ,value_object " << blob << "   ,is_null boolean not null default false"
      << "   ,constraint " << idx << " unique (message_id, property_name)"
      << ");";
  return sql.str();
}
void Storage::removeMessagesBySession(const upmq::broker::Session &session) {
  std::stringstream sql;
  sql << "select * from " << _messageTableID << " where consumer_id like \'%" << session.id() << "%\'";
  if (session.isTransactAcknowledge()) {
    sql << " and transaction_id = \'" << session.txName() << "\'";
  }
  sql << ";" << non_std_endl;

  MessageInfo messageInfo;
  TRY_POCO_DATA_EXCEPTION {
    Poco::Data::Statement select((*session.currentDBSession)());
    select << sql.str(), Poco::Data::Keywords::into(messageInfo.tuple), Poco::Data::Keywords::range(0, 1);
    while (!select.done()) {
      select.execute();

      auto &fieldMessageId = messageInfo.tuple.get<message::field_message_id.position>();
      if (!fieldMessageId.empty()) {
        auto &fieldGroupId = messageInfo.tuple.get<message::field_group_id.position>();
        if (!fieldGroupId.isNull() && !fieldGroupId.value().empty()) {
          if (messageInfo.tuple.get<message::field_last_in_group.position>()) {
            removeGroupMessage(fieldGroupId.value(), session);
          }
        } else {
          removeMessage(fieldMessageId, *session.currentDBSession);
        }
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't remove all messages in session", sql.str(), ERROR_ON_SAVE_MESSAGE)
}
void Storage::resetMessagesBySession(const upmq::broker::Session &session) {
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
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::into(result), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't get message group for ack", sql.str(), ERROR_ON_ACK_MESSAGE)

  for (const auto &msgID : result) {
    removeMessage(msgID, dbSession);
  }
  if (tempDBMSSession) {
    tempDBMSSession->commitTX();
  }
}
message::GroupStatus Storage::checkIsGroupClosed(const MessageDataContainer &sMessage, const Session &session) const {
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
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::into(result), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't check last in group for ack", sql.str(), ERROR_ON_ACK_MESSAGE)

  switch (result) {
    case 0:
      return message::GroupStatus::ONE_OF_GROUP;
    case 1:
      return message::GroupStatus::LAST_IN_GROUP;
    default:
      return message::GroupStatus::NOT_IN_GROUP;
  }
}
int Storage::deleteMessageHeader(storage::DBMSSession &dbSession, const std::string &messageID) {
  std::stringstream sql;
  int persistent = static_cast<int>(!_nonPersistent.contains(messageID));
  sql << "delete from " << _messageTableID << " where message_id = \'" << messageID << "\'"
      << ";" << non_std_endl;
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't erase message", sql.str(), ERROR_UNKNOWN)
  return persistent;
}
void Storage::deleteMessageProperties(storage::DBMSSession &dbSession, const std::string &messageID) {
  std::stringstream sql;
  sql << "delete from " << _propertyTableID << " where message_id = \'" << messageID << "\'"
      << ";" << non_std_endl;
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE_TROW_INVALID_SQL("can't erase message", sql.str(), ERROR_UNKNOWN)
}
int Storage::getSubscribersCount(storage::DBMSSession &dbSession, const std::string &messageID) {
  int subscribersCount = 1;
  std::stringstream sql;
  sql << "select subscribers_count from " << STORAGE_CONFIG.messageJournal(_parent->name()) << " where message_id = \'" << messageID << "\';";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::into(subscribersCount), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't get subscribers count", sql.str(), ERROR_UNKNOWN)
  return (--subscribersCount);
}
void Storage::updateSubscribersCount(storage::DBMSSession &dbSession, const std::string &messageID) {
  std::stringstream sql;
  sql << "update " << STORAGE_CONFIG.messageJournal(_parent->name()) << " set subscribers_count = subscribers_count - 1 where message_id =\'"
      << messageID << "\';" << non_std_endl;
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't erase message", sql.str(), ERROR_UNKNOWN)
}
void Storage::deleteMessageInfoFromJournal(storage::DBMSSession &dbSession, const std::string &messageID) {
  std::stringstream sql;
  sql << "delete from " << STORAGE_CONFIG.messageJournal(_parent->name()) << " where message_id = \'" << messageID << "\';";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't erase message", sql.str(), ERROR_UNKNOWN)
}
void Storage::deleteMessageDataIfExists(const std::string &messageID, int persistent) {
  if (persistent == 1) {
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
  bool externConnection = extDBSession.isValid();
  std::unique_ptr<storage::DBMSSession> tempDBMSSession;
  if (!externConnection) {
    tempDBMSSession = dbms::Instance().dbmsSessionPtr();
  }
  storage::DBMSSession &dbSession = externConnection ? extDBSession : *tempDBMSSession;
  if (!externConnection) {
    dbSession.beginTX(messageID);
  }

  const int wasPersistent = deleteMessageHeader(dbSession, messageID);
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
  storage::DBMSSession &dbs = *session.currentDBSession;
  const Proto::Message &message = sMessage.message();
  int persistent = message.persistent() ? 1 : 0;
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

  // Save header

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
}
void Storage::save(const upmq::broker::Session &session, const MessageDataContainer &sMessage) {
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
      throw EXCEPTION(ioex.message(), _messageTableID + " or " + _propertyTableID, ERROR_ON_SAVE_MESSAGE);
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
bool Storage::checkTTLIsOut(const std::string &stringMessageTime, Poco::Int64 ttl) {
  if (ttl <= 0) {
    return false;
  }
  Poco::DateTime currentDateTime;
  int tzd = Poco::Timezone::tzd();
  Poco::DateTime messageDateTime;
  Poco::DateTimeParser::parse(DT_FORMAT, stringMessageTime, messageDateTime, tzd);
  Poco::Timespan ttlTimespan(ttl * Poco::Timespan::MILLISECONDS);

  return ((messageDateTime + ttlTimespan).timestamp() < currentDateTime.timestamp());
}
std::shared_ptr<MessageDataContainer> Storage::get(const Consumer &consumer, bool useFileLink) {
  std::stringstream sql;
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();

  if (consumer.abort) {
    consumer.select->clear();
    consumer.abort = false;
  }

  bool needFiltered = false;
  if (consumer.select->empty()) {
    sql.str("");
    sql << "select "
        << " msgs.num, "
        << " msgs.message_id,"
        << " msgs.priority, "
        << " msgs.persistent, "
        << " msgs.correlation_id, "
        << " msgs.reply_to, "
        << " msgs.type, "
        << " msgs.client_timestamp, "
        << " msgs.ttl, "
        << " msgs.expiration, "
        << " msgs.created_time, "
        << " msgs.body_type,"
        << " msgs.delivery_count, "
        << " msgs.group_id, "
        << " msgs.group_seq "
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

    consumer::Msg tempMsg;
    dbSession.beginTX(_extParentID);
    TRY_POCO_DATA_EXCEPTION {
      Poco::Data::Statement select(dbSession());

      select << sql.str(), Poco::Data::Keywords::into(tempMsg.num), Poco::Data::Keywords::into(tempMsg.messageId),
          Poco::Data::Keywords::into(tempMsg.priority), Poco::Data::Keywords::into(tempMsg.persistent),
          Poco::Data::Keywords::into(tempMsg.correlationID), Poco::Data::Keywords::into(tempMsg.replyTo), Poco::Data::Keywords::into(tempMsg.type),
          Poco::Data::Keywords::into(tempMsg.timestamp), Poco::Data::Keywords::into(tempMsg.ttl), Poco::Data::Keywords::into(tempMsg.expiration),
          Poco::Data::Keywords::into(tempMsg.screated), Poco::Data::Keywords::into(tempMsg.bodyType),
          Poco::Data::Keywords::into(tempMsg.deliveryCount), Poco::Data::Keywords::into(tempMsg.groupID),
          Poco::Data::Keywords::into(tempMsg.groupSeq), Poco::Data::Keywords::range(0, 1);

      while (!select.done()) {
        tempMsg.reset();
        select.execute();
        if (!tempMsg.messageId.empty() && needFiltered) {
          storage::MappedDBMessage mappedDBMessage(tempMsg.messageId, *this);
          mappedDBMessage.dbmsConnection = dbSession.dbmsConnnectionRef();
          if (consumer.selector->filter(mappedDBMessage)) {
            auto sMessage = makeMessage(dbSession, tempMsg, consumer, useFileLink);
            if (sMessage) {
              consumer.select->push_back(std::move(sMessage));
            }
          }
        } else if (!tempMsg.messageId.empty() && !needFiltered) {
          auto sMessage = makeMessage(dbSession, tempMsg, consumer, useFileLink);
          if (sMessage) {
            consumer.select->push_back(std::move(sMessage));
          }
        }
      }
      setMessagesToWasSent(dbSession, consumer);
    }
    CATCH_POCO_DATA_EXCEPTION_PURE("get message", sql.str(), ERROR_ON_GET_MESSAGE)
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
void Storage::saveMessageProperties(const upmq::broker::Session &session, const Message &message) {
  storage::DBMSSession &dbSession = *session.currentDBSession;
  std::stringstream sql;
  std::string upsert = "insert or replace";
  std::string postfix;
  if (STORAGE_CONFIG.connection.props.dbmsType == storage::Postgresql) {
    upsert = "insert";
  }

  NextBindParam nextParam;

  for (google::protobuf::Map<std::string, Proto::Property>::const_iterator it = message.property().begin(); it != message.property().end(); ++it) {
    nextParam.reset();
    sql.str("");
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
           "  is_null) values ("
        << "\'" << message.message_id() << "\'"
        << ","
        << "\'" << it->first << "\'"
        << "," << it->second.PropertyValue_case() << ",";
    bool isNull = it->second.is_null();
    switch (it->second.PropertyValue_case()) {
      case Property::kValueString: {
        sql << nextParam();
        sql << ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::useRef(it->second.value_string()), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueChar: {
        sql << "NULL, " << it->second.value_char() << " , NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueBool: {
        bool boolValue = it->second.value_bool();
        sql << "NULL, NULL, " << nextParam();
        sql << ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(boolValue), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueByte: {
        sql << "NULL, NULL, NULL, " << it->second.value_byte() << ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueShort: {
        sql << "NULL, NULL, NULL, NULL, " << it->second.value_short() << ", NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueInt: {
        sql << "NULL, NULL, NULL, NULL, NULL, " << it->second.value_int() << ", NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueLong: {
        sql << "NULL, NULL, NULL, NULL, NULL, NULL, " << it->second.value_long() << ", NULL, NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueFloat: {
        float fval = it->second.value_float();
        sql << "NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ", NULL, NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(fval), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueDouble: {
        double dval = it->second.value_double();
        sql << "NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ", NULL, NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        dbSession << sql.str(), Poco::Data::Keywords::use(dval), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueBytes: {
        sql << "NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ", NULL, " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        Poco::Data::BLOB blob((const unsigned char *)it->second.value_bytes().c_str(), it->second.value_bytes().size());
        dbSession << sql.str(), Poco::Data::Keywords::use(blob), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      case Property::kValueObject: {
        sql << "NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, " << nextParam();
        sql << ", " << nextParam();
        sql << ")" << postfix << ";" << non_std_endl;
        Poco::Data::BLOB blob((const unsigned char *)it->second.value_object().c_str(), it->second.value_object().size());
        dbSession << sql.str(), Poco::Data::Keywords::use(blob), Poco::Data::Keywords::use(isNull), Poco::Data::Keywords::now;
      } break;

      default:
        sql.str("");
        break;
    }
  }
}
void Storage::begin(const Session &session, const std::string &extParentId) {
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
  TRY_POCO_DATA_EXCEPTION {
    storage::DBMSConnectionPool::doNow(mainTXsql);
    for (const auto &index : mainTXsqlIndexes) {
      storage::DBMSConnectionPool::doNow(index);
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't create tx_table", mainTXsql, ERROR_ON_BEGIN)
}
void Storage::commit(const Session &session) {
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
  dbSession->beginTX(session.id());

  TRY_POCO_DATA_EXCEPTION { *dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't commit", sql.str(), ERROR_ON_COMMIT)
  TRY_POCO_DATA_EXCEPTION { dropTXTable(*dbSession, mainTXTable); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't commit", mainTXTable, ERROR_ON_COMMIT)
  TRY_POCO_DATA_EXCEPTION {
    session.currentDBSession = std::move(dbSession);
    removeMessagesBySession(session);
  }
  CATCH_POCO_DATA_EXCEPTION("can't commit (removeMessagesBySession)", session.id(), session.currentDBSession.reset(nullptr), ERROR_ON_COMMIT)

  session.currentDBSession->commitTX();
  session.currentDBSession.reset(nullptr);

  _parent->postNewMessageEvent();
}
void Storage::abort(const Session &session) {
  std::string mainTXTable = "\"" + std::to_string(Poco::hash(_extParentID + "_" + session.txName())) + "\"";
  std::stringstream sql;
  bool tbExist = false;
  sql << "select 1 from " << mainTXTable << ";";
  std::unique_ptr<storage::DBMSSession> dbSession = dbms::Instance().dbmsSessionPtr();
  dbSession->beginTX(session.id());
  TRY_POCO_DATA_EXCEPTION {
    *dbSession << sql.str(), Poco::Data::Keywords::now;
    tbExist = true;
  }
  catch (PDSQLITE::InvalidSQLStatementException &issex) {
    UNUSED_VAR(issex);
    tbExist = false;
  }
  CATCH_POCO_DATA_EXCEPTION_NO_INVALID_SQL("can't check table on existence", sql.str(), , ERROR_ON_ABORT)

  if (tbExist) {
    sql.str("");
    sql << "delete from " << STORAGE_CONFIG.messageJournal(_parent->name()) << " where message_id in ("
        << " select message_id from " << mainTXTable << ");";

    TRY_POCO_DATA_EXCEPTION { *dbSession << sql.str(), Poco::Data::Keywords::now; }
    CATCH_POCO_DATA_EXCEPTION_PURE("can't abort", sql.str(), ERROR_ON_ABORT)
    sql.str("");
    sql << "delete from " << _propertyTableID << " where message_id in ("
        << " select message_id from " << mainTXTable << ");";
    TRY_POCO_DATA_EXCEPTION { *dbSession << sql.str(), Poco::Data::Keywords::now; }
    CATCH_POCO_DATA_EXCEPTION_PURE("can't abort", sql.str(), ERROR_ON_ABORT)
    TRY_POCO_DATA_EXCEPTION { dropTXTable(*dbSession, mainTXTable); }
    CATCH_POCO_DATA_EXCEPTION_PURE("can't abort", sql.str(), ERROR_ON_ABORT)
  }
  TRY_POCO_DATA_EXCEPTION {
    session.currentDBSession = std::move(dbSession);
    resetMessagesBySession(session);
  }
  CATCH_POCO_DATA_EXCEPTION("can't abort", sql.str(), session.currentDBSession.reset(nullptr), ERROR_ON_ABORT)

  session.currentDBSession->commitTX();
  session.currentDBSession.reset(nullptr);
  _parent->postNewMessageEvent();
}
void Storage::dropTXTable(storage::DBMSSession &dbSession, const std::string &mainTXTable) const {
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
  storage::DBMSSession &dbSession = session.currentDBSession.get();
  TRY_POCO_DATA_EXCEPTION {
    Poco::Data::Statement select(dbSession());
    MessageInfo messageInfo;
    select << sql.str(), Poco::Data::Keywords::into(messageInfo.tuple), Poco::Data::Keywords::range(0, 1);
    while (!select.done()) {
      select.execute();
      if (!(messageInfo.tuple.get<message::field_message_id.position>()).empty()) {
        result.push_back(messageInfo);
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't get all messages below id", sql.str(), ERROR_ON_ACK_MESSAGE)
  return result;
}
void Storage::setMessageToWasSent(const std::string &messageID, const Consumer &consumer) {
  std::stringstream sql;
  sql << "update " << _messageTableID << " set delivery_status = " << message::WAS_SENT << ",    consumer_id = \'" << consumer.id << "\'"
      << ",    delivery_count  = delivery_count + 1";
  if (consumer.session.type == SESSION_TRANSACTED) {
    consumer.session.txName = BROKER::Instance().currentTransaction(consumer.clientID, consumer.session.id);
    sql << ",  transaction_id = \'" << consumer.session.txName << "\'";
  }
  sql << " where message_id = \'" << messageID << "\'"
      << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't set message to was_sent ", sql.str(), ERROR_STORAGE)
}
void Storage::setMessagesToWasSent(storage::DBMSSession &dbSession, const Consumer &consumer) {
  if (!consumer.select->empty()) {
    std::stringstream sql;
    sql << "update " << _messageTableID << " set delivery_status = " << message::WAS_SENT << ",    consumer_id = \'" << consumer.id << "\'"
        << ",    delivery_count  = delivery_count + 1";
    if (consumer.session.type == SESSION_TRANSACTED) {
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
    TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
    CATCH_POCO_DATA_EXCEPTION_PURE("can't set message to was_sent ", sql.str(), ERROR_STORAGE)
  }
}
void Storage::setMessageToDelivered(const upmq::broker::Session &session, const std::string &messageID) {
  std::stringstream sql;
  sql << "update " << _messageTableID << " set delivery_status = " << message::DELIVERED << " where message_id = \'" << messageID << "\'"
      << ";";
  storage::DBMSSession &dbSession = session.currentDBSession.get();
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't set message to delivered", sql.str(), ERROR_STORAGE)
}
void Storage::setMessageToLastInGroup(const Session &session, const std::string &messageID) {
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
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't set message to last_in_group", sql.str(), ERROR_ON_SAVE_MESSAGE)
  if (!externConnection) {
    dbSession.commitTX();
  }
}
std::string Storage::saveTableName(const Session &session) const {
  std::string messageTable = _messageTableID;
  if (session.isTransactAcknowledge()) {
    messageTable = "\"" + std::to_string(Poco::hash(_extParentID + "_" + session.txName())) + "\"";
  }
  return messageTable;
}
void Storage::setMessagesToNotSent(const Consumer &consumer) {
  std::stringstream sql;
  sql << "update " << _messageTableID << " set delivery_status = " << message::NOT_SENT << " where consumer_id like \'%" << consumer.id << "%\'";
  if (consumer.session.type == SESSION_TRANSACTED) {
    sql << " and transaction_id = \'" << consumer.session.txName << "\'";
  }
  sql << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT("can't set messages to not-sent", sql.str(), ERROR_STORAGE)
}
void Storage::copyTo(Storage &storage, const Consumer &consumer) {
  storage.resetNonPersistent(_nonPersistent);

  bool withSelector = (consumer.selector && !consumer.selector->expression().empty());
  std::stringstream sql;

  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();

  sql.str("");
  if (withSelector) {
    sql << " select message_id, priority, persistent, correlation_id, "
           "reply_to, type, client_timestamp, ttl, "
           "expiration, body_type, client_id, group_id, group_seq from "
        << _messageTableID << " where delivery_status <> " << message::DELIVERED << " order by num asc;";
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

  dbSession.beginTX(consumer.objectID);

  TRY_POCO_DATA_EXCEPTION {
    if (withSelector) {
      std::string messageID;
      int priority;
      int persistent;
      Poco::Nullable<std::string> correlationID;
      Poco::Nullable<std::string> replyTo;
      std::string type;
      std::string clientTimestamp;
      Poco::Int64 ttl;
      Poco::Int64 expiration;
      int bodyType;
      std::string clientID;
      Poco::Nullable<std::string> groupID;
      int groupSeq;

      auto &session = dbSession();
      Poco::Data::Statement select(session);
      Poco::Data::Statement insert(session);
      select << sql.str(), Poco::Data::Keywords::into(messageID), Poco::Data::Keywords::into(priority), Poco::Data::Keywords::into(persistent),
          Poco::Data::Keywords::into(correlationID), Poco::Data::Keywords::into(replyTo), Poco::Data::Keywords::into(type),
          Poco::Data::Keywords::into(clientTimestamp), Poco::Data::Keywords::into(ttl), Poco::Data::Keywords::into(expiration),
          Poco::Data::Keywords::into(bodyType), Poco::Data::Keywords::into(clientID), Poco::Data::Keywords::into(groupID),
          Poco::Data::Keywords::into(groupSeq), Poco::Data::Keywords::range(0, 1);
      NextBindParam nextParam;
      while (!select.done()) {
        nextParam.reset();
        select.execute();
        if (!messageID.empty()) {
          storage::MappedDBMessage mappedDBMessage(messageID, *this);
          mappedDBMessage.dbmsConnection = dbSession.dbmsConnnectionRef();
          if (consumer.selector->filter(mappedDBMessage)) {
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
            insert << sql.str(), Poco::Data::Keywords::use(messageID), Poco::Data::Keywords::use(priority), Poco::Data::Keywords::use(persistent),
                Poco::Data::Keywords::use(correlationID), Poco::Data::Keywords::use(replyTo), Poco::Data::Keywords::use(type),
                Poco::Data::Keywords::use(clientTimestamp), Poco::Data::Keywords::use(ttl), Poco::Data::Keywords::use(expiration),
                Poco::Data::Keywords::use(bodyType), Poco::Data::Keywords::use(clientID), Poco::Data::Keywords::use(groupID),
                Poco::Data::Keywords::use(groupSeq), Poco::Data::Keywords::now;
          }
        }
      }
    } else {
      dbSession << sql.str(), Poco::Data::Keywords::now;
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't copy messages to the browser", sql.str(), ERROR_ON_BROWSER)
  sql.str("");
  sql << "update " << STORAGE_CONFIG.messageJournal(_parent->name()) << " set subscribers_count = subscribers_count + 1 where message_id in ("
      << "select message_id from " << storage.messageTableID() << ");";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't copy messages to the browser", sql.str(), ERROR_ON_BROWSER)
  sql.str("");
  sql << "insert into " << storage.propertyTableID() << " select * from " << _propertyTableID << " where message_id in "
      << "("
      << " select message_id from " << storage.messageTableID() << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't copy messages to the browser", sql.str(), ERROR_ON_BROWSER)
  dbSession.commitTX();
}
void Storage::resetNonPersistent(const Storage::NonPersistentMessagesListType &nonPersistentMessagesList) {
  _nonPersistent.clear();
  _nonPersistent = nonPersistentMessagesList;
}
int64_t Storage::size() {
  std::stringstream sql;
  Poco::Int64 result = 0;
  sql << "select count(*) from " << _messageTableID << " as msgs"
      << " where delivery_status <> " << message::DELIVERED;
  sql << ";";

  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  dbSession.beginTX(_messageTableID, storage::DBMSSession::TransactionMode::READ);
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::into(result), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT("can't get queue size", sql.str(), ERROR_STORAGE)
  dbSession.commitTX();
  return result;
}
std::shared_ptr<MessageDataContainer> Storage::makeMessage(storage::DBMSSession &dbSession,
                                                           const consumer::Msg &msgInfo,
                                                           const Consumer &consumer,
                                                           bool useFileLink) {
  std::shared_ptr<MessageDataContainer> sMessage;
  if (!msgInfo.messageId.empty()) {
    bool ttlIsOut = checkTTLIsOut(msgInfo.screated, msgInfo.ttl);

    if (ttlIsOut) {
      removeMessage(msgInfo.messageId, dbSession);
      return {};
    }

    bool needToFillProperties = true;

    sMessage = std::make_shared<MessageDataContainer>(STORAGE_CONFIG.data.get().toString());
    try {
      Proto::Message &message = sMessage->createMessageHeader(consumer.objectID);
      sMessage->clientID = Poco::replace(consumer.clientID, "-browser", "");
      sMessage->handlerNum = consumer.tcpNum;

      message.set_message_id(msgInfo.messageId);
      message.set_destination_uri(_parent->uri());
      message.set_priority(msgInfo.priority);
      message.set_persistent(msgInfo.persistent == 1);
      message.set_sender_id(BROKER::Instance().id());
      sMessage->data.clear();
      if (msgInfo.persistent == 1) {
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
      } else {
        auto item = _nonPersistent.find(msgInfo.messageId);
        if (item.hasValue()) {
          needToFillProperties = (*item)->message().property_size() > 0;
          sMessage->data = (*item)->data;
        } else {
          removeMessage(msgInfo.messageId, dbSession);
          return {};
        }
      }
      if (!msgInfo.correlationID.isNull()) {
        message.set_correlation_id(msgInfo.correlationID.value());
      }
      if (!msgInfo.replyTo.isNull()) {
        message.set_reply_to(msgInfo.replyTo);
      }
      message.set_type(msgInfo.type);
      message.set_timestamp(msgInfo.timestamp);
      message.set_timetolive(msgInfo.ttl);
      message.set_expiration(msgInfo.expiration);
      if (sMessage) {
        sMessage->setDeliveryCount(msgInfo.deliveryCount);
      }
      message.set_body_type(msgInfo.bodyType);
      message.set_session_id(consumer.session.id);
      if (!msgInfo.groupID.value().empty()) {
        Poco::StringTokenizer groupIDAll(msgInfo.groupID, "+", Poco::StringTokenizer::TOK_TRIM);
        message.set_group_id(groupIDAll[0]);
      } else {
        message.set_group_id(msgInfo.groupID.value());
      }
      message.set_group_seq(msgInfo.groupSeq);
      if (needToFillProperties) {
        fillProperties(dbSession, message);
      }
    } catch (Exception &ex) {
      removeMessage(msgInfo.messageId, dbSession);
      throw Exception(ex);
    }
  }
  return sMessage;
}
void Storage::fillProperties(storage::DBMSSession &dbSession, Proto::Message &message) {
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
  TRY_POCO_DATA_EXCEPTION {
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
  }
  CATCH_POCO_DATA_EXCEPTION_PURE_TROW_INVALID_SQL("can't fill properties", sql.str(), ERROR_ON_GET_MESSAGE)
}
void Storage::dropTables() {
  std::stringstream sql;
  sql << "drop table if exists " << _messageTableID << ";" << non_std_endl;
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT("can't drop table", sql.str(), ERROR_ON_UNSUBSCRIPTION)

  sql.str("");
  sql << "drop table if exists " << _propertyTableID << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT("can't drop table", sql.str(), ERROR_ON_UNSUBSCRIPTION)
}
bool Storage::hasTransaction(const Session &session) const {
  upmq::ScopedReadRWLock readRWLock(_txSessionsLock);
  return (_txSessions.find(session.id()) != _txSessions.end());
}
}  // namespace broker
}  // namespace upmq
