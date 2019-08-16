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

#include "MessageDataContainer.h"
#include <Exchange.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <memory>
#include "Defines.h"
#include "Exception.h"
#include "fake_cpp14.h"
#include <Poco/UTF8Encoding.h>
#include <Poco/UnicodeConverter.h>

namespace upmq {
namespace broker {

MessageDataContainer::MessageDataContainer() : _path(), _headerMessage(nullptr), _dataMessage(nullptr), _withFile(false) {}

MessageDataContainer::MessageDataContainer(std::string path)
    : _path(std::move(path)), _headerMessage(nullptr), _dataMessage(nullptr), _withFile(false) {}

MessageDataContainer::MessageDataContainer(std::string path, std::string _header, std::string _data, bool useFileLink)
    : header(std::move(_header)),
      data(std::move(_data)),
      _path(std::move(path)),
      _headerMessage(nullptr),
      _dataMessage(nullptr),
      _withFile(useFileLink) {
  initDataFileStream();
}
void MessageDataContainer::initDataFileStream() {
  if (_withFile) {
    if (!_dataFileStream) {
      Poco::Path dataFilePath(_path);
      dataFilePath.append(data).makeFile();
      Poco::File pathDir(dataFilePath.parent());
      if (!pathDir.exists()) {
        pathDir.createDirectories();
      }

      _dataFileStream = std::make_unique<std::fstream>();
#ifdef _WIN32
      std::wstring wFilePath;
      Poco::UnicodeConverter::toUTF16(dataFilePath.toString(), wFilePath);
#else
      std::string wFilePath = dataFilePath.toString();
#endif
      _dataFileStream->open(wFilePath, std::ios_base::out | std::ios_base::in | std::ios_base::binary | std::ios_base::app);
      if (!_dataFileStream->is_open()) {
        throw EXCEPTION("can't open data file", dataFilePath.toString(), ERROR_UNKNOWN);
      }
    }
  }
}
MessageDataContainer::MessageDataContainer(ProtoMessage *headerProtoMessage)
    : _headerMessage(headerProtoMessage), _dataMessage(nullptr), _withFile(false) {
  data.clear();
}
MessageDataContainer::MessageDataContainer(ProtoMessage *headerProtoMessage, Body *dataBody)
    : _headerMessage(headerProtoMessage), _dataMessage(dataBody), _withFile(false) {
  header = _headerMessage->SerializeAsString();
  data = _dataMessage->SerializeAsString();
}
MessageDataContainer::~MessageDataContainer() = default;
bool MessageDataContainer::empty() const { return (!_headerMessage && !_dataMessage && header.empty() && data.empty()); }
ProtoMessage &MessageDataContainer::protoMessage() const {
  initHeader();
  return *_headerMessage;
}
const Connect &MessageDataContainer::connect() const {
  initHeader();
  return _headerMessage->connect();
}
const Proto::Disconnect &MessageDataContainer::disconnect() const {
  initHeader();
  return _headerMessage->disconnect();
}
void MessageDataContainer::initHeader() const {
  if (!header.empty() && !_headerMessage) {
    _headerMessage = std::make_unique<ProtoMessage>();
    if (!_headerMessage->ParseFromString(header)) {
      _headerMessage->Clear();
    }
  }
}
ProtoMessage::ProtoMessageTypeCase MessageDataContainer::type() const {
  initHeader();
  return _headerMessage->ProtoMessageType_case();
}
bool MessageDataContainer::isPing() const { return (type() == ProtoMessage::kPing); }
bool MessageDataContainer::isConnect() const { return (type() == ProtoMessage::kConnect); }
bool MessageDataContainer::isClientInfo() const { return (type() == ProtoMessage::kClientInfo); }
bool MessageDataContainer::isDisconnect() const { return (type() == ProtoMessage::kDisconnect); }
bool MessageDataContainer::isSession() const { return (type() == ProtoMessage::kSession); }
bool MessageDataContainer::isUnsession() const { return (type() == ProtoMessage::kUnsession); }
bool MessageDataContainer::isDestination() const { return (type() == ProtoMessage::kDestination); }
bool MessageDataContainer::isUndestination() const { return (type() == ProtoMessage::kUndestination); }
bool MessageDataContainer::isSender() const { return (type() == ProtoMessage::kSender); }
bool MessageDataContainer::isUnsender() const { return (type() == ProtoMessage::kUnsender); }
bool MessageDataContainer::isSubscription() const { return (type() == ProtoMessage::kSubscription); }
bool MessageDataContainer::isSubscribe() const { return (type() == ProtoMessage::kSubscribe); }
bool MessageDataContainer::isUnsubscribe() const { return (type() == ProtoMessage::kUnsubscribe); }
bool MessageDataContainer::isUnsubscription() const { return (type() == ProtoMessage::kUnsubscription); }
bool MessageDataContainer::isBegin() const { return (type() == ProtoMessage::kBegin); }
bool MessageDataContainer::isCommit() const { return (type() == ProtoMessage::kCommit); }
bool MessageDataContainer::isAbort() const { return (type() == ProtoMessage::kAbort); }
bool MessageDataContainer::isAck() const { return (type() == ProtoMessage::kAck); }
bool MessageDataContainer::isMessage() const { return (type() == ProtoMessage::kMessage); }
bool MessageDataContainer::isBrowser() const { return (type() == ProtoMessage::kBrowser); }
bool MessageDataContainer::isForServer() const {
  return (isConnect() || isClientInfo() || isDisconnect() || isSession() || isUnsession() || isDestination() || isUndestination() || isSender() ||
          isUnsender() || isSubscription() || isSubscribe() || isUnsubscribe() || isUnsubscription() || isBegin() || isCommit() || isAbort() ||
          isAck() || isMessage() || isBrowser() || isPing());
}
bool MessageDataContainer::isNotForServer() const { return !isForServer(); }
std::string MessageDataContainer::typeName() const {
  switch (type()) {
    case ProtoMessage::kConnect:
      return "connect";
    case ProtoMessage::kClientInfo:
      return "client_info";
    case ProtoMessage::kDisconnect:
      return "disconnect";
    case ProtoMessage::kSession:
      return "session";
    case ProtoMessage::kUnsession:
      return "unsession";
    case ProtoMessage::kDestination:
      return "destination";
    case ProtoMessage::kUndestination:
      return "undestination";
    case ProtoMessage::kSender:
      return "sender";
    case ProtoMessage::kUnsender:
      return "unsender";
    case ProtoMessage::kSubscription:
      return "subscription";
    case ProtoMessage::kSubscribe:
      return "subscribe";
    case ProtoMessage::kUnsubscribe:
      return "unsubscribe";
    case ProtoMessage::kUnsubscription:
      return "unsubscription";
    case ProtoMessage::kBegin:
      return "begin";
    case ProtoMessage::kCommit:
      return "commit";
    case ProtoMessage::kAbort:
      return "abort";
    case ProtoMessage::kAck:
      return "ack";
    case ProtoMessage::kMessage:
      return "message";
    case ProtoMessage::kConnected:
      return "connected";
    case ProtoMessage::kReceipt:
      return "receipt";
    case ProtoMessage::kError:
      return "error";
    case ProtoMessage::kBrowser:
      return "browser";
    case ProtoMessage::kBrowserInfo:
      return "browser_info";
    case ProtoMessage::kPing:
      return "ping";
    case ProtoMessage::kPong:
      return "pong";
    case ProtoMessage::PROTOMESSAGETYPE_NOT_SET:
    default:
      return "protomessagetype_not_set";
  }
}
const Message &MessageDataContainer::message() const {
  initHeader();
  return _headerMessage->message();
}
Proto::Message &MessageDataContainer::mutableMessage() const {
  initHeader();
  return *_headerMessage->mutable_message();
}
bool MessageDataContainer::isNeedReceipt() const {
  initHeader();
  switch (type()) {
    case ProtoMessage::kDisconnect: {
      const Proto::Disconnect &aDisconnect = disconnect();
      return !(aDisconnect.receipt_id().empty());
    }
    case ProtoMessage::kClientInfo: {
      const Proto::ClientInfo &aClientInfo = clientInfo();
      return !(aClientInfo.receipt_id().empty());
    }
    case ProtoMessage::kSession: {
      const Proto::Session &aSession = session();
      return !(aSession.receipt_id().empty());
    }
    case ProtoMessage::kUnsession: {
      const Proto::Unsession &aUnsession = unsession();
      return !(aUnsession.receipt_id().empty());
    }
    case ProtoMessage::kDestination: {
      const Proto::Destination &aDestination = destination();
      return !(aDestination.receipt_id().empty());
    }
    case ProtoMessage::kUndestination: {
      const Proto::Undestination &aUndestination = undestination();
      return !(aUndestination.receipt_id().empty());
    }
    case ProtoMessage::kSender: {
      const Proto::Sender &aSender = sender();
      return !(aSender.receipt_id().empty());
    }
    case ProtoMessage::kUnsender: {
      const Proto::Unsender &aUnsender = unsender();
      return !(aUnsender.receipt_id().empty());
    }
    case ProtoMessage::kSubscription: {
      const Proto::Subscription &aSubscription = subscription();
      return !(aSubscription.receipt_id().empty());
    }
    case ProtoMessage::kSubscribe: {
      const Proto::Subscribe &aSubscribe = subscribe();
      return !(aSubscribe.receipt_id().empty());
    }
    case ProtoMessage::kUnsubscribe: {
      const Proto::Unsubscribe &aUnsubscribe = unsubscribe();
      return !(aUnsubscribe.receipt_id().empty());
    }
    case ProtoMessage::kUnsubscription: {
      const Proto::Unsubscription &aUnsubscription = unsubscription();
      return !(aUnsubscription.receipt_id().empty());
    }
    case ProtoMessage::kBegin: {
      const Proto::Begin &aBegin = begin();
      return !(aBegin.receipt_id().empty());
    }
    case ProtoMessage::kCommit: {
      const Proto::Commit &aCommit = commit();
      return !(aCommit.receipt_id().empty());
    }
    case ProtoMessage::kAbort: {
      const Proto::Abort &aAbort = abort();
      return !(aAbort.receipt_id().empty());
    }
    case ProtoMessage::kMessage: {
      const Proto::Message &aMessage = message();
      return !(aMessage.receipt_id().empty());
    }
    case ProtoMessage::kAck: {
      const Proto::Ack &aAck = ack();
      return !(aAck.receipt_id().empty());
    }
    default:
      break;
  }
  return false;
}
const Proto::ClientInfo &MessageDataContainer::clientInfo() const {
  initHeader();
  return _headerMessage->client_info();
}
const Proto::Session &MessageDataContainer::session() const {
  initHeader();
  return _headerMessage->session();
}
const Proto::Unsession &MessageDataContainer::unsession() const {
  initHeader();
  return _headerMessage->unsession();
}
const Proto::Destination &MessageDataContainer::destination() const {
  initHeader();
  return _headerMessage->destination();
}
const Proto::Undestination &MessageDataContainer::undestination() const {
  initHeader();
  return _headerMessage->undestination();
}
const Proto::Sender &MessageDataContainer::sender() const {
  initHeader();
  return _headerMessage->sender();
}
const Proto::Unsender &MessageDataContainer::unsender() const {
  initHeader();
  return _headerMessage->unsender();
}
const Proto::Subscription &MessageDataContainer::subscription() const {
  initHeader();
  return _headerMessage->subscription();
}
const Proto::Subscribe &MessageDataContainer::subscribe() const {
  initHeader();
  return _headerMessage->subscribe();
}
const Proto::Unsubscribe &MessageDataContainer::unsubscribe() const {
  initHeader();
  return _headerMessage->unsubscribe();
}
const Proto::Unsubscription &MessageDataContainer::unsubscription() const {
  initHeader();
  return _headerMessage->unsubscription();
}
const Proto::Begin &MessageDataContainer::begin() const {
  initHeader();
  return _headerMessage->begin();
}
const Proto::Commit &MessageDataContainer::commit() const {
  initHeader();
  return _headerMessage->commit();
}
const Proto::Abort &MessageDataContainer::abort() const {
  initHeader();
  return _headerMessage->abort();
}
const Proto::Ack &MessageDataContainer::ack() const {
  initHeader();
  return _headerMessage->ack();
}
const Proto::Browser &MessageDataContainer::browser() const {
  initHeader();
  return _headerMessage->browser();
}
std::string MessageDataContainer::receiptId() const {
  initHeader();
  switch (type()) {
    case ProtoMessage::kDisconnect: {
      const Proto::Disconnect &aDisconnect = disconnect();
      return aDisconnect.receipt_id();
    }
    case ProtoMessage::kClientInfo: {
      const Proto::ClientInfo &aClientInfo = clientInfo();
      return aClientInfo.receipt_id();
    }
    case ProtoMessage::kSession: {
      const Proto::Session &aSession = session();
      return aSession.receipt_id();
    }
    case ProtoMessage::kUnsession: {
      const Proto::Unsession &aUnsession = unsession();
      return aUnsession.receipt_id();
    }
    case ProtoMessage::kDestination: {
      const Proto::Destination &aDestination = destination();
      return aDestination.receipt_id();
    }
    case ProtoMessage::kUndestination: {
      const Proto::Undestination &aUndestination = undestination();
      return aUndestination.receipt_id();
    }
    case ProtoMessage::kSender: {
      const Proto::Sender &aSender = sender();
      return aSender.receipt_id();
    }
    case ProtoMessage::kUnsender: {
      const Proto::Unsender &aUnsender = unsender();
      return aUnsender.receipt_id();
    }
    case ProtoMessage::kSubscription: {
      const Proto::Subscription &aSubscription = subscription();
      return aSubscription.receipt_id();
    }
    case ProtoMessage::kSubscribe: {
      const Proto::Subscribe &aSubscribe = subscribe();
      return aSubscribe.receipt_id();
    }
    case ProtoMessage::kUnsubscribe: {
      const Proto::Unsubscribe &aUnsubscribe = unsubscribe();
      return aUnsubscribe.receipt_id();
    }
    case ProtoMessage::kUnsubscription: {
      const Proto::Unsubscription &aUnsubscription = unsubscription();
      return aUnsubscription.receipt_id();
    }
    case ProtoMessage::kBegin: {
      const Proto::Begin &aBegin = begin();
      return aBegin.receipt_id();
    }
    case ProtoMessage::kCommit: {
      const Proto::Commit &aCommit = commit();
      return aCommit.receipt_id();
    }
    case ProtoMessage::kAbort: {
      const Proto::Abort &aAbort = abort();
      return aAbort.receipt_id();
    }
    case ProtoMessage::kMessage: {
      const Proto::Message &aMessage = message();
      return aMessage.message_id();
    }
    case ProtoMessage::kAck: {
      const Proto::Ack &aAck = ack();
      return aAck.receipt_id();
    }
    default:
      break;
  }
  return emptyString;
}
std::string MessageDataContainer::objectID() const {
  initHeader();
  return _headerMessage->object_id();
}
void MessageDataContainer::setObjectID(const std::string &newObjectID) {
  initHeader();
  _headerMessage->set_object_id(newObjectID);
}
int MessageDataContainer::rrID() const {
  initHeader();
  return _headerMessage->request_reply_id();
}
void MessageDataContainer::setRRID(int rrID) {
  if (_headerMessage) {
    _headerMessage->set_request_reply_id(rrID);
  }
}
void MessageDataContainer::setRedelivered(bool status) {
  initHeader();
  _headerMessage->mutable_message()->set_redelivered(status);
}
void MessageDataContainer::setDeliveryCount(int count) {
  initHeader();
  if (count > 0) {
    _headerMessage->mutable_message()->set_redelivered(true);
  }
  _headerMessage->mutable_message()->set_delivery_count(count + 1);
}
bool MessageDataContainer::isReceipt() const { return type() == ProtoMessage::kReceipt; }
bool MessageDataContainer::isError() const { return type() == ProtoMessage::kError; }
Proto::Connect &MessageDataContainer::createConnect(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_connect();
}
Proto::Connected &MessageDataContainer::createConnected(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_connected();
  ;
}
Proto::ClientInfo &MessageDataContainer::createClientInfo(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_client_info();
}
Proto::Session &MessageDataContainer::createSession(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_session();
}
Proto::Unsession &MessageDataContainer::createUnsession(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_unsession();
}
Proto::Unsender &MessageDataContainer::createUnsender(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_unsender();
}
Proto::Subscribe &MessageDataContainer::createSubscribe(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_subscribe();
}
Proto::Subscription &MessageDataContainer::createSubscription(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_subscription();
}
Proto::Unsubscribe &MessageDataContainer::createUnsubscribe(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_unsubscribe();
}
Proto::Unsubscription &MessageDataContainer::createUnsubscription(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_unsubscription();
}
Proto::Begin &MessageDataContainer::createBegin(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_begin();
}
Proto::Commit &MessageDataContainer::createCommit(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_commit();
}
Proto::Abort &MessageDataContainer::createAbort(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_abort();
}
Proto::Ack &MessageDataContainer::createAck(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_ack();
}
Proto::BrowserInfo &MessageDataContainer::createBrowserInfo(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_browser_info();
}
Proto::Pong &MessageDataContainer::createPong(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_pong();
}
Proto::Message &MessageDataContainer::createMessageHeader(const std::string &objectID) {
  newMessage(objectID);
  return *_headerMessage->mutable_message();
}
Proto::Body &MessageDataContainer::createMessageBody() {
  if (_dataMessage) {
    _dataMessage.reset(nullptr);
  }
  _dataMessage = std::make_unique<Body>();
  return *_dataMessage;
}
void MessageDataContainer::newMessage(const std::string &objectID) {
  if (_headerMessage) {
    _headerMessage.reset(nullptr);
  }
  if (_dataMessage) {
    _dataMessage.reset(nullptr);
  }
  header.clear();
  data.clear();
  _headerMessage = std::make_unique<ProtoMessage>();
  _headerMessage->set_object_id(objectID);
}
void MessageDataContainer::serialize() {
  if (_headerMessage) {
    header = _headerMessage->SerializeAsString();
  }
  if (_dataMessage) {
    data = _dataMessage->SerializeAsString();
  }
}
void MessageDataContainer::debugPrintHeader() const {
  if (_headerMessage) {
    _headerMessage->PrintDebugString();
  }
}
void MessageDataContainer::reparseHeader() {
  if (!header.empty()) {
    if (_headerMessage) {
      _headerMessage.reset(nullptr);
    }
    _headerMessage = std::make_unique<ProtoMessage>();
    if (!_headerMessage->ParseFromString(header)) {
      _headerMessage->Clear();
    }
  }
  if (_dataMessage) {
    _dataMessage.reset(nullptr);
  }
}
void MessageDataContainer::resetSessionId(const std::string &sessionID) {
  if (_headerMessage && _headerMessage->has_message()) {
    _headerMessage->mutable_message()->set_session_id(sessionID);
  }
}
MessageDataContainer *MessageDataContainer::clone() const {
  auto dataContainer = std::make_unique<MessageDataContainer>(_path);
  dataContainer->header = header;
  dataContainer->data = data;
  dataContainer->clientID = clientID;
  dataContainer->setWithFile(withFile());
  dataContainer->initHeader();
  return dataContainer.release();
}
bool MessageDataContainer::withFile() const { return _withFile; }
void MessageDataContainer::setWithFile(bool withFile) { _withFile = withFile; }

uint64_t MessageDataContainer::dataSize() const {
  if (_withFile) {
    Poco::Path dataFilePath(_path);
    dataFilePath.append(data).makeFile();
    Poco::File dataFile(dataFilePath);
    if (dataFile.exists()) {
      return dataFile.getSize();
    }
  } else {
    return data.size();
  }
  return 0;
}
void MessageDataContainer::appendData(const char *part, size_t size) {
  if (_withFile) {
    initDataFileStream();
    _dataFileStream->write(part, size);
  } else {
    data.append(part, size);
  }
}
std::vector<char> MessageDataContainer::getPartOfData(size_t offset, size_t size) {
  std::vector<char> v;
  if (offset > dataSize()) {
    return v;
  }
  uint64_t localSize = size;
  if (offset + size > dataSize()) {
    localSize = dataSize() - offset;
  }
  if (_withFile) {
    initDataFileStream();
    if (!_dataFileStream) {
      return v;
    }
    v.resize(static_cast<std::vector<char>::size_type>(localSize));
    _dataFileStream->seekg(offset, _dataFileStream->beg);
    _dataFileStream->read(&v[0], static_cast<std::streamsize>(localSize));
  } else {
    std::copy(data.c_str() + offset, data.c_str() + localSize, std::back_inserter(v));
  }
  return v;
}
void MessageDataContainer::setData(const std::string &in) { data = in; }
void MessageDataContainer::flushData() {
  if (_withFile) {
    if (_dataFileStream) {
      _dataFileStream->flush();
    }
  }
}
const std::string &MessageDataContainer::path() const { return _path; }
void MessageDataContainer::removeLinkedFile() {
  if (_withFile) {
    if (_dataFileStream) {
      _dataFileStream->close();
    }
    Poco::Path dataFilePath(_path);
    dataFilePath.append(data).makeFile();
    Poco::File tmp(dataFilePath);
    if (tmp.exists()) {
      tmp.remove();
    }
  }
}
bool MessageDataContainer::isDataExists() const {
  if (_withFile) {
    Poco::Path dataFilePath(_path);
    dataFilePath.append(data).makeFile();
    Poco::File tmp(dataFilePath);
    return tmp.exists();
  }
  return !data.empty();
}
void MessageDataContainer::initPersistentDataFileLink() {
  if (isMessage() && message().persistent()) {
    setWithFile(true);
    data = message().message_id();
    data[2] = '_';
    data = Exchange::mainDestinationPath(message().destination_uri()) + "/" + data;
  }
}
void MessageDataContainer::moveDataTo(const std::string &uri) const {
  if (_withFile) {
    Poco::Path fromDataFilePath(_path);
    fromDataFilePath.append(data).makeFile();
    std::string tempData = message().message_id();
    tempData[2] = '_';
    tempData = Exchange::mainDestinationPath(uri) + "/" + tempData;

    Poco::Path toDataFilePath(_path);
    toDataFilePath.append(tempData).makeFile();
    Poco::File toDataDir(toDataFilePath.parent());
    if (!toDataDir.exists()) {
      toDataDir.createDirectories();
    }
    Poco::File fromFile(fromDataFilePath);
    if (fromFile.exists()) {
      fromFile.renameTo(toDataFilePath.toString());
      data = tempData;
    }
  }
}
std::fstream &MessageDataContainer::fileStream() {
  initDataFileStream();
  return *_dataFileStream;
}
}  // namespace broker
}  // namespace upmq
