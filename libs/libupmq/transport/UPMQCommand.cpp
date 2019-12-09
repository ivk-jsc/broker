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

#include "UPMQCommand.h"
#include <cms/CMSException.h>
#include <cms/InvalidSelectorException.h>
#include <libupmq/ExceptionImpl.h>
#include <libupmq/ProtoHeader.h>
#include "decaf/util/Config.h"

using namespace std;

UPMQCommand::UPMQCommand()
    : commandId(0),
      correlationId(0),
      responseRequired(false),
      _header(new Proto::ProtoMessage()),
      _body(new Proto::Body()),
      _bodyBuff(nullptr),
      _bodyBuffSize(0),
      _headerString(EMPTY_STRING),
      _headerSize(0),
      _bodyString(EMPTY_STRING),
      _bodySize(0),
      _response(true),
      _consumer(nullptr) {}

UPMQCommand::UPMQCommand(const UPMQCommand &o)
    : commandId(o.commandId),
      correlationId(o.correlationId),
      responseRequired(o.responseRequired),
      _header(o._header == nullptr ? nullptr : new Proto::ProtoMessage(*o._header)),
      _body(o._body == nullptr ? nullptr : new Proto::Body(*o._body)),
      _bodyBuff(nullptr),
      _bodyBuffSize(o._bodyBuffSize),
      _headerString(o._headerString),
      _headerSize(o._headerSize),
      _bodyString(o._bodyString),
      _bodySize(o._bodySize),
      _response(o._response),
      _consumer(o._consumer) {
  if (_bodyBuffSize) {
    _bodyBuff = new unsigned char[size_t(_bodyBuffSize)]();
    memcpy(_bodyBuff, o._bodyBuff, size_t(_bodyBuffSize));
  }
}

UPMQCommand::UPMQCommand(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size)
    : commandId(0),
      correlationId(0),
      responseRequired(false),
      _header(header),
      _body(nullptr),
      _bodyBuff(body_buff),
      _bodyBuffSize(body_size),
      _headerString(EMPTY_STRING),
      _headerSize(0),
      _bodyString(EMPTY_STRING),
      _bodySize(0),
      _response(true),
      _consumer(nullptr) {}

bool UPMQCommand::isAck() const { return false; }

bool UPMQCommand::isConnect() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kConnect; }

bool UPMQCommand::isConnected() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kConnected; }

bool UPMQCommand::isDisconnect() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kDisconnect; }

bool UPMQCommand::isClientInfo() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kClientInfo; }

bool UPMQCommand::isSession() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kSession; }

bool UPMQCommand::isUnsession() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kUnsession; }

bool UPMQCommand::isSubscription() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kSubscription; }

bool UPMQCommand::isUnsubscription() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kUnsubscription; }

bool UPMQCommand::isSubscribe() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kSubscribe; }

bool UPMQCommand::isUnsubscribe() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kUnsubscribe; }

bool UPMQCommand::isDestination() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kDestination; }

bool UPMQCommand::isUndestination() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kUndestination; }

bool UPMQCommand::isBegin() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kBegin; }

bool UPMQCommand::isCommit() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kCommit; }

bool UPMQCommand::isAbort() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kAbort; }

bool UPMQCommand::isMessage() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kMessage; }

bool UPMQCommand::isReceipt() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kReceipt; }

bool UPMQCommand::isBrowserInfo() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kBrowserInfo; }

bool UPMQCommand::isBrowser() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kBrowser; }

bool UPMQCommand::isError() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kError; }

bool UPMQCommand::isSender() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kSender; }

bool UPMQCommand::isUnsender() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kUnsender; }

bool UPMQCommand::isPing() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kPing; }

bool UPMQCommand::isPong() const { return _header->ProtoMessageType_case() == Proto::ProtoMessage::kPong; }

