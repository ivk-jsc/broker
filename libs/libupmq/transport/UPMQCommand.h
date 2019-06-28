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

#pragma once

#include <libupmq/ProtoHeader.h>
#include <transport/Response.h>

using namespace upmq;
using namespace upmq::transport;
using namespace std;

class UPMQCommand : public Response {
 public:
  UPMQCommand();
  UPMQCommand(const UPMQCommand &o);
  UPMQCommand(UPMQCommand &&) = delete;
  UPMQCommand &operator=(const UPMQCommand &) = delete;
  UPMQCommand &operator=(UPMQCommand &&) = delete;
  ~UPMQCommand() override;

  UPMQCommand(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size);

  std::string getCurrId() override;
  std::string getParentId() override;

  bool isConnect() const override;
  bool isConnected() const override;
  bool isDisconnect() const override;
  bool isClientInfo() const override;
  bool isSession() const override;
  bool isUnsession() const override;
  bool isSubscription() const override;
  bool isUnsubscription() const override;
  bool isDestination() const override;
  bool isUndestination() const override;
  bool isSubscribe() const override;
  bool isUnsubscribe() const override;
  bool isSender() const override;
  bool isUnsender() const override;
  bool isBegin() const override;
  bool isCommit() const override;
  bool isAbort() const override;
  bool isAck() const override;
  bool isMessage() const override;
  bool isReceipt() const override;
  bool isBrowser() const override;
  bool isBrowserInfo() const override;
  bool isError() const override;
  bool isPing() const override;
  bool isPong() const override;

  Command *duplicate() override;

  int commandId;
  int correlationId;
  bool responseRequired;

  void setCommandId(int id) override;
  int getCommandId() const override;

  int getCorrelationId() const override;
  void setCorrelationId(int newCorrelationId) override;

  void setResponseRequired(const bool required) override;
  bool isResponseRequired() const override;

  Proto::ProtoMessage &getProtoMessage() const;
  Proto::Subscription &getSubscription() const;
  Proto::Unsubscription &getUnsubscription() const;
  Proto::Subscribe &getSubscribe() const;
  Proto::Unsubscribe &getUnsubscribe() const;
  Proto::Browser &getBrowser() const;
  Proto::Connect &getConnect() const;
  Proto::Disconnect &getDisconnect() const;
  Proto::ClientInfo &getClientInfo() const;
  Proto::Session &getSession() const;
  Proto::Unsession &getUnsession() const;
  Proto::Commit &getCommit() const;
  Proto::Abort &getAbort() const;
  Proto::Ack &getAck() const;
  Proto::Sender &getSender() const;
  Proto::Unsender &getUnsender() const;
  Proto::Ping &getPing() const;
  Proto::Destination &getDestination() const;
  Proto::Undestination &getUndestination() const;
  Proto::Message &getMessage() const;

  void deserialize(void *array1, int size1, void *array2, int size2);
  void deserializeHeader(void *array, int size);
  void deserializeBody(void *array, int size);
  std::vector<char> serialize() const;
  void serializeToOstream(std::ostream &outStream) const;
  string serializeHeader() const;
  void serializeHeaderToOstream(std::ostream &outStream) const;
  string serializeBody() const;
  void serializeBodyToOstream(std::ostream &outStream) const;
  int getHeaderSize() const;
  int getBodySize() const;
  const string &getHeader() const;
  const string &getBody() const;
  string toString() const override;
  string toJsonString() const;
  string toPrettyJsonString() const;

  bool isResponse() const override;
  void setResponse(bool value);

  void processCommand();
  void processReceipt() override;
  Proto::Connected processConnected() override;
  Proto::BrowserInfo processBrowserInfo() override;

  const string &getObjectId();

  Proto::ProtoMessage *_header;
  Proto::Body *_body;

  // for income message
  unsigned char *_bodyBuff;
  long long _bodyBuffSize;

  // for outcome message
  mutable string _headerString;
  mutable int _headerSize;
  mutable string _bodyString;
  mutable int _bodySize;

  bool _response;

  void *_consumer;
};
