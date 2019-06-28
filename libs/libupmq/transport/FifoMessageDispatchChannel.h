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

#ifndef _UPMQ_CORE_FIFOMESSAGEDISPATCHCHANNEL_H_
#define _UPMQ_CORE_FIFOMESSAGEDISPATCHCHANNEL_H_

#include <transport/Config.h>
#include <transport/MessageDispatchChannel.h>

#include <decaf/lang/Pointer.h>
#include <decaf/util/LinkedList.h>

namespace upmq {
namespace transport {

class UPMQCPP_API FifoMessageDispatchChannel : public MessageDispatchChannel {
 private:
  bool closed;
  bool running;

  mutable decaf::util::LinkedList<Pointer<Command> > channel;

 private:
  FifoMessageDispatchChannel(const FifoMessageDispatchChannel &);
  FifoMessageDispatchChannel &operator=(const FifoMessageDispatchChannel &);

 public:
  FifoMessageDispatchChannel();

  virtual ~FifoMessageDispatchChannel();

  void enqueue(const Pointer<Command> &message) override;

  void enqueueFirst(const Pointer<Command> &message) override;

  bool isEmpty() const override;

  bool isClosed() const override { return this->closed; }

  bool isRunning() const override { return this->running; }

  Pointer<Command> dequeue(long long timeout) override;

  Pointer<Command> dequeueNoWait() override;

  Pointer<Command> peek() const override;

  void start() override;

  void stop() override;

  void close() override;

  void clear() override;

  int size() const override;

  std::vector<Pointer<Command> > removeAll() override;

 public:
  void lock() override { channel.lock(); }

  bool tryLock() override { return channel.tryLock(); }

  void unlock() override { channel.unlock(); }

  void wait() override { channel.wait(); }

  void wait(long long millisecs) override { channel.wait(millisecs); }

  void wait(long long millisecs, int nanos) override { channel.wait(millisecs, nanos); }

  void notify() override { channel.notify(); }

  void notifyAll() override { channel.notifyAll(); }
};
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_CORE_FIFOMESSAGEDISPATCHCHANNEL_H_ */
