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

#include "FifoMessageDispatchChannel.h"

using namespace std;
using namespace upmq;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;

////////////////////////////////////////////////////////////////////////////////
FifoMessageDispatchChannel::FifoMessageDispatchChannel() : closed(false), running(false), channel() {}

////////////////////////////////////////////////////////////////////////////////
FifoMessageDispatchChannel::~FifoMessageDispatchChannel() {}

////////////////////////////////////////////////////////////////////////////////
void FifoMessageDispatchChannel::enqueue(const Pointer<Command> &message) {
  synchronized(&channel) {
    channel.addLast(message);
    channel.notify();
  }
}

////////////////////////////////////////////////////////////////////////////////
void FifoMessageDispatchChannel::enqueueFirst(const Pointer<Command> &message) {
  synchronized(&channel) {
    channel.addFirst(message);
    channel.notify();
  }
}

////////////////////////////////////////////////////////////////////////////////
bool FifoMessageDispatchChannel::isEmpty() const {
  bool isEmpty = false;
  synchronized(&channel) { isEmpty = channel.isEmpty(); }

  return isEmpty;
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> FifoMessageDispatchChannel::dequeue(long long timeout) {
  Pointer<Command> command;
  synchronized(&channel) {
    // Wait until the channel is ready to deliver messages.
    while (timeout != 0 && !closed && (channel.isEmpty() || !running)) {
      if (timeout == -1) {
        channel.wait();
      } else {
        channel.wait(static_cast<unsigned long>(timeout));
        break;
      }
    }

    if (!closed && running && !channel.isEmpty()) {
      command = channel.pop();
    }
  }

  return command;
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> FifoMessageDispatchChannel::dequeueNoWait() {
  Pointer<Command> command;
  synchronized(&channel) {
    if (!closed && running && !channel.isEmpty()) {
      command = channel.pop();
    }
  }

  return command;
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> FifoMessageDispatchChannel::peek() const {
  Pointer<Command> command;
  synchronized(&channel) {
    if (!closed && running && !channel.isEmpty()) {
      command = channel.getFirst();
    }
  }

  return command;
}

////////////////////////////////////////////////////////////////////////////////
void FifoMessageDispatchChannel::start() {
  synchronized(&channel) {
    if (!closed) {
      running = true;
      channel.notifyAll();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void FifoMessageDispatchChannel::stop() {
  synchronized(&channel) {
    running = false;
    channel.notifyAll();
  }
}

////////////////////////////////////////////////////////////////////////////////
void FifoMessageDispatchChannel::close() {
  synchronized(&channel) {
    if (!closed) {
      running = false;
      closed = true;
    }
    channel.notifyAll();
  }
}

////////////////////////////////////////////////////////////////////////////////
void FifoMessageDispatchChannel::clear() {
  synchronized(&channel) { channel.clear(); }
}

////////////////////////////////////////////////////////////////////////////////
int FifoMessageDispatchChannel::size() const {
  int size = 0;
  synchronized(&channel) { size = static_cast<int>(channel.size()); }

  return size;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Pointer<Command> > FifoMessageDispatchChannel::removeAll() {
  std::vector<Pointer<Command> > result;

  synchronized(&channel) {
    result = channel.toArray();
    channel.clear();
  }

  return result;
}
