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

#include "AsyncHandlerRegestry.h"
#include <Exception.h>
#include <Poco/Hash.h>
#include "Configuration.h"

namespace upmq {
namespace broker {
AsyncHandlerRegestry::AsyncHandlerRegestry()
    : _size(static_cast<size_t>(NET_CONFIG.maxConnections)),
      _thread("AsyncHandlerRegestry"),
      _connections(_size),
      _isRunning(false),
      _current_size(0),
      _connectionCounter(0) {
  for (int i = 0; i < static_cast<int>(_size); ++i) {
    _freeNums.enqueue(i);
  }
}

void AsyncHandlerRegestry::addAHandler(AsyncTCPHandler *ahandler) {
  int nextNum = freeNum();
  if (nextNum != -1) {
    ahandler->num = static_cast<size_t>(nextNum);
    _connections[ahandler->num] = std::shared_ptr<AsyncTCPHandler>(ahandler);
    ++_current_size;
  } else {
    throw EXCEPTION("can't get free connection handeler", "try to encrease max connections", -1);
  }
}

std::shared_ptr<AsyncTCPHandler> AsyncHandlerRegestry::aHandler(size_t num) const { return _connections[num]; }

void AsyncHandlerRegestry::deleteAHandler(size_t num) {
  _freeNums.enqueue(static_cast<int>(num));
  --_current_size;
}
void AsyncHandlerRegestry::put(size_t num, std::shared_ptr<MessageDataContainer> sMessage) {
  if (_connections[num] == nullptr) {
    throw EXCEPTION("tcp connection not found", std::to_string(num), Proto::ERROR_CONNECTION);
  }
  _connections[num]->put(std::move(sMessage));
}
void AsyncHandlerRegestry::run() {
  _isRunning = true;
  int num;
  while (_isRunning) {
    if (_needToErase.wait_dequeue_timed(num, 1000000)) {
      auto &connection = _connections[static_cast<size_t>(num)];
      if (connection && connection->needErase()) {
        if (connection->readComplete()) {
          connection = nullptr;
        } else {
          connection->onReadable(nullptr);
          if (connection->readComplete()) {
            connection = nullptr;
          }
        }
      }
    }
  }

  size_t cnt = 0;
  while (cnt != _connections.size()) {
    cnt = 0;
    for (auto &connection : _connections) {
      if (!connection) {
        ++cnt;
        continue;
      }
      if (connection->needErase()) {
        if (connection->readComplete()) {
          connection = nullptr;
        } else {
          connection->onReadable(nullptr);
        }
      } else {
        connection->setNeedErase();
        connection->onReadable(nullptr);
        if (connection->readComplete()) {
          connection = nullptr;
          ++cnt;
        }
      }
    }
  }

  try {
    _connections.clear();
  } catch (...) {
  }
}
int AsyncHandlerRegestry::erasedConnections() {
  int erased = 0;
  for (auto &connection : _connections) {
    if (connection && connection->needErase()) {
      connection = nullptr;
      ++erased;
    }
  }
  return erased;
}
void AsyncHandlerRegestry::start() {
  if (_isRunning) {
    return;
  }
  try {
    _thread.start(*this);
  } catch (Poco::Exception &pex) {
    throw EXCEPTION("can't start AsyncHandlerRegestry", pex.message(), -1);  // error name and number
  }
}

void AsyncHandlerRegestry::stop() {
  if (_isRunning) {
    _isRunning = false;
    _thread.join();
  }
}

size_t AsyncHandlerRegestry::size() const { return _current_size; }
int AsyncHandlerRegestry::freeNum() const {
  int num = 0;
  if (!_freeNums.wait_dequeue_timed(num, 1000000)) {
    return -1;
  }
  return num;
}
AsyncHandlerRegestry::~AsyncHandlerRegestry() = default;
void AsyncHandlerRegestry::needToErase(size_t num) { _needToErase.enqueue(static_cast<int>(num)); }
}  // namespace broker
}  // namespace upmq
