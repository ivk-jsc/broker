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

#ifndef BROKER_STORAGE_H
#define BROKER_STORAGE_H

#include <Poco/RWLock.h>
#include <memory>
#include <string>
#include "FixedSizeUnorderedMap.h"
#include "DBMSSession.h"
#include "MessageDataContainer.h"
#include "MoveableRWLock.h"

namespace upmq {
namespace broker {

class Session;
class Destination;
class Consumer;
namespace consumer {
struct Msg;
}

class Storage {
 public:
  using NonPersistentMessagesListType = FSUnorderedMap<std::string, std::shared_ptr<MessageDataContainer>>;
  /// @brief TransactSessionsListType - set<transact_session_id>
  using TransactSessionsListType = std::unordered_set<std::string>;

 private:
  std::string _messageTableID;
  std::string _propertyTableID;
  const Destination *_parent;
  NonPersistentMessagesListType _nonPersistent;
  std::string _extParentID;
  TransactSessionsListType _txSessions;
  mutable upmq::MRWLock _txSessionsLock;

 private:
  std::string saveTableName(const upmq::broker::Session &session) const;
  std::shared_ptr<MessageDataContainer> makeMessage(storage::DBMSSession &dbSession,
                                                    const consumer::Msg &msgInfo,
                                                    const Consumer &consumer,
                                                    bool useFileLink);
  void fillProperties(storage::DBMSSession &dbSession, Proto::Message &message);
  int deleteMessageHeader(storage::DBMSSession &dbSession, const std::string &messageID);
  void deleteMessageProperties(storage::DBMSSession &dbSession, const std::string &messageID);
  int getSubscribersCount(storage::DBMSSession &dbSession, const std::string &messageID);
  void updateSubscribersCount(storage::DBMSSession &dbSession, const std::string &messageID);
  void deleteMessageInfoFromJournal(storage::DBMSSession &dbSession, const std::string &messageID);
  void deleteMessageDataIfExists(const std::string &messageID, int persistent);
  void saveMessageHeader(const upmq::broker::Session &session, const MessageDataContainer &sMessage);
  void saveMessageProperties(const upmq::broker::Session &session, const Message &message);
  bool checkTTLIsOut(const std::string &stringMessageTime, Poco::Int64 ttl);

 public:
  explicit Storage(const std::string &messageTableID, size_t nonPersistentSize);
  Storage(Storage &&) = default;
  Storage &operator=(Storage &&) = default;
  virtual ~Storage();
  void setParent(const broker::Destination *parent);
  const std::string &messageTableID() const;
  const std::string &propertyTableID() const;
  void save(const upmq::broker::Session &session, const MessageDataContainer &sMessage);
  std::shared_ptr<MessageDataContainer> get(const Consumer &consumer, bool useFileLink);
  void removeGroupMessage(const std::string &groupID, const upmq::broker::Session &session);
  void removeMessagesBySession(const upmq::broker::Session &session);
  void resetMessagesBySession(const upmq::broker::Session &session);
  void removeMessage(const std::string &messageID, storage::DBMSSession &extDBSession);
  const std::string &uri() const;
  void begin(const upmq::broker::Session &session, const std::string &extParentId = "");
  void commit(const upmq::broker::Session &session);
  void abort(const upmq::broker::Session &session);

  std::string generateSQLMainTable(const std::string &tableName) const;
  std::vector<std::string> generateSQLMainTableIndexes(const std::string &tableName) const;
  std::string generateSQLProperties() const;
  std::vector<MessageInfo> getMessagesBelow(const upmq::broker::Session &session, const std::string &messageID) const;
  void setMessageToWasSent(const std::string &messageID, const Consumer &consumer);
  void setMessagesToWasSent(storage::DBMSSession &dbSession, const Consumer &consumer);
  void setMessageToDelivered(const upmq::broker::Session &session, const std::string &messageID);
  void setMessagesToNotSent(const Consumer &consumer);
  void setMessageToLastInGroup(const upmq::broker::Session &session, const std::string &messageID);
  void dropTXTable(storage::DBMSSession &dbSession, const std::string &mainTXTable) const;
  void copyTo(Storage &storage, const Consumer &consumer);
  void resetNonPersistent(const NonPersistentMessagesListType &nonPersistentMessagesList);
  int64_t size();
  void dropTables();
  bool hasTransaction(const upmq::broker::Session &session) const;
  message::GroupStatus checkIsGroupClosed(const MessageDataContainer &sMessage, const upmq::broker::Session &session) const;
};
}  // namespace broker
}  // namespace upmq
#endif  // BROKER_STORAGE_H