std::string UPMQCommand::typeName() const {
  std::string type = "Unknown";
  if (_header) {
    switch (_header->ProtoMessageType_case()) {
      case Proto::ProtoMessage::kConnect:
        type = "Connect";
        break;
      case Proto::ProtoMessage::kConnected:
        type = "Connected";
        break;
      case Proto::ProtoMessage::kDisconnect:
        type = "Disconnect";
        break;
      case Proto::ProtoMessage::kClientInfo:
        type = "ClientInfo";
        break;
      case Proto::ProtoMessage::kSession:
        type = "Session";
        break;
      case Proto::ProtoMessage::kUnsession:
        type = "Unsession";
        break;
      case Proto::ProtoMessage::kSubscription:
        type = "Subscription";
        break;
      case Proto::ProtoMessage::kSubscribe:
        type = "Subscribe";
        break;
      case Proto::ProtoMessage::kUnsubscribe:
        type = "Unsubscribe";
        break;
      case Proto::ProtoMessage::kUnsubscription:
        type = "Unsubscription";
        break;
      case Proto::ProtoMessage::kBegin:
        type = "Begin";
        break;
      case Proto::ProtoMessage::kCommit:
        type = "Commit";
        break;
      case Proto::ProtoMessage::kAbort:
        type = "Abort";
        break;
      case Proto::ProtoMessage::kAck:
        type = "Ack";
        break;
      case Proto::ProtoMessage::kMessage:
        type = "Message";
        break;
      case Proto::ProtoMessage::kReceipt:
        type = "Receipt";
        break;
      case Proto::ProtoMessage::kError:
        type = "Error";
        break;
      case Proto::ProtoMessage::kBrowser:
        type = "Browser";
        break;
      case Proto::ProtoMessage::kBrowserInfo:
        type = "BrowserInfo";
        break;
      case Proto::ProtoMessage::kSender:
        type = "Sender";
        break;
      case Proto::ProtoMessage::kUnsender:
        type = "Unsender";
        break;
      case Proto::ProtoMessage::kPing:
        type = "Ping";
        break;
      case Proto::ProtoMessage::kPong:
        type = "Pong";
        break;
      case Proto::ProtoMessage::kDestination:
        type = "Destination";
        break;
      case Proto::ProtoMessage::kUndestination:
        type = "Undestination";
        break;
      case Proto::ProtoMessage::PROTOMESSAGETYPE_NOT_SET:
        type = "PROTOMESSAGETYPE_NOT_SET";
        break;
      default:;
    }
  }
  return type;
}

Command *UPMQCommand::duplicate() { return new UPMQCommand(*this); }

string UPMQCommand::getCurrId() {
  switch (this->_header->ProtoMessageType_case()) {
    case Proto::ProtoMessage::kConnect:
      return this->getConnect().client_id();
    case Proto::ProtoMessage::kSession:
      return this->getSession().session_id();
    case Proto::ProtoMessage::kUnsession:
      return this->getUnsession().session_id();
    case Proto::ProtoMessage::kSender:
      return this->getSender().sender_id();
    case Proto::ProtoMessage::kUnsender:
      return this->getUnsender().sender_id();
    case Proto::ProtoMessage::kSubscription:
      return this->getSubscription().destination_uri() + this->getSubscription().subscription_name();
    case Proto::ProtoMessage::kUnsubscription:
      return this->getUnsubscription().destination_uri() + this->getUnsubscription().subscription_name();
    case Proto::ProtoMessage::kSubscribe:
      return this->getSubscribe().destination_uri() + this->getSubscribe().subscription_name();
    case Proto::ProtoMessage::kUnsubscribe:
      return this->getUnsubscribe().destination_uri() + this->getUnsubscribe().subscription_name();
    case Proto::ProtoMessage::kDestination:
      return this->getDestination().destination_uri();
    case Proto::ProtoMessage::PROTOMESSAGETYPE_NOT_SET:
      break;
    default:
      break;
  }
  return "";
}

string UPMQCommand::getParentId() {
  switch (this->_header->ProtoMessageType_case()) {
    case Proto::ProtoMessage::kConnect:
      return this->getConnect().client_id();
    case Proto::ProtoMessage::kSession:
      return this->getSession().connection_id();
    // case Proto::ProtoMessage::kUnsession:
    // return this->getUnsession().connection_id();
    case Proto::ProtoMessage::kSender:
      return this->getSender().session_id();
    case Proto::ProtoMessage::kUnsender:
      return this->getUnsender().session_id();
    case Proto::ProtoMessage::kSubscription:
      return this->getSubscription().session_id();
    case Proto::ProtoMessage::kUnsubscription:
      return this->getUnsubscription().session_id();
    case Proto::ProtoMessage::kSubscribe:
      return this->getSubscribe().session_id();
    case Proto::ProtoMessage::kUnsubscribe:
      return this->getUnsubscribe().session_id();
    case Proto::ProtoMessage::kDestination:
      return this->getDestination().destination_uri();
    case Proto::ProtoMessage::PROTOMESSAGETYPE_NOT_SET:
      break;
    default:
      break;
  }
  return "";
}

void UPMQCommand::setCommandId(int id) { this->commandId = id; }

int UPMQCommand::getCommandId() const { return commandId; }

void UPMQCommand::setResponseRequired(const bool required) { this->responseRequired = required; }

bool UPMQCommand::isResponseRequired() const { return responseRequired; }

