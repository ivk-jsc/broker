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

#ifndef BROKER_ASYNCHANDLERREGESTRY_H
#define BROKER_ASYNCHANDLERREGESTRY_H

#include <Poco/Condition.h>
#include <Poco/RWLock.h>
#include <Poco/Runnable.h>
#include <unordered_map>
#include "AsyncTCPHandler.h"
#include "MessageDataContainer.h"
#include "Singleton.h"
#include "BlockingConcurrentQueueHeader.h"
namespace upmq {
namespace broker {

class AsyncHandlerRegestry : public Poco::Runnable {
 public:
  typedef std::vector<std::shared_ptr<AsyncTCPHandler>> ConnectionsListType;
  AsyncHandlerRegestry();
  ~AsyncHandlerRegestry() override;
  void addAHandler(std::shared_ptr<AsyncTCPHandler> ahandler);
  std::shared_ptr<AsyncTCPHandler> aHandler(size_t num) const;
  void deleteAHandler(size_t num);
  void put(size_t num, std::shared_ptr<MessageDataContainer> sMessage);
  void run() override;

  void start();
  void stop();
  void notify();
  size_t size() const;

  void needToErase(size_t num);

 private:
  size_t _size{0};
  Poco::Thread _thread;
  Poco::FastMutex _mutex;
  Poco::Condition _condition;
  ConnectionsListType _connections;
  mutable moodycamel::BlockingConcurrentQueue<int> _freeNums{};
  moodycamel::BlockingConcurrentQueue<int> _needToErase{};
  std::atomic_bool _isRunning;
  std::atomic_size_t _current_size;
  int erasedConnections();
  int freeNum() const;

 public:
  std::atomic_size_t _connectionCounter;
};
}  // namespace broker
}  // namespace upmq

typedef Singleton<upmq::broker::AsyncHandlerRegestry> AHRegestry;

#endif  // BROKER_ASYNCHANDLERREGESTRY_H
