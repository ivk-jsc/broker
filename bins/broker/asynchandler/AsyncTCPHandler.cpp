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
#include <Poco/File.h>
#include <S2SProto.h>
#include <memory>
#include <fake_cpp14.h>
#include "AsyncHandlerRegestry.h"
#include "Broker.h"
#include "Exception.h"
#include "MainApplication.h"
#include "Connection.h"
#include "Poco/Error.h"
#include <cerrno>

#ifdef ENABLE_USING_SENDFILE
#include "sendfile/SendFile.h"
#include "AsyncTCPHandler.h"

#endif

#ifdef _WIN32
using send_size_t = int;
#else
using send_size_t = size_t;
#endif  // _WIN32

namespace upmq {
namespace broker {

AsyncTCPHandler::AsyncTCPHandler(Poco::Net::StreamSocket &socket, upmq::Net::SocketReactor &reactor)
    : _socket(socket),
      _reactor(reactor),
      _peerAddress(socket.peerAddress().toString()),
      _readableCallBack(*this, &AsyncTCPHandler::onReadable),
      _errorCallBack(*this, &AsyncTCPHandler::onError),
      _shutdownCallBack(*this, &AsyncTCPHandler::onShutdown),
      _wasError(false),
      _needErase(false),
      _maxNotAcknowledgedMessages(0),
      _connection(nullptr),
      _readComplete(true) {
  AHRegestry::Instance().addAHandler(this);

  const size_t queueNum = AHRegestry::Instance()._connectionCounter++;

  _queueReadNum = queueNum % THREADS_CONFIG.readers;
  _queueWriteNum = queueNum % THREADS_CONFIG.writers;

  log = &Poco::Logger::get(CONFIGURATION::Instance().log().name);

  _socket.setNoDelay(true);
  _socket.setBlocking(false);

  log->information("%s",
                   std::to_string(num)
                       .append(" * => new asynchandler from ")
                       .append(_peerAddress)
                       .append(" q-num : ")
                       .append(std::to_string(queueNum))
                       .append(" ( ")
                       .append(std::to_string(AHRegestry::Instance().size()))
                       .append(" asynchandlers now )"));

  _reactor.addEventHandler(_socket, _readableCallBack);
  _reactor.addEventHandler(_socket, _shutdownCallBack);
  _reactor.addEventHandler(_socket, _errorCallBack);
}

void AsyncTCPHandler::removeErrorShutdownHandler() {
  if (_closeEventLock.tryLock(2000)) {
    _reactor.removeEventHandler(_socket, _shutdownCallBack);
    _reactor.removeEventHandler(_socket, _errorCallBack);
    _closeEventLock.unlock();
  }
}

void AsyncTCPHandler::removeConsumers() {
  for (const auto &dest : _subscriptions) {
    for (const auto &subs : dest.second) {
      try {
        BROKER::Instance().removeConsumers(dest.first, subs, num);
      } catch (...) {  // -V565
      }
    }
  }
}

AsyncTCPHandler::~AsyncTCPHandler() {
  try {
    while (!_readComplete) {
      Poco::Thread::sleep(100);
    }

    _reactor.removeEventHandler(_socket, _readableCallBack);

    AHRegestry::Instance().deleteAHandler(num);

    removeConsumers();

    removeErrorShutdownHandler();

    BROKER::Instance().removeTcpConnection(_clientID, num);
    EXCHANGE::Instance().dropOwnedDestination(_clientID);

  } catch (std::exception &ex) {
    log->critical("%s", std::to_string(num).append(" ! => ").append(std::string(ex.what())));
  }

  log->information("%s", std::to_string(num).append(" * => destruct asynchandler from ").append(_peerAddress));
}

void AsyncTCPHandler::onReadable(const AutoPtr<upmq::Net::ReadableNotification> &pNf) {
  if (_needErase && !_readComplete) {
    _readComplete = true;
    return;
  }
  if (_needErase && _readComplete) {
    Poco::Thread::sleep(100);
    return;
  }
  UNUSED_VAR(pNf);
  if (_allowPutEvent) {
    _allowPutEvent = false;
    BROKER::Instance().putReadable(_queueReadNum, num);
  }
  if (!_needErase) {
    _readComplete = false;
  }
}
void AsyncTCPHandler::put(std::shared_ptr<MessageDataContainer> sMessage) {
  do {
    if (_needErase) {
      return;
    }
  } while (!outputQueue.enqueue(sMessage));
  BROKER::Instance().putWritable(_queueWriteNum, num);
}

void AsyncTCPHandler::onShutdown(const AutoPtr<upmq::Net::ShutdownNotification> &pNf) {
  UNUSED_VAR(pNf);
  log->notice("%s", std::to_string(num).append(" ! => shutdown : ").append(_peerAddress));
  emitCloseEvent();
}

void AsyncTCPHandler::onError(const AutoPtr<upmq::Net::ErrorNotification> &pNf) {
  UNUSED_VAR(pNf);
  log->error("%s", std::to_string(num).append(" ! => network error : ").append(_peerAddress));
  emitCloseEvent(true);
}

AsyncTCPHandler::DataStatus AsyncTCPHandler::sendHeaderAndData(MessageDataContainer &sMessage) {
  uint32_t headerSize = static_cast<uint32_t>(sMessage.header.size());
  uint64_t dataSize = sMessage.dataSize();

  std::string sSize;
  sSize.append((char *)&headerSize, sizeof(headerSize));
  sSize.append((char *)&dataSize, sizeof(dataSize));

  sSize.append(sMessage.header);

  if (!sMessage.withFile()) {
    sSize.append(sMessage.data);
  }
  ptrdiff_t n = 0;
  ptrdiff_t sent = 0;
  send_size_t allSize = static_cast<send_size_t>(sSize.size());

  do {
    send_size_t tmpDataSize = (allSize < static_cast<send_size_t>(BUFFER_SIZE)) ? allSize : static_cast<send_size_t>(BUFFER_SIZE);
    errno = 0;
    do {
      n = ::send(_socket.impl()->sockfd(), &sSize.c_str()[sent], tmpDataSize, MSG_NOSIGNAL);
    } while (n < 0 && Poco::Error::last() == POCO_EINTR);
    int error = Poco::Error::last();
    if ((error == POCO_EAGAIN) || (error == POCO_EWOULDBLOCK)) {
      if (sent == 0) {
        return DataStatus::TRYAGAIN;
      }
      Poco::Thread::yield();
      continue;
    }
    if (n < 0) {
      return DataStatus::AS_ERROR;
    }
    sent += n;
    allSize -= static_cast<send_size_t>(n);
  } while (allSize > 0);

  if (sMessage.withFile() && dataSize != 0) {
#ifdef ENABLE_USING_SENDFILE
    SendFile sendfile(_socket, sMessage.fileStream(), BUFFER_SIZE);
    if (!sendfile()) {
      return DataStatus::AS_ERROR;
    }
#else  // !ENABLE_USING_SENDFILE
    sent = 0;
    do {
      int tmpDataSize = ((dataSize - sent) < BUFFER_SIZE) ? static_cast<int>(dataSize - sent) : BUFFER_SIZE;
      const std::vector<char> partOfData = sMessage.getPartOfData(static_cast<size_t>(sent), static_cast<size_t>(tmpDataSize));
      tmpDataSize = partOfData.size();  // should be the same, but just in case
      size_t tmpOffs = 0;
      do {
        do {
          n = ::send(_socket.impl()->sockfd(), &partOfData[static_cast<size_t>(tmpOffs)], tmpDataSize, MSG_NOSIGNAL);
        } while (n < 0 && Poco::Error::last() == POCO_EINTR);
        int error = Poco::Error::last();
        if ((error == POCO_EAGAIN) || (error == POCO_EWOULDBLOCK)) {
          n = 0;
          Poco::Thread::yield();
          continue;
        } else if (n < 0) {
          throw EXCEPTION("sendHeaderAndData", Poco::Error::getMessage(error), error);
        }
        tmpOffs += static_cast<size_t>(n);
        sent += static_cast<uint64_t>(n);
        tmpDataSize -= n;
      } while (tmpDataSize > 0);
    } while (dataSize > sent);
#endif  // !ENABLE_USING_SENDFILE
  }
  return DataStatus::OK;
}

void AsyncTCPHandler::emitCloseEvent(bool withError) {
  if (_needErase) {
    return;
  }
  if (_closeEventLock.tryLock()) {
    try {
      if (_needErase) {
        _closeEventLock.unlock();
        return;
      }
      if (AHRegestry::Instance().aHandler(num) != nullptr) {
        setNeedErase();
        _wasError = withError;
        _closeEventLock.unlock();
        return;
      }
    } catch (...) {  // -V565
    }
    _closeEventLock.unlock();
  }
}
const std::string &AsyncTCPHandler::peerAddress() const { return _peerAddress; }
std::string AsyncTCPHandler::toString() const {
  std::stringstream out;
  out << "tcp connection id : " << num << " : "
      << "client id : " << _clientID << " : " << clientVersion.toString() << " : " << heartbeat.toString() << " : " << protocolVersion.toString()
      << " max_not_acknowledged_messages = " << std::to_string(_maxNotAcknowledgedMessages);
  return out.str();
}
void AsyncTCPHandler::setClientID(const std::string &clientID) { _clientID = clientID; }
const std::string &AsyncTCPHandler::clientID() const { return _clientID; }
void AsyncTCPHandler::setConnection(Connection *connection) const { _connection = connection; }
Connection *AsyncTCPHandler::connection() const { return _connection; }
int AsyncTCPHandler::maxNotAcknowledgedMessages() const { return _maxNotAcknowledgedMessages; }
void AsyncTCPHandler::storeClientInfo(const MessageDataContainer &sMessage) {
  const Proto::Connect &connect = sMessage.connect();
  const Proto::ClientVersion &version = connect.client_version();
  clientVersion.vendorId = version.vendor_id();
  clientVersion.majorVersion = version.client_major_version();
  clientVersion.minorVersion = version.client_minor_version();
  clientVersion.revisionVersion = version.client_revision_version();

  const Proto::ProtocolVersion &protoVersion = connect.protocol_version();
  protocolVersion.major = protoVersion.protocol_major_version();
  protocolVersion.minor = protoVersion.protocol_minor_version();

  const Proto::Heartbeat &hb = connect.heartbeat();
  heartbeat.sendTimeout = hb.send_timeout();
  heartbeat.recvTimeout = hb.recv_timeout();

  _maxNotAcknowledgedMessages = connect.max_not_acknowledged_messages();

  setClientID(connect.client_id());
}
void AsyncTCPHandler::initSubscription(const MessageDataContainer &sMessage) const {
  const Proto::Subscription &subscription = sMessage.subscription();
  std::string destinationID = upmq::broker::Exchange::mainDestinationPath(subscription.destination_uri());
  auto subs = _subscriptions.find(destinationID);
  if (subs == _subscriptions.end()) {
    _subscriptions.insert(std::make_pair(destinationID, std::unordered_set<std::string>()));
    subs = _subscriptions.find(destinationID);
  }
  subs->second.insert(subscription.subscription_name());
}
void AsyncTCPHandler::eraseSubscription(const MessageDataContainer &sMessage) const {
  const Proto::Unsubscription &unsubscription = sMessage.unsubscription();
  std::string destinationID = upmq::broker::Exchange::mainDestinationPath(unsubscription.destination_uri());
  auto subs = _subscriptions.find(destinationID);
  if (subs != _subscriptions.end()) {
    subs->second.erase(unsubscription.subscription_name());
  }
}
bool AsyncTCPHandler::needErase() const { return _needErase; }
void AsyncTCPHandler::setNeedErase() {
  _needErase = true;
  AHRegestry::Instance().needToErase(num);
}
AsyncTCPHandler::DataStatus AsyncTCPHandler::fillHeaderBodyLens() {
  headerBodyLens.headerLen = 0;
  headerBodyLens.bodyLen = 0;

  ptrdiff_t n = 0;
  send_size_t tmpDtSize = 0;

  do {
    errno = 0;
    do {
      n = ::recv(_socket.impl()->sockfd(), &hbLens[tmpDtSize], static_cast<send_size_t>(sizeof(hbLens)) - tmpDtSize, MSG_NOSIGNAL);  // -V781
    } while (n < 0 && Poco::Error::last() == POCO_EINTR);
    if ((n < 0) || (n == 0 && (tmpDtSize != sizeof(hbLens)))) {
      int error = Poco::Error::last();
      if ((error == POCO_EWOULDBLOCK) || (error == POCO_EAGAIN)) {
        if (tmpDtSize == 0) {
          return DataStatus::TRYAGAIN;
        }
        Poco::Thread::yield();
        continue;
      }
      return DataStatus::AS_ERROR;
    }
    if (n == 0) {
      break;
    }
    tmpDtSize += static_cast<send_size_t>(n);
  } while (tmpDtSize != sizeof(hbLens));

  headerBodyLens.headerLen = *reinterpret_cast<uint32_t *>(hbLens);
  headerBodyLens.bodyLen = *reinterpret_cast<uint64_t *>(&hbLens[sizeof(uint32_t)]);

  if (headerBodyLens.headerLen == 0) {
    return DataStatus::AS_ERROR;
  }

  if (headerBodyLens.bodyLen == UINT64_MAX) {
    return DataStatus::AS_ERROR;
  }

  return DataStatus::OK;
}
AsyncTCPHandler::DataStatus AsyncTCPHandler::fillHeader(MessageDataContainer &sMessage) {
  ptrdiff_t n = 0;
  do {
    send_size_t tmpHeaderSize =
        (headerBodyLens.headerLen < INT_MAX) ? static_cast<send_size_t>(headerBodyLens.headerLen) : static_cast<send_size_t>(BUFFER_SIZE);
    errno = 0;
    do {
      n = ::recv(_socket.impl()->sockfd(), pBuffer, std::min<send_size_t>(BUFFER_SIZE, tmpHeaderSize), MSG_NOSIGNAL);
    } while (n < 0 && Poco::Error::last() == POCO_EINTR);
    if (n < 0 || (n == 0 && (headerBodyLens.headerLen > 0))) {
      int error = Poco::Error::last();
      if ((error == POCO_EWOULDBLOCK) || (error == POCO_EAGAIN)) {
        Poco::Thread::yield();
        continue;
      }
      return DataStatus::AS_ERROR;
    }
    if (n == 0) {
      break;
    }
    headerBodyLens.headerLen -= static_cast<uint32_t>(n);
    sMessage.header.append(pBuffer, static_cast<size_t>(n));
  } while (headerBodyLens.headerLen > 0);
  return DataStatus::OK;
}
AsyncTCPHandler::DataStatus AsyncTCPHandler::fillBody(MessageDataContainer &sMessage) {
  ptrdiff_t n = 0;
  sMessage.initPersistentDataFileLink();
  do {
    send_size_t tmpDataSize =
        (headerBodyLens.bodyLen < INT_MAX) ? static_cast<send_size_t>(headerBodyLens.bodyLen) : static_cast<send_size_t>(BUFFER_SIZE);
    errno = 0;
    do {
      n = ::recv(_socket.impl()->sockfd(), pBuffer, std::min<send_size_t>(BUFFER_SIZE, tmpDataSize), MSG_NOSIGNAL);
    } while (n < 0 && Poco::Error::last() == POCO_EINTR);
    if (n < 0 || (n == 0 && (headerBodyLens.bodyLen > 0))) {
      int error = Poco::Error::last();
      if ((error == POCO_EWOULDBLOCK) || (error == POCO_EAGAIN)) {
        Poco::Thread::yield();
        continue;
      }
      return DataStatus::AS_ERROR;
    }
    if (n == 0) {
      break;
    }
    headerBodyLens.bodyLen -= static_cast<uint64_t>(n);
    sMessage.appendData(pBuffer, static_cast<size_t>(n));
  } while (headerBodyLens.bodyLen > 0);
  sMessage.flushData();
  return DataStatus::OK;
}
AsyncTCPHandler::DataStatus AsyncTCPHandler::tryMoveBodyByLink(MessageDataContainer &sMessage) {
  auto &message = sMessage.message();
  auto &property = message.property();
  auto it = property.find(s2s::proto::upmq_data_link);
  if (it != property.end() && !it->second.is_null() && !it->second.value_string().empty()) {
    auto dataSizeItem = property.find(broker::s2s::proto::upmq_data_size);
    if (dataSizeItem != property.end() && !dataSizeItem->second.is_null() && (dataSizeItem->second.value_long() > 0)) {
      Poco::File dataSrcFile(it->second.value_string());
      Poco::Path dataDestPath = STORAGE_CONFIG.data.get();
      std::string msgID = message.message_id();
      msgID[2] = '_';
      dataDestPath.append(Exchange::mainDestinationPath(message.destination_uri())).append(msgID);

      dataDestPath.makeFile();
      Poco::File dataDestDir(dataDestPath.parent());
      if (!dataDestDir.exists()) {
        dataDestDir.createDirectories();
      }
      dataSrcFile.renameTo(dataDestPath.toString());
    }
  }
  return DataStatus::OK;
}
void AsyncTCPHandler::allowPutReadEvent() { _allowPutEvent = true; }
size_t AsyncTCPHandler::queueReadNum() const { return _queueReadNum; }
size_t AsyncTCPHandler::queueWriteNum() const { return _queueWriteNum; }
void AsyncTCPHandler::setReadComplete(bool readComplete) { _readComplete = readComplete; }
bool AsyncTCPHandler::readComplete() const { return _readComplete; }
}  // namespace broker
}  // namespace upmq