UPMQCommand::~UPMQCommand() {
  delete _header;
  _header = nullptr;

  if (_body != nullptr) {
    delete _body;
    _body = nullptr;
  }

  if (_bodyBuff != nullptr) {
    delete[] _bodyBuff;
    _bodyBuff = nullptr;
  }
}

Proto::Connect &UPMQCommand::getConnect() const { return *_header->mutable_connect(); }

Proto::Disconnect &UPMQCommand::getDisconnect() const { return *_header->mutable_disconnect(); }

Proto::ClientInfo &UPMQCommand::getClientInfo() const { return *_header->mutable_client_info(); }

Proto::Session &UPMQCommand::getSession() const { return *_header->mutable_session(); }

Proto::Unsession &UPMQCommand::getUnsession() const { return *_header->mutable_unsession(); }

Proto::Commit &UPMQCommand::getCommit() const { return *_header->mutable_commit(); }

Proto::Abort &UPMQCommand::getAbort() const { return *_header->mutable_abort(); }

Proto::Message &UPMQCommand::getMessage() const { return *_header->mutable_message(); }

int UPMQCommand::getCorrelationId() const { return correlationId; }

void UPMQCommand::setCorrelationId(int newCorrelationId) { this->correlationId = newCorrelationId; }

Proto::ProtoMessage &UPMQCommand::getProtoMessage() const { return *_header; }

Proto::Subscription &UPMQCommand::getSubscription() const { return *_header->mutable_subscription(); }

Proto::Unsubscription &UPMQCommand::getUnsubscription() const { return *_header->mutable_unsubscription(); }

Proto::Subscribe &UPMQCommand::getSubscribe() const { return *_header->mutable_subscribe(); }

Proto::Unsubscribe &UPMQCommand::getUnsubscribe() const { return *_header->mutable_unsubscribe(); }

Proto::Browser &UPMQCommand::getBrowser() const { return *_header->mutable_browser(); }

Proto::Ack &UPMQCommand::getAck() const { return *_header->mutable_ack(); }

Proto::Sender &UPMQCommand::getSender() const { return *_header->mutable_sender(); }

Proto::Unsender &UPMQCommand::getUnsender() const { return *_header->mutable_unsender(); }

Proto::Ping &UPMQCommand::getPing() const { return *_header->mutable_ping(); }

Proto::Destination &UPMQCommand::getDestination() const { return *_header->mutable_destination(); }

Proto::Undestination &UPMQCommand::getUndestination() const { return *_header->mutable_undestination(); }

void UPMQCommand::deserialize(void *array1, int size1, void *array2, int size2) {
  DECAF_UNUSED_VAR(array2);
  DECAF_UNUSED_VAR(size2);
  deserializeHeader(array1, size1);
}

void UPMQCommand::deserializeHeader(void *array, int size) {
  if (!_header->ParseFromArray(array, size)) {
    _header->Clear();
    // TODO throw ex ?
  }
}

void UPMQCommand::deserializeBody(void *array, int size) {
  if (!_body->ParseFromArray(array, size)) {
    _body->Clear();
  }
}

std::vector<char> UPMQCommand::serialize() const {
  std::vector<char> result;

  std::string header = serializeHeader();
  std::string body = serializeBody();
  int headerSize = static_cast<int>(header.size());
  long long bodySize = body.size();

  long long allSize = headerSize + bodySize + sizeof(int) + sizeof(long long);
  result.resize(static_cast<std::vector<char>::size_type>(allSize));
  memcpy(&(result)[0], &headerSize, sizeof(int));
  memcpy(&(result)[sizeof(int)], &bodySize, sizeof(long long));
  memcpy(&(result)[sizeof(int) + sizeof(long long)], header.c_str(), static_cast<size_t>(headerSize));
  if (!body.empty() && bodySize > 0) {
    memcpy(&(result)[sizeof(int) + sizeof(long long) + static_cast<size_t>(headerSize)], body.c_str(), static_cast<size_t>(bodySize));
  }

  return result;
}
void UPMQCommand::serializeToOstream(std::ostream &outStream) const {
  uint32_t hSize = 0;
  if (_header && _header->ByteSizeLong()) {
    hSize = static_cast<uint32_t>(_header->ByteSizeLong());
  }
  unsigned long long bSize = 0;
  if (_body && _body->ByteSizeLong()) {
    bSize = _body->ByteSizeLong();
  }
  outStream.write((char *)&hSize, sizeof(hSize));
  outStream.write((char *)&bSize, sizeof(bSize));
  serializeHeaderToOstream(outStream);
  serializeBodyToOstream(outStream);
}
string UPMQCommand::serializeHeader() const {
  if (_header) {
    _headerSize = static_cast<int>(_header->ByteSizeLong());
    if (_headerSize) {
      _headerString = _header->SerializeAsString();
    } else {
      _headerString.clear();
    }
    return _headerString;
  }
  return emptyString;
}
void UPMQCommand::serializeHeaderToOstream(std::ostream &outStream) const {
  if (_header && _header->ByteSizeLong()) {
    _header->SerializeToOstream(&outStream);
    _header->SerializeToString(&_headerString);
  }
}
string UPMQCommand::serializeBody() const {
  if (!_body && _bodyBuff && (_bodyBuffSize > 0)) {
    return std::string(reinterpret_cast<char *>(_bodyBuff), static_cast<size_t>(_bodyBuffSize));
  }
  if (_body) {
    _bodySize = static_cast<int>(_body->ByteSizeLong());  // FIXME: body size type must be uint64_t
    if (_bodySize) {
      _bodyString = _body->SerializeAsString();
    } else {
      _bodyString.clear();
    }
    return _bodyString;
  }
  return emptyString;
}
void UPMQCommand::serializeBodyToOstream(std::ostream &outStream) const {
  if (_body && _body->ByteSizeLong()) {
    _body->SerializeToOstream(&outStream);
  }
}
int UPMQCommand::getHeaderSize() const { return _headerSize; }

