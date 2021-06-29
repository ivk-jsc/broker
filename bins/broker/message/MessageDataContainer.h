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

#ifndef BROKER_STORAGEMESSAGE_H
#define BROKER_STORAGEMESSAGE_H

#include <Poco/AutoPtr.h>
#include <Poco/FileStream.h>
#include <Poco/Path.h>
#include <map>
#include <memory>
#include <string>
#include <fstream>
#include "MessageInfo.h"
#include "ProtoBuf.h"
#include "StorageDefines.h"
#include "PropertyHandler.h"

namespace upmq {
namespace broker {

class MessageDataContainer {
 public:
  MessageDataContainer();
  explicit MessageDataContainer(std::string path);
  explicit MessageDataContainer(Poco::Path path);
  explicit MessageDataContainer(Proto::ProtoMessage *headerProtoMessage);
  MessageDataContainer(Proto::ProtoMessage *headerProtoMessage, Proto::Body *dataBody);
  MessageDataContainer(std::string path, std::string _header, std::string _data, bool useFileLink);
  MessageDataContainer &operator=(const MessageDataContainer &o) = delete;
  MessageDataContainer &operator=(MessageDataContainer &&o) = default;
  MessageDataContainer(const MessageDataContainer &o) = delete;
  MessageDataContainer(MessageDataContainer &&) = default;
  virtual ~MessageDataContainer();
  Proto::Connect &createConnect(const std::string &objectID);
  Proto::Connected &createConnected(const std::string &objectID);
  Proto::ClientInfo &createClientInfo(const std::string &objectID);
  Proto::Session &createSession(const std::string &objectID);
  Proto::Unsession &createUnsession(const std::string &objectID);
  Proto::Unsender &createUnsender(const std::string &objectID);
  Proto::Subscribe &createSubscribe(const std::string &objectID);
  Proto::Subscription &createSubscription(const std::string &objectID);
  Proto::Unsubscribe &createUnsubscribe(const std::string &objectID);
  Proto::Unsubscription &createUnsubscription(const std::string &objectID);
  Proto::Begin &createBegin(const std::string &objectID);
  Proto::Commit &createCommit(const std::string &objectID);
  Proto::Abort &createAbort(const std::string &objectID);
  Proto::Ack &createAck(const std::string &objectID);
  Proto::Message &createMessageHeader(const std::string &objectID);
  Proto::Body &createMessageBody();
  Proto::BrowserInfo &createBrowserInfo(const std::string &objectID);
  Proto::Pong &createPong(const std::string &objectID);
  void serialize();

  bool empty() const;

  Proto::ProtoMessage &protoMessage() const;
  const Proto::Connect &connect() const;
  const Proto::ClientInfo &clientInfo() const;
  const Proto::Disconnect &disconnect() const;
  const Proto::Session &session() const;
  const Proto::Unsession &unsession() const;
  const Proto::Destination &destination() const;
  const Proto::Undestination &undestination() const;
  const Proto::Sender &sender() const;
  const Proto::Unsender &unsender() const;
  const Proto::Subscription &subscription() const;
  const Proto::Subscribe &subscribe() const;
  const Proto::Unsubscribe &unsubscribe() const;
  const Proto::Unsubscription &unsubscription() const;
  const Proto::Begin &begin() const;
  const Proto::Commit &commit() const;
  const Proto::Abort &abort() const;
  const Proto::Ack &ack() const;
  const Proto::Message &message() const;
  Proto::Message &mutableMessage() const;
  const Proto::Browser &browser() const;
  Proto::ProtoMessage::ProtoMessageTypeCase type() const;
  bool isPing() const;
  bool isConnect() const;
  bool isClientInfo() const;
  bool isDisconnect() const;
  bool isSession() const;
  bool isUnsession() const;
  bool isDestination() const;
  bool isUndestination() const;
  bool isSender() const;
  bool isUnsender() const;
  bool isSubscription() const;
  bool isSubscribe() const;
  bool isUnsubscribe() const;
  bool isUnsubscription() const;
  bool isBegin() const;
  bool isCommit() const;
  bool isAbort() const;
  bool isAck() const;
  bool isMessage() const;
  bool isForServer() const;
  bool isNotForServer() const;
  bool isNeedReceipt() const;
  bool isReceipt() const;
  bool isError() const;
  bool isBrowser() const;
  std::string receiptId() const;
  std::string objectID() const;
  void setObjectID(const std::string &newObjectID);
  int rrID() const;
  void setRRID(int rrID);
  void setRedelivered(bool status);
  void setDeliveryCount(int count);
  void debugPrintHeader() const;
  std::string typeName() const;
  std::string header;
  mutable std::string data;
  std::string clientID;
  size_t handlerNum = 0;
  void reparseHeader();
  void resetSessionId(const std::string &sessionID);
  MessageDataContainer *clone() const;
  bool withFile() const;
  void setWithFile(bool withFile);
  uint64_t dataSize() const;
  void appendData(const char *part, size_t size);
  std::vector<char> getPartOfData(size_t offset, size_t size);
  void setData(const std::string &in);
  void flushData();
  const Poco::Path &path() const;
  void removeLinkedFile();
  bool isDataExists() const;
  void initPersistentDataFileLink();
  void moveDataTo(const std::string &uri) const;
  std::fstream &fileStream();
  bool toDisconnect = false;
  void processProperties(upmq::broker::PropertyHandler &handler, const std::string &identifier) const;
  int64_t created() const;
  void setCreated(int64_t created);

 private:
  Poco::Path _path;
  mutable std::unique_ptr<Proto::ProtoMessage> _headerMessage;
  mutable std::unique_ptr<Proto::Body> _dataMessage;
  bool _withFile = false;
  std::unique_ptr<std::fstream> _dataFileStream;
  int64_t _created = {0};
  void initHeader() const;
  void newMessage(const std::string &objectID);
  void initDataFileStream();
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_STORAGEMESSAGE_H
