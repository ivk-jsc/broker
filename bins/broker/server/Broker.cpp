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

#include "Broker.h"
#include "AsyncHandlerRegestry.h"
#include "Connection.h"
#include "Consumer.h"
#include "Exception.h"
#include "Exchange.h"
#include "MiscDefines.h"
#include "Poco/Error.h"
#include "Poco/File.h"
#include "S2SProto.h"
#include "Session.h"
#include "Version.hpp"
#include "fake_cpp14.h"

namespace upmq {
namespace broker {

Broker::Broker(std::string id)
    : logStream(new ThreadSafeLogStream(ASYNCLOGGER::Instance().get(LOG_CONFIG.name))),
      _id(std::move(id)),
      _isRunning(false),
      _isReadable(false),
      _isWritable(false),
      _readablePool(_id + "readable", 1, static_cast<int>(THREADS_CONFIG.readers + 1)),
      _writablePool(_id + "writable", 1, static_cast<int>(THREADS_CONFIG.writers + 1)),
      _readableIndexes(THREADS_CONFIG.readers),
      _writableIndexes(THREADS_CONFIG.writers) {
  std::stringstream sql;
  sql << "drop table if exists \"" << _id << "\";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("broker initialization error", sql.str(), ERROR_STORAGE);
  sql.str("");
  sql << "create table if not exists \"" << _id << "\" ("
      << " client_id text not null unique"
      << ",create_time timestamp not null default current_timestamp"
      << ")"
      << ";";
  TRY_POCO_DATA_EXCEPTION { storage::DBMSConnectionPool::doNow(sql.str()); }
  CATCH_POCO_DATA_EXCEPTION_PURE("broker initialization error", sql.str(), ERROR_STORAGE);
}
Broker::~Broker() = default;
const std::string &Broker::id() const { return _id; }
void Broker::onEvent(const AsyncTCPHandler &ahandler, MessageDataContainer &sMessage) {
  ASYNCLOG_INFORMATION(ahandler.logStream, (std::to_string(sMessage.handlerNum).append(" # => ").append(sMessage.typeName())));
  std::shared_ptr<MessageDataContainer> outMessage = std::make_shared<MessageDataContainer>(new Proto::ProtoMessage());
  try {
    switch (static_cast<int>(sMessage.type())) {
      case ProtoMessage::kConnect: {
        onConnect(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kClientInfo: {
        onSetClientId(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kDisconnect: {
        onDisconnect(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kSession: {
        onCreateSession(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kUnsession: {
        onCloseSession(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kDestination: {
        onDestination(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kUndestination: {
        onUndestination(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kBegin: {
        onBegin(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kCommit: {
        onCommit(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kAbort: {
        onAbort(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kMessage: {
        onMessage(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kAck: {
        onAcknowledge(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kSender: {
        onSender(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kUnsender: {
        onUnsender(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kSubscription: {
        onSubscription(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kSubscribe: {
        onSubscribe(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kUnsubscribe: {
        onUnsubscribe(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kUnsubscription: {
        onUnsubscription(ahandler, sMessage, *outMessage);
      } break;
      case ProtoMessage::kBrowser: {
        onBrowser(ahandler, sMessage, *outMessage);
      } break;
      default:
        throw EXCEPTION("unknown message type", std::to_string(static_cast<int>(sMessage.type())), ERROR_UNKNOWN);
    }
  } catch (Exception &ex) {
    outMessage->protoMessage().Clear();
    outMessage->protoMessage().mutable_error()->set_error_code(static_cast<Proto::ErrorCode>(ex.error()));
    outMessage->protoMessage().mutable_error()->set_error_message(ex.message());
    ASYNCLOG_ERROR(ahandler.logStream, (std::to_string(sMessage.handlerNum).append(" ! => ").append(std::string(ex.what())) += non_std_endl));
    if (sMessage.isMessage()) {
      sMessage.removeLinkedFile();
    }
  }
  if (sMessage.isNeedReceipt() || sMessage.isConnect() || sMessage.isBrowser() || outMessage->protoMessage().has_error()) {
    if (outMessage->protoMessage().has_error()) {
      outMessage->protoMessage().mutable_error()->set_receipt_id(sMessage.receiptId());
    } else if (sMessage.isNeedReceipt()) {
      outMessage->protoMessage().mutable_receipt()->set_receipt_id(sMessage.receiptId());
    }
    outMessage->protoMessage().set_object_id(sMessage.objectID());
    outMessage->setRRID(sMessage.rrID());
    outMessage->serialize();
    const_cast<AsyncTCPHandler &>(ahandler).put(std::move(outMessage));
    ASYNCLOG_INFORMATION(ahandler.logStream, (std::to_string(sMessage.handlerNum).append(" * <= ").append("send reply on ").append(sMessage.typeName()) += non_std_endl));
  }
}
void Broker::onConnect(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(tcpHandler);
  const Proto::Connect &connect = sMessage.connect();

  {
    upmq::ScopedWriteRWLock writeRWLock(_connectionsLock);
    auto it = _connections.find(connect.client_id());
    if (it == _connections.end()) {
      _connections.insert(std::make_pair(connect.client_id(), std::make_unique<Connection>(connect.client_id())));
      it = _connections.find(connect.client_id());
    } else {
      if (it->second->isTcpConnectionExists(sMessage.handlerNum)) {
        throw EXCEPTION("connection already exists", connect.client_id() + " : " + std::to_string(sMessage.handlerNum), ERROR_CLIENT_ID_EXISTS);
      }
    }
    it->second->addTcpConnection(sMessage.handlerNum);
    tcpHandler.setConnection(it->second.get());
  }

  Proto::Connected &connected = outMessage.createConnected(sMessage.objectID());

  connected.mutable_heartbeat()->set_send_timeout(Singleton<Configuration>::Instance().heartbeat().sendTimeout);
  connected.mutable_heartbeat()->set_recv_timeout(Singleton<Configuration>::Instance().heartbeat().recvTimeout);
  connected.mutable_server_version()->set_server_major_version(MQ_VERSION_MAJOR);
  connected.mutable_server_version()->set_server_minor_version(MQ_VERSION_MINOR);
  connected.mutable_server_version()->set_server_revision_version(MQ_VERSION_REVISION);
  connected.mutable_server_version()->set_server_vendor_id("upmq(cpp.ver)");
  Proto::ServerVersion serverVersion;
  connected.mutable_protocol_version()->set_protocol_major_version(serverVersion.server_major_version());
  connected.mutable_protocol_version()->set_protocol_minor_version(serverVersion.server_minor_version());
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
}
void Broker::removeTcpConnection(const std::string &clientID, size_t tcpConnectionNum) {
  auto it = _connections.end();
  {
    upmq::ScopedReadRWLock readRWLock(_connectionsLock);
    it = _connections.find(clientID);
    if (it != _connections.end()) {
      it->second->removeTcpConnection(tcpConnectionNum);
    }
  }
  if (it != _connections.end() && it->second->tcpConnectionsCount() == 0) {
    eraseConnection(clientID);
    ASYNCLOG_INFORMATION(logStream, (std::to_string(tcpConnectionNum).append(" # => ").append("erase connection ").append(clientID) += non_std_endl));
  }
}
void Broker::removeTcpConnection(Connection &connection, size_t tcpConnectionNum) {
  connection.removeTcpConnection(tcpConnectionNum);
  if (connection.tcpConnectionsCount() == 0) {
    eraseConnection(connection.clientID());
    ASYNCLOG_INFORMATION(logStream, (std::to_string(tcpConnectionNum).append(" # => ").append("erase connection ").append(connection.clientID()) += non_std_endl));
  }
}
void Broker::removeConsumers(const std::string &destinationID, const std::string &subscriptionID, size_t tcpNum) {
  upmq::ScopedReadRWLock readRWLock(_connectionsLock);
  for (const auto &conn : _connections) {
    conn.second->removeConsumers(destinationID, subscriptionID, tcpNum);
  }
}
void Broker::onSetClientId(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(tcpHandler);
  UNUSED_VAR(outMessage);
  const Proto::ClientInfo &clientInfo = sMessage.clientInfo();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" : from ").append(clientInfo.old_client_id()).append(" to ").append(clientInfo.new_client_id()) += non_std_endl));

  if (isConnectionExists(clientInfo.new_client_id())) {
    throw EXCEPTION("connection already exists", clientInfo.new_client_id(), ERROR_CLIENT_ID_EXISTS);
  }
  if (!isConnectionExists(clientInfo.old_client_id())) {
    throw EXCEPTION("connection not found", clientInfo.old_client_id(), ERROR_CONNECTION);
  }

  {
    upmq::ScopedWriteRWLock wlock(_connectionsLock);
    auto it = _connections.find(clientInfo.old_client_id());
    if (it != _connections.end()) {
      it->second->setClientID(clientInfo.new_client_id());
    }
    std::unique_ptr<Connection> connection = std::move(it->second);
    _connections.erase(it);
    _connections.insert(std::make_pair(connection->clientID(), std::move(connection)));
  }
}
void Broker::eraseConnection(const std::string &connectionID) {
  {
    upmq::ScopedWriteRWLock wlock(_connectionsLock);
    _connections.erase(connectionID);
  }
  ASYNCLOG_INFORMATION(logStream, (std::string("-").append(" # => ").append(" erased ").append(connectionID) += non_std_endl));
}
void Broker::onDisconnect(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  const Proto::Disconnect &disconnect = sMessage.disconnect();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" id : ").append(disconnect.client_id()) += non_std_endl));
  outMessage.toDisconnect = true;
}
void Broker::onCreateSession(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Proto::Session &session = sMessage.session();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" id : ").append(session.session_id()).append(" ack : ").append(Session::acknowlegeName(session.acknowledge_type())) += non_std_endl));
  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_SESSION);
  }
  tcpHandler.connection()->addSession(session.session_id(), session.acknowledge_type());
}
void Broker::onCloseSession(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Proto::Unsession &unsession = sMessage.unsession();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" id : ").append(unsession.session_id()) += non_std_endl));
  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_UNSESSION);
  }
  tcpHandler.connection()->removeSession(unsession.session_id(), tcpHandler.num);
}
void Broker::onBegin(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Proto::Begin &begin = sMessage.begin();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" : on session : ").append(begin.session_id()) += non_std_endl));
  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_BEGIN);
  }
  tcpHandler.connection()->beginTX(begin.session_id());
}
void Broker::onCommit(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Proto::Commit &commit = sMessage.commit();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" : on session : ").append(commit.session_id()) += non_std_endl));

  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_COMMIT);
  }
  tcpHandler.connection()->commitTX(commit.session_id());
}
void Broker::onAbort(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Proto::Abort &abort = sMessage.abort();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" : on session : ").append(abort.session_id()) += non_std_endl));
  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_COMMIT);
  }
  tcpHandler.connection()->abortTX(abort.session_id());
}
void Broker::onMessage(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);

  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_SAVE_MESSAGE);
  }

  const Message &constMessage = sMessage.message();
  Destination &dest = EXCHANGE::Instance().destination(constMessage.destination_uri());  // NOTE: !NEED for pre-creation of destination, try to mitigate deadlock
  UNUSED_VAR(dest);
  if (DESTINATION_CONFIG.forwardByProperty && (constMessage.property_size() > 0)) {
    auto &msg = const_cast<MessageDataContainer &>(sMessage);
    auto &pmap = *msg.mutableMessage().mutable_property();
    const auto propIt = pmap.find(broker::s2s::proto::upmq_s2s_destination_broker_name);
    if ((propIt != pmap.end()) && (propIt->second.value_string() != CONFIGURATION::Instance().name())) {
      std::string destinationName = DestinationFactory::destinationName(constMessage.destination_uri());
      Destination::Type destinationType = DestinationFactory::destinationType(constMessage.destination_uri());

      pmap[broker::s2s::proto::upmq_s2s_source_destination_name].set_value_string(destinationName);
      pmap[broker::s2s::proto::upmq_s2s_source_destination_name].set_is_null(false);

      pmap[broker::s2s::proto::upmq_s2s_source_destination_type].set_value_int(static_cast<int>(destinationType));
      pmap[broker::s2s::proto::upmq_s2s_source_destination_type].set_is_null(false);

      pmap[broker::s2s::proto::upmq_s2s_source_broker_name].set_value_string(CONFIGURATION::Instance().name());
      pmap[broker::s2s::proto::upmq_s2s_source_broker_name].set_is_null(false);

      std::string s2sQueue(QUEUE_PREFIX);
      s2sQueue.append("://s2s");
      msg.mutableMessage().set_destination_uri(s2sQueue);
      if (constMessage.persistent()) {
        sMessage.moveDataTo(s2sQueue);
      }
    }
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string("id [").append(constMessage.message_id()).append("] : into destination : ").append(constMessage.destination_uri()) += non_std_endl));
  tcpHandler.connection()->saveMessage(sMessage);
  EXCHANGE::Instance().postNewMessageEvent(constMessage.destination_uri());
}
void Broker::onSender(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);

  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_SENDER);
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
  Destination &dest = EXCHANGE::Instance().destination(sMessage.sender().destination_uri());  // NOTE: !NEED for pre-creation of destination, try to mitigate deadlock
  UNUSED_VAR(dest);
  tcpHandler.connection()->addSender(sMessage);
}
void Broker::onUnsender(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);

  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_UNSENDER);
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
  tcpHandler.connection()->removeSender(sMessage);
}
void Broker::onSubscription(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_SUBSCRIPTION);
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" : on destination [").append(sMessage.subscription().destination_uri()).append("]") += non_std_endl));
  Destination &dest = EXCHANGE::Instance().destination(sMessage.subscription().destination_uri());  // NOTE: !NEED for pre-creation of destination, try to mitigate deadlock
  UNUSED_VAR(dest);
  tcpHandler.connection()->addSubscription(sMessage);
}
void Broker::onSubscribe(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Subscribe &subscribe = sMessage.subscribe();
  const std::string &name = subscribe.subscription_name();
  if (name.empty()) {
    throw EXCEPTION("subscription name is empty", "subscribe", ERROR_ON_SUBSCRIBE);
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
  EXCHANGE::Instance().destination(subscribe.destination_uri(), Exchange::DestinationCreationMode::NO_CREATE).subscribe(sMessage);
}
void Broker::onUnsubscribe(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
  try {
    EXCHANGE::Instance().destination(sMessage.unsubscribe().destination_uri(), Exchange::DestinationCreationMode::NO_CREATE).unsubscribe(sMessage);
  } catch (Exception &ex) {
    if (ex.error() != ERROR_UNKNOWN) {
      throw Exception(ex);
    }
  }
}
void Broker::onUnsubscription(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_UNSUBSCRIPTION);
  }
  try {
    tcpHandler.connection()->removeConsumer(sMessage, tcpHandler.num);
  } catch (Exception &ex) {
    if (ex.error() != ERROR_UNKNOWN) {
      throw Exception(ex);
    }
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" : from destination [").append(sMessage.unsubscription().destination_uri()).append("]") += non_std_endl));
  tcpHandler.eraseSubscription(sMessage);
}
void Broker::onAcknowledge(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  if (tcpHandler.connection() == nullptr) {
    throw EXCEPTION("connection not found", sMessage.clientID, ERROR_ON_ACK_MESSAGE);
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, (std::string(" on message id [").append(sMessage.ack().message_id()).append("]") += non_std_endl));
  tcpHandler.connection()->processAcknowledge(sMessage);
}
void Broker::onBrowser(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  const Proto::Browser &browser = sMessage.browser();
  const std::string &name = browser.subscription_name();
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
  // NOTE: do subscribe into initBrowser
  int64_t count = EXCHANGE::Instance().destination(sMessage.browser().destination_uri(), Exchange::DestinationCreationMode::NO_CREATE).initBrowser(name);
  outMessage.protoMessage().mutable_browser_info()->set_message_count(count);
}
void Broker::onDestination(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Proto::Destination &destination = sMessage.destination();
  auto &dest = EXCHANGE::Instance().destination(destination.destination_uri());
  ASYNCLOG_INFORMATION(tcpHandler.logStream, std::string(" create destination (").append(destination.destination_uri()).append(")"));
  if (!dest.hasOwner()) {
    ASYNCLOG_INFORMATION(tcpHandler.logStream, std::string(" | set owner (").append(sMessage.clientID).append(" : ").append(std::to_string(tcpHandler.num)).append(")"));
    dest.setOwner(sMessage.clientID, tcpHandler.num);
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
}
void Broker::onUndestination(const AsyncTCPHandler &tcpHandler, const MessageDataContainer &sMessage, MessageDataContainer &outMessage) {
  UNUSED_VAR(outMessage);
  const Proto::Undestination &undestination = sMessage.undestination();
  try {
    auto &dest = EXCHANGE::Instance().destination(undestination.destination_uri(), Exchange::DestinationCreationMode::NO_CREATE);
    if (dest.hasOwner() && (dest.owner().clientID == sMessage.clientID) && (dest.owner().tcpID == tcpHandler.num)) {
      EXCHANGE::Instance().deleteDestination(undestination.destination_uri());
      ASYNCLOG_INFORMATION(tcpHandler.logStream, std::string(" delete destination (").append(undestination.destination_uri()).append(")"));
      ASYNCLOG_INFORMATION(tcpHandler.logStream, std::string(" | owner (").append(sMessage.clientID).append(" : ").append(std::to_string(tcpHandler.num)).append(")"));
    }
  } catch (Exception &) {
    // NOTE : if destination not exists then do nothing
  }
  ASYNCLOG_INFORMATION(tcpHandler.logStream, non_std_endl);
}
bool Broker::isConnectionExists(const std::string &clientID) {
  upmq::ScopedReadRWLock readRWLock(_connectionsLock);
  auto it = _connections.find(clientID);
  return (it != _connections.end());
}
std::string Broker::currentTransaction(const std::string &clientID, const std::string &sessionID) const {
  upmq::ScopedReadRWLock rLock(_connectionsLock);
  auto it = _connections.find(clientID);
  if (it == _connections.end()) {
    throw EXCEPTION("connection not found", clientID, ERROR_CONNECTION);
  }
  return it->second->transactionID(sessionID);
}
void Broker::onWritable() {
  size_t indexNum = _writableIndexCounter++;
  _isWritable = true;
  size_t num = 0;
  auto &blockingConcurrentQueue = _writableIndexes[indexNum];
  do {
    try {
      num = 0;
      while (blockingConcurrentQueue.wait_dequeue_timed(num, 1000000)) {
        if (!write(num)) {
          blockingConcurrentQueue.enqueue(num);
        }
      }
    } catch (std::exception &ex) {
      ASYNCLOG_ERROR(logStream, (std::string("-").append(" ! => write error : ").append(std::string(ex.what())) += non_std_endl));
    }
  } while (_isWritable);
}
bool Broker::write(size_t num) {
  auto ahandler = AHRegestry::Instance().aHandler(num);
  if (ahandler != nullptr) {
    if (ahandler->onWritableLock.tryLock()) {
      std::shared_ptr<MessageDataContainer> sMessage;
      do {
        if (ahandler->needErase()) {
          ahandler->onWritableLock.unlock();
          return true;
        }
        try {
          sMessage.reset();
          if (ahandler->outputQueue.try_dequeue(sMessage) && sMessage != nullptr) {
            if (sMessage->header.empty()) {
              sMessage->serialize();
            }
            const std::string &messageId = sMessage->isMessage() ? sMessage->message().message_id() : emptyString;
            AsyncTCPHandler::DataStatus status = AsyncTCPHandler::DataStatus::TRYAGAIN;
            do {
              status = ahandler->sendHeaderAndData(*sMessage);
              if (status == AsyncTCPHandler::DataStatus::OK) {
                ASYNCLOG_INFORMATION(ahandler->logStream,
                                     (std::to_string(num)
                                          .append(" * <= ")
                                          .append("sent ")
                                          .append(sMessage->typeName())
                                          .append(" id[")
                                          .append(messageId)
                                          .append("]")
                                          .append(" to (")
                                          .append(sMessage->objectID())
                                          .append("/")
                                          .append(ahandler->peerAddress())
                                          .append(")") += non_std_endl));
                if (sMessage->toDisconnect) {
                  ahandler->onWritableLock.unlock();
                  ahandler->emitCloseEvent();
                  return true;
                }
              } else {
                if (status == AsyncTCPHandler::DataStatus::AS_ERROR) {
                  ahandler->onWritableLock.unlock();
                  return status == AsyncTCPHandler::DataStatus::AS_ERROR;
                }
                Poco::Thread::yield();
              }
            } while (status == AsyncTCPHandler::DataStatus::TRYAGAIN && _isWritable);
          }
        } catch (Exception &ex) {
          ASYNCLOG_ERROR(ahandler->logStream,
                         (std::to_string(num).append(" ! <= AsyncTCPHandler::sendHeaderAndData : (").append(std::to_string(ex.error())).append(") ").append(ex.message()) += non_std_endl));
        } catch (Poco::Exception &ex) {
          ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! <= AsyncTCPHandler::sendHeaderAndData : (").append(ex.className()).append(") ").append(ex.message()) += non_std_endl));
        } catch (...) {
          ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! <= AsyncTCPHandler::sendHeaderAndData : (").append("unknown error").append(") ") += non_std_endl));
        }
      } while (sMessage != nullptr && !ahandler->needErase());
      ahandler->onWritableLock.unlock();
    } else {
      return false;
    }
  }
  return true;
}
void Broker::onReadable() {
  size_t indexNum = _readableIndexCounter++;
  _isReadable = true;
  size_t num = 0;
  auto &blockingConcurrentQueue = _readableIndexes[indexNum];
  do {
    try {
      num = 0;
      while (blockingConcurrentQueue.wait_dequeue_timed(num, 1000000)) {
        read(num);
      }
    } catch (std::exception &ex) {
      ASYNCLOG_ERROR(logStream, (std::string("-").append(" ! => read error : ").append(std::string(ex.what())) += non_std_endl));
    }
  } while (_isReadable);
}
bool Broker::read(size_t num) {
  auto ahandler = AHRegestry::Instance().aHandler(num);
  if (ahandler != nullptr) {
    if (ahandler->onReadableLock.tryLock()) {
      if (ahandler->needErase()) {
        return true;
      }
      memset(ahandler->pBuffer, 0, BUFFER_SIZE);
      MessageDataContainer sMessage(STORAGE_CONFIG.data.bigFilesPath().toString());
      try {
        switch (ahandler->fillHeaderBodyLens()) {
          case AsyncTCPHandler::DataStatus::AS_ERROR:
            ahandler->onReadableLock.unlock();
            ahandler->emitCloseEvent(false);
            return true;
          case AsyncTCPHandler::DataStatus::TRYAGAIN:
            ahandler->onReadableLock.unlock();
            ahandler->allowPutReadEvent();
            return false;
          case AsyncTCPHandler::DataStatus::OK:
            break;
        }
      } catch (Poco::Exception &ex) {
        ASYNCLOG_ERROR(
            ahandler->logStream,
            (std::to_string(num).append(" ! => AsyncTCPHandler::read::fillHeaderBodyLens => (").append(ex.className()).append(") ").append(ex.message()).append(" : ").append(std::string(ex.what())) +=
             non_std_endl));
        ahandler->onReadableLock.unlock();
        ahandler->emitCloseEvent(true);
        return true;
      } catch (...) {
        ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => AsyncTCPHandler::read::fillHeaderBodyLens => (unknown error) ") += non_std_endl));
        ahandler->onReadableLock.unlock();
        ahandler->emitCloseEvent(true);
        return true;
      }

      try {
        if (ahandler->fillHeader(sMessage) == AsyncTCPHandler::DataStatus::AS_ERROR) {
          ahandler->onReadableLock.unlock();
          ahandler->emitCloseEvent(true);
          return true;
        }
      } catch (Poco::Exception &ex) {
        ASYNCLOG_ERROR(
            ahandler->logStream,
            (std::to_string(num).append(" ! => AsyncTCPHandler::read::fillHeader => (").append(ex.className()).append(") ").append(ex.message()).append(" : ").append(std::string(ex.what())) +=
             non_std_endl));
        ahandler->onReadableLock.unlock();
        ahandler->emitCloseEvent(true);
        return true;
      } catch (...) {
        ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => AsyncTCPHandler::read::fillHeader => (unknown error) ") += non_std_endl));
        ahandler->onReadableLock.unlock();
        ahandler->emitCloseEvent(true);
        return true;
      }

      if (ahandler->headerBodyLens.bodyLen != 0) {
        try {
          if (ahandler->fillBody(sMessage) == AsyncTCPHandler::DataStatus::AS_ERROR) {
            ahandler->onReadableLock.unlock();
            ahandler->emitCloseEvent(true);
            return true;
          }
        } catch (Poco::Exception &ex) {
          ASYNCLOG_ERROR(
              ahandler->logStream,
              (std::to_string(num).append(" ! => AsyncTCPHandler::read::fillBody => (").append(ex.className()).append(") ").append(ex.message()).append(" : ").append(std::string(ex.what())) +=
               non_std_endl));
          ahandler->onReadableLock.unlock();
          ahandler->emitCloseEvent(true);
          return true;
        } catch (std::exception &ex) {
          ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => AsyncTCPHandler::read::fillBody => ").append(ex.what()) += non_std_endl));
          ahandler->onReadableLock.unlock();
          ahandler->emitCloseEvent(true);
          return true;
        } catch (...) {
          ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => AsyncTCPHandler::read::fillBody => (unknown error) ") += non_std_endl));
          ahandler->onReadableLock.unlock();
          ahandler->emitCloseEvent(true);
          return true;
        }
      } else {
        if (sMessage.isMessage() && (sMessage.message().property_size() > 0)) {
          try {
            ahandler->tryMoveBodyByLink(sMessage);
          } catch (Poco::Exception &ex) {
            ASYNCLOG_ERROR(
                ahandler->logStream,
                (std::to_string(num).append(" ! => AsyncTCPHandler::read::tryMoveBodyByLink => (").append(ex.className()).append(") ").append(ex.message()).append(" : ").append(ex.what()) +=
                 non_std_endl));
            ahandler->onReadableLock.unlock();
            ahandler->emitCloseEvent(true);
            return true;
          } catch (...) {
            ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => AsyncTCPHandler::read::tryMoveBodyByLink => (unknown error) ") += non_std_endl));
            ahandler->onReadableLock.unlock();
            ahandler->emitCloseEvent(true);
            return true;
          }
        }
      }

      try {
        if (sMessage.isNotForServer()) {
          ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => message is not for server ( type is ").append(sMessage.typeName()).append(" )") += non_std_endl));
          ahandler->onReadableLock.unlock();
          ahandler->emitCloseEvent();
          return true;
        }
        sMessage.handlerNum = num;
        sMessage.clientID = ((ahandler->connection() != nullptr) ? ahandler->connection()->clientID() : emptyString);

        switch (static_cast<int>(sMessage.type())) {
          case ProtoMessage::kPing: {
            std::shared_ptr<MessageDataContainer> dc = std::make_shared<MessageDataContainer>(new Proto::ProtoMessage());
            dc->protoMessage().mutable_pong();
            dc->setObjectID(sMessage.objectID());
            dc->setRRID(sMessage.rrID());
            dc->serialize();
            ahandler->put(std::move(dc));
            ahandler->onReadableLock.unlock();
            ahandler->allowPutReadEvent();
          }
            return true;
          case ProtoMessage::kConnect: {
            ASYNCLOG_INFORMATION(ahandler->logStream, (std::to_string(num).append(" * ").append("=> get connect frame") += non_std_endl));
            ahandler->storeClientInfo(sMessage);
            ASYNCLOG_INFORMATION(ahandler->logStream, (std::to_string(num).append(" # => ").append(ahandler->toString()) += non_std_endl));
          } break;
          case ProtoMessage::kSubscription: {
            ASYNCLOG_INFORMATION(ahandler->logStream, (std::to_string(num).append(" * ").append("=> get subscription frame") += non_std_endl));
            ahandler->initSubscription(sMessage);
          } break;
          default: {
            ASYNCLOG_INFORMATION(ahandler->logStream, (std::to_string(num).append(" * ").append("=> get ").append(sMessage.typeName()).append(" frame") += non_std_endl));
            break;
          }
        }

        onEvent(*ahandler, sMessage);
        ahandler->allowPutReadEvent();
      } catch (Exception &ex) {
        ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => internal error : ").append(ex.message()) += non_std_endl));
        ahandler->onReadableLock.unlock();
        ahandler->emitCloseEvent();
        return true;
      } catch (std::exception &pbex) {
        ASYNCLOG_ERROR(ahandler->logStream, (std::to_string(num).append(" ! => message parsing error : ").append(std::string(pbex.what())) += non_std_endl));
        ahandler->onReadableLock.unlock();
        ahandler->emitCloseEvent();
        return true;
      }
      ahandler->onReadableLock.unlock();
      return true;
    }
  }
  return true;
}
void Broker::start() {
  if (!_isReadable) {
    _readbleAdapter = std::make_unique<Poco::RunnableAdapter<Broker>>(*this, &Broker::onReadable);
    int count = _readablePool.capacity() - 1;
    for (int i = 0; i < count; ++i) {
      _readablePool.start(*_readbleAdapter);
    }
  }
  if (!_isWritable) {
    _writableAdapter = std::make_unique<Poco::RunnableAdapter<Broker>>(*this, &Broker::onWritable);
    int count = _writablePool.capacity() - 1;
    for (int i = 0; i < count; ++i) {
      _writablePool.start(*_writableAdapter);
    }
  }
}
void Broker::stop() {
  if (_isRunning) {
    _isRunning = false;
  }
  if (_isReadable) {
    _isReadable = false;
    _readablePool.joinAll();
  }
  if (_isWritable) {
    _isWritable = false;
    _writablePool.joinAll();
  }
}
void Broker::putReadable(size_t queueNum, size_t num) {
  if (!_isReadable) {
    return;
  }
  rwput(_isReadable, _readableIndexes[queueNum], num);
}
void Broker::putWritable(size_t queueNum, size_t num) {
  if (!_isWritable) {
    return;
  }
  rwput(_isWritable, _writableIndexes[queueNum], num);
}

size_t Broker::connectionsSize() const {
  upmq::ScopedReadRWLock readRWLock(_connectionsLock);
  return _connections.size();
}

void Broker::rwput(std::atomic_bool &isValid, BQIndexes &bqIndex, size_t num) {
  bool result = false;
  do {
    if (!isValid) {
      return;
    }
    result = bqIndex.enqueue(num);
  } while (!result);
}

}  // namespace broker
}  // namespace upmq
