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
 
#ifndef _UPMQ_CORE_SIMPLEPRIORITYMESSAGEDISPATCHCHANNEL_H_
#define _UPMQ_CORE_SIMPLEPRIORITYMESSAGEDISPATCHCHANNEL_H_

#include <transport/Config.h>
#include <transport/MessageDispatchChannel.h>

#include <decaf/lang/Pointer.h>
#include <decaf/lang/ArrayPointer.h>

#include <decaf/util/LinkedList.h>
#include <decaf/util/concurrent/Mutex.h>

namespace upmq {
namespace transport {

using decaf::lang::ArrayPointer;

class UPMQCPP_API SimplePriorityMessageDispatchChannel : public MessageDispatchChannel {
 private:
  static const int MAX_PRIORITIES;

  bool closed;
  bool running;

  mutable decaf::util::concurrent::Mutex mutex;

  mutable ArrayPointer<decaf::util::LinkedList<Pointer<Command> > > channels;

  int enqueued;

 private:
  SimplePriorityMessageDispatchChannel(const SimplePriorityMessageDispatchChannel&);
  SimplePriorityMessageDispatchChannel& operator=(const SimplePriorityMessageDispatchChannel&);

 public:
  SimplePriorityMessageDispatchChannel();
  virtual ~SimplePriorityMessageDispatchChannel();

  void enqueue(const Pointer<Command>& message) override;

  void enqueueFirst(const Pointer<Command>& message) override;

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
  void lock() override { mutex.lock(); }

  bool tryLock() override { return mutex.tryLock(); }

  void unlock() override { mutex.unlock(); }

  void wait() override { mutex.wait(); }

  void wait(long long millisecs) override { mutex.wait(millisecs); }

  void wait(long long millisecs, int nanos) override { mutex.wait(millisecs, nanos); }

  void notify() override { mutex.notify(); }

  void notifyAll() override { mutex.notifyAll(); }

 private:
  decaf::util::LinkedList<Pointer<Command> >& getChannel(const Pointer<Command>& dispatch);

  Pointer<Command> removeFirst();

  Pointer<Command> getFirst() const;
};
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_CORE_SIMPLEPRIORITYMESSAGEDISPATCHCHANNEL_H_ */