int UPMQCommand::getBodySize() const { return _bodySize; }

const string &UPMQCommand::getHeader() const {
  if (_header && !_headerString.empty()) {
    return _headerString;
  }
  return emptyString;
}

const string &UPMQCommand::getBody() const {
  if (_body && !_bodyString.empty()) {
    return _bodyString;
  }
  return emptyString;
}

std::string UPMQCommand::toString() const { return _header->ShortDebugString(); }

string UPMQCommand::toJsonString() const {
  std::string result = "{\"header\":";
  if (_header) {
    google::protobuf::util::MessageToJsonString(*_header, &result);
  }
  if (_body) {
    std::string bodys;
    google::protobuf::util::MessageToJsonString(*_body, &bodys);
    if (!bodys.empty()) {
      result.append(",\"body\":").append(bodys);
    }
  }
  result.append("}");
  return result;
}

string UPMQCommand::toPrettyJsonString() const {
  google::protobuf::util::JsonPrintOptions jsonPrintOptions;
  jsonPrintOptions.add_whitespace = true;
  std::string result = "{\n\"header\" : ";
  if (_header) {
    google::protobuf::util::MessageToJsonString(*_header, &result, jsonPrintOptions);
  }
  if (_body) {
    std::string bodys;
    google::protobuf::util::MessageToJsonString(*_body, &bodys, jsonPrintOptions);
    if (!bodys.empty()) {
      result.append(",\n\"body\" : ").append(bodys);
    }
  }
  result.append("\n}");
  return result;
}

bool UPMQCommand::isResponse() const { return _response; }

void UPMQCommand::setResponse(bool value) { _response = value; }

void UPMQCommand::processCommand() {
  if (_header->ProtoMessageType_case() == Proto::ProtoMessage::ProtoMessageTypeCase::kError) {
    string errorCode = "ERROR CODE: " + to_string(_header->error().error_code());
    string errorMessage = " ERROR MESSAGE: " + _header->error().error_message();

    switch (_header->error().error_code()) {
      case Proto::ErrorCode::ERROR_INVALID_SELECTOR:
        throw cms::InvalidSelectorException(errorCode + errorMessage);
      case Proto::ErrorCode::ERROR_CLIENT_ID_EXISTS:
        throw cms::InvalidClientIdException(errorCode + errorMessage);
      case Proto::ErrorCode::ERROR_DESTINATION:
        throw cms::InvalidDestinationException(errorCode + errorMessage);
      default:
        throw cms::CMSException(errorCode + errorMessage);
    }
  }
}

Proto::Connected UPMQCommand::processConnected() {
  try {
    processCommand();
    if (_header->ProtoMessageType_case() != Proto::ProtoMessage::kConnected) {
      throw cms::CMSException("Connected command type error");
    } else {
      return _header->connected();
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

Proto::BrowserInfo UPMQCommand::processBrowserInfo() {
  try {
    processCommand();
    if (_header->ProtoMessageType_case() != Proto::ProtoMessage::kBrowserInfo) {
      throw cms::CMSException("BrowserInfo command type error");
    } else {
      return _header->browser_info();
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

void UPMQCommand::processReceipt() {
  try {
    processCommand();
    if (_header->ProtoMessageType_case() != Proto::ProtoMessage::kReceipt) {
      throw cms::CMSException("Receipt command type error");
    }
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

const string &UPMQCommand::getObjectId() { return _header->object_id(); }
