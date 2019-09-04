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

#include "UPMQWireFormat.h"
#include <transport/UPMQCommand.h>
#include "libupmq/MessageFactoryImpl.h"

using namespace std;
using namespace upmq;
using namespace upmq::transport;
using namespace decaf::io;
using namespace decaf::util;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

using namespace upmq::transport;

////////////////////////////////////////////////////////////////////////////////
UPMQWireFormat::UPMQWireFormat() {}

////////////////////////////////////////////////////////////////////////////////
UPMQWireFormat::~UPMQWireFormat() {}

////////////////////////////////////////////////////////////////////////////////
void UPMQWireFormat::marshal(const Pointer<transport::Command> command,
                             const upmq::transport::Transport *transport,
                             decaf::io::DataOutputStream *dataOut) {
  if (transport == nullptr) {
    throw decaf::io::IOException(__FILE__, __LINE__, "Transport passed is NULL");
  }

  if (dataOut == nullptr) {
    throw decaf::io::IOException(__FILE__, __LINE__, "DataOutputStream passed is NULL");
  }

  UPMQCommand *upmq = dynamic_cast<UPMQCommand *>(command.get());
  if (upmq != nullptr) {
    if (upmq->isResponseRequired()) {
      upmq->getProtoMessage().set_request_reply_id(command->getCommandId());
    } else {
      upmq->getProtoMessage().set_request_reply_id(0);
    }

    auto result = upmq->serialize();
    dataOut->write(reinterpret_cast<unsigned char *>(&result[0]), static_cast<int>(result.size()));
  }
}

////////////////////////////////////////////////////////////////////////////////
Pointer<transport::Command> UPMQWireFormat::unmarshal(const upmq::transport::Transport *transport UPMQCPP_UNUSED, decaf::io::DataInputStream *dis) {
  DECAF_UNUSED_VAR(transport);
  unsigned char sizeBuf[sizeof(int) + sizeof(long long)] = {0};

  dis->readFully(&sizeBuf[0], sizeof(sizeBuf));

  int headerSize = *reinterpret_cast<int *>(&sizeBuf[0]);
  long long bodySize = *reinterpret_cast<long long *>(&sizeBuf[sizeof(int)]);

  std::vector<unsigned char> headerBuff(headerSize);

  dis->readFully(&headerBuff[0], headerSize);

  unsigned char *bodyBuff = nullptr;
  if (bodySize) {
    bodyBuff = new unsigned char[size_t(bodySize)]();
    dis->readFully(&bodyBuff[0], int(bodySize));
  }

  Proto::ProtoMessage *header = new Proto::ProtoMessage();
  if (!header->ParseFromArray(&headerBuff[0], headerSize)) {
    header->Clear();
    // TODO throw ex;
  }

  Pointer<UPMQCommand> command(MessageFactoryImpl::getProperMessage(header, bodyBuff, bodySize));

  if (command->getProtoMessage().request_reply_id() == 0) {
    command->setResponse(false);
  } else {
    command->setResponse(true);
    command->setCorrelationId(command->getProtoMessage().request_reply_id());
  }

  return command.dynamicCast<transport::Command>();
}
