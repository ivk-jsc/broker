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

#include "SimplePriorityMessageDispatchChannel.h"

#include <cms/Message.h>
#include <decaf/lang/Math.h>

#include "UPMQCommand.h"

using namespace std;
using namespace upmq;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;

////////////////////////////////////////////////////////////////////////////////
const int SimplePriorityMessageDispatchChannel::MAX_PRIORITIES = 10;

////////////////////////////////////////////////////////////////////////////////
SimplePriorityMessageDispatchChannel::SimplePriorityMessageDispatchChannel()
    : closed(false), running(false), mutex(), channels(MAX_PRIORITIES), enqueued(0) {}

////////////////////////////////////////////////////////////////////////////////
SimplePriorityMessageDispatchChannel::~SimplePriorityMessageDispatchChannel() {}

////////////////////////////////////////////////////////////////////////////////
void SimplePriorityMessageDispatchChannel::enqueue(const Pointer<Command>& message) {
  synchronized(&mutex) {
    this->getChannel(message).addLast(message);
    this->enqueued++;
    mutex.notify();
  }
}

////////////////////////////////////////////////////////////////////////////////
void SimplePriorityMessageDispatchChannel::enqueueFirst(const Pointer<Command>& message) {
  synchronized(&mutex) {
    this->getChannel(message).addFirst(message);
    this->enqueued++;
    mutex.notify();
  }
}

////////////////////////////////////////////////////////////////////////////////
bool SimplePriorityMessageDispatchChannel::isEmpty() const { return this->enqueued == 0; }

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> SimplePriorityMessageDispatchChannel::dequeue(long long timeout) {
  Pointer<Command> command;
  synchronized(&mutex) {
    // Wait until the channel is ready to deliver messages.
    while (timeout != 0 && !closed && (isEmpty() || !running)) {
      if (timeout == -1) {
        mutex.wait();
      } else {
        mutex.wait(static_cast<unsigned long>(timeout));
        break;
      }
    }

    if (!closed && running && !isEmpty()) {
      command = removeFirst();
    }
  }

  return command;
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> SimplePriorityMessageDispatchChannel::dequeueNoWait() {
  Pointer<Command> command;
  synchronized(&mutex) {
    if (!closed && running && !isEmpty()) {
      command = removeFirst();
    }
  }

  return command;
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> SimplePriorityMessageDispatchChannel::peek() const {
  Pointer<Command> command;
  synchronized(&mutex) {
    if (!closed && running && !isEmpty()) {
      command = getFirst();
    }
  }

  return command;
}

////////////////////////////////////////////////////////////////////////////////
void SimplePriorityMessageDispatchChannel::start() {
  synchronized(&mutex) {
    if (!closed) {
      running = true;
      mutex.notifyAll();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void SimplePriorityMessageDispatchChannel::stop() {
  synchronized(&mutex) {
    running = false;
    mutex.notifyAll();
  }
}

////////////////////////////////////////////////////////////////////////////////
void SimplePriorityMessageDispatchChannel::close() {
  synchronized(&mutex) {
    if (!closed) {
      running = false;
      closed = true;
    }
    mutex.notifyAll();
  }
}

////////////////////////////////////////////////////////////////////////////////
void SimplePriorityMessageDispatchChannel::clear() {
  synchronized(&mutex) {
    for (int i = 0; i < MAX_PRIORITIES; i++) {
      this->channels[i].clear();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
int SimplePriorityMessageDispatchChannel::size() const {
  int size = 0;
  synchronized(&mutex) { size = this->enqueued; }

  return size;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Pointer<Command> > SimplePriorityMessageDispatchChannel::removeAll() {
  std::vector<Pointer<Command> > result;

  synchronized(&mutex) {
    for (int i = MAX_PRIORITIES - 1; i >= 0; --i) {
      std::vector<Pointer<Command> > temp(channels[i].toArray());
      result.insert(result.end(), temp.begin(), temp.end());
      this->enqueued -= (int)temp.size();
      channels[i].clear();
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
LinkedList<Pointer<Command> >& SimplePriorityMessageDispatchChannel::getChannel(const Pointer<Command>& dispatch) {
  int priority = cms::Message::DEFAULT_MSG_PRIORITY;

  if (dispatch.get() != nullptr) {
    priority = Math::max(((UPMQCommand*)dispatch.get())->_header->message().priority(), 0);
    priority = Math::min(priority, 9);
  }

  return this->channels[priority];
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> SimplePriorityMessageDispatchChannel::removeFirst() {
  if (this->enqueued > 0) {
    for (int i = MAX_PRIORITIES - 1; i >= 0; i--) {
      LinkedList<Pointer<Command> >& channel = channels[i];
      if (!channel.isEmpty()) {
        this->enqueued--;
        return channel.pop();
      }
    }
  }

  return Pointer<Command>();
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Command> SimplePriorityMessageDispatchChannel::getFirst() const {
  if (this->enqueued > 0) {
    for (int i = MAX_PRIORITIES - 1; i >= 0; i--) {
      LinkedList<Pointer<Command> >& channel = channels[i];
      if (!channel.isEmpty()) {
        return channel.getFirst();
      }
    }
  }

  return Pointer<Command>();
}
