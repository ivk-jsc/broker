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

#ifndef BROKER_ASYNCHANDLER_H
#define BROKER_ASYNCHANDLER_H

#include <Poco/AutoPtr.h>
#include <Poco/Delegate.h>
#include <Poco/FIFOEvent.h>
#include <Poco/NObserver.h>
#include <Poco/Net/NetException.h>
#include "SocketNotifier.h"
#include "SocketReactor.h"
#include "SocketNotification.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/RWLock.h>
#include <Poco/Thread.h>
#include <Poco/UUID.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/ServerApplication.h>

#include <atomic>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "AsyncLogger.h"
#include "MessageDataContainer.h"
#include "ConcurrentQueueHeader.h"
#include <Poco/Logger.h>
#ifdef __APPLE__
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
#endif

#ifdef _WIN32
#define MSG_NOSIGNAL 0
#endif

namespace upmq {
namespace broker {

class Connection;

class AsyncTCPHandler {
 public:
  // SubscriptionsList - set<destination_id, set<subscription_id> >
  using SubscriptionsList = std::unordered_map<std::string, std::unordered_set<std::string> >;
  struct HeartBeat {
    int sendTimeout;
    int recvTimeout;
    std::string toString() const { return "heartbeat (" + std::to_string(sendTimeout) + ":" + std::to_string(recvTimeout) + ")"; }
  };
  struct ClientVersion {
    int majorVersion;
    int minorVersion;
    int revisionVersion;
    std::string vendorId;
    std::string toString() const {
      return "vendor (" + vendorId + ") version (" + std::to_string(majorVersion) + "." + std::to_string(minorVersion) + "." +
             std::to_string(revisionVersion) + ")";
    }
  };
  struct ProtocolVersion {
    int major;
    int minor;
    std::string toString() const { return "protocol (" + std::to_string(major) + "." + std::to_string(minor) + ")"; }
  };

  enum class DataStatus { AS_ERROR, TRYAGAIN, OK };

  AsyncTCPHandler(Poco::Net::StreamSocket &socket, upmq::Net::SocketReactor &reactor);
  void removeErrorShutdownHandler();
  void removeConsumers();
  virtual ~AsyncTCPHandler();
  void onReadable(const AutoPtr<upmq::Net::ReadableNotification> &pNf);
  void onShutdown(const AutoPtr<upmq::Net::ShutdownNotification> &pNf);
  void onError(const AutoPtr<upmq::Net::ErrorNotification> &pNf);
  void setClientID(const std::string &clientID);
  const std::string &clientID() const;
  void setConnection(Connection *connection) const;
  Connection *connection() const;
  bool needErase() const;
  void setNeedErase();

  const std::string &peerAddress() const;

  void put(std::shared_ptr<MessageDataContainer> sMessage);

  std::string toString() const;

  int maxNotAcknowledgedMessages() const;
  HeartBeat heartbeat{};
  ClientVersion clientVersion;
  ProtocolVersion protocolVersion{};

  void emitCloseEvent(bool withError = false);

  DataStatus sendHeaderAndData(MessageDataContainer &sMessage);

  size_t queueReadNum() const;
  size_t queueWriteNum() const;

 private:
  enum { BUFFER_SIZE = 65536 };

  Poco::Net::StreamSocket _socket;
  upmq::Net::SocketReactor &_reactor;
  std::string _peerAddress;
  std::atomic_bool _allowPutEvent{true};

 public:
  void allowPutReadEvent();

 public:
  moodycamel::ConcurrentQueue<std::shared_ptr<MessageDataContainer> > outputQueue;
  Poco::FastMutex onWritableLock;
  Poco::FastMutex onReadableLock;
  char pBuffer[BUFFER_SIZE]{};
  char hbLens[sizeof(uint32_t) + sizeof(uint64_t)]{};
  struct HeaderBodyLens {
    uint32_t headerLen = 0;
    uint64_t bodyLen = 0;
  };
  HeaderBodyLens headerBodyLens;

  Poco::NObserver<AsyncTCPHandler, upmq::Net::ReadableNotification> _readableCallBack;
  Poco::NObserver<AsyncTCPHandler, upmq::Net::ErrorNotification> _errorCallBack;
  Poco::NObserver<AsyncTCPHandler, upmq::Net::ShutdownNotification> _shutdownCallBack;

  size_t num{};

 private:
  bool _wasError;
  std::atomic_bool _needErase;
  size_t _queueReadNum = 0;
  size_t _queueWriteNum = 0;

  int _maxNotAcknowledgedMessages;
  std::string _clientID;
  mutable SubscriptionsList _subscriptions;
  mutable Connection *_connection;
  mutable Poco::FastMutex _closeEventLock;
  std::atomic_bool _readComplete;

 public:
  mutable Poco::Logger *log{nullptr};
  void storeClientInfo(const MessageDataContainer &sMessage);
  void initSubscription(const MessageDataContainer &sMessage) const;
  void eraseSubscription(const MessageDataContainer &sMessage) const;
  AsyncTCPHandler::DataStatus fillHeaderBodyLens();
  AsyncTCPHandler::DataStatus fillHeader(MessageDataContainer &sMessage);
  AsyncTCPHandler::DataStatus fillBody(MessageDataContainer &sMessage);
  AsyncTCPHandler::DataStatus tryMoveBodyByLink(MessageDataContainer &sMessage);
  void setReadComplete(bool readComplete);
  bool readComplete() const;
};
}  // namespace broker
}  // namespace upmq
#endif  // BROKER_ASYNCHANDLER_H
