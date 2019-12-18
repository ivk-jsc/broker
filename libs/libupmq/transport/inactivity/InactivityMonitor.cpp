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

#include "InactivityMonitor.h"

#include <threads/CompositeTask.h>
#include <threads/CompositeTaskRunner.h>
#include <transport/UPMQCommand.h>
#include <transport/inactivity/WriteChecker.h>

#include <decaf/lang/Boolean.h>
#include <decaf/lang/Math.h>
#include <decaf/lang/Runnable.h>
#include <decaf/lang/Thread.h>
#include <decaf/util/Timer.h>
#include <decaf/util/concurrent/atomic/AtomicBoolean.h>
#include <decaf/util/concurrent/atomic/AtomicInteger.h>
#include <utility>

using namespace std;
using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::inactivity;
using namespace upmq::threads;
using namespace decaf;
using namespace decaf::io;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace decaf::util::concurrent::atomic;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
namespace upmq {
namespace transport {
namespace inactivity {

class InactivityMonitorData {
 public:
  InactivityMonitorData(const InactivityMonitorData &) = delete;
  InactivityMonitorData operator=(const InactivityMonitorData &) = delete;

  Pointer<WriteChecker> writeCheckerTask;

  Timer readCheckTimer;
  Timer writeCheckTimer;

  Pointer<CompositeTaskRunner> asyncTasks;
  Pointer<AsyncWriteTask> asyncWriteTask;

  AtomicBoolean monitorStarted;

  AtomicBoolean commandSent;
  AtomicBoolean commandReceived;

  AtomicBoolean failed;
  AtomicBoolean inRead;
  AtomicBoolean inWrite;

  Mutex inWriteMutex;
  Mutex monitor;

  long long readCheckTime;
  long long writeCheckTime;
  long long initialDelayTime;

  InactivityMonitorData(Pointer<WireFormat> wireFormat, long long delay, long long period)
      : writeCheckerTask(),
        readCheckTimer("InactivityMonitor Read Check Timer"),
        writeCheckTimer("InactivityMonitor Write Check Timer"),
        asyncTasks(),
        asyncWriteTask(),
        monitorStarted(),
        commandSent(),
        commandReceived(true),
        failed(),
        inRead(),
        inWrite(),
        inWriteMutex(),
        monitor(),
        readCheckTime(period),
        writeCheckTime(period),
        initialDelayTime(delay) {
    DECAF_UNUSED_VAR(wireFormat);
  }
};

// Task that fires when the TaskRunner is signaled by the WriteCheck Timer Task.
class AsyncWriteTask : public CompositeTask {
 private:
  InactivityMonitor *parent;
  AtomicBoolean write;

 private:
  AsyncWriteTask(const AsyncWriteTask &);
  AsyncWriteTask operator=(const AsyncWriteTask &);

 public:
  AsyncWriteTask(InactivityMonitor *parent_) : parent(parent_) {}

  void setWrite(bool newWrite) { this->write.set(newWrite); }

  bool isPending() const override { return this->write.get(); }

  bool iterate() override {
    if (this->write.compareAndSet(true, false) && this->parent->members->monitorStarted.get()) {
      try {
        Pointer<UPMQCommand> request(new UPMQCommand());
        request->getProtoMessage().set_object_id("");
        if (!request->getPing().IsInitialized()) {
          throw cms::CMSException("request not initialized");
        }

        this->parent->oneway(request.dynamicCast<Command>());
      } catch (IOException &e) {
        this->parent->onException(e);
      }
    }

    return this->write.get();
  }
};
}  // namespace inactivity
}  // namespace transport
}  // namespace upmq

////////////////////////////////////////////////////////////////////////////////
InactivityMonitor::InactivityMonitor(Pointer<Transport> next_, Pointer<transport::WireFormat> wireFormat, long long delay, long long period)
    : TransportFilter(std::move(next_)), members(new InactivityMonitorData(std::move(wireFormat), delay, period)) {}

////////////////////////////////////////////////////////////////////////////////
InactivityMonitor::InactivityMonitor(
    Pointer<Transport> next_, const decaf::util::Properties &properties, Pointer<transport::WireFormat> wireFormat, long long delay, long long period)
    : TransportFilter(std::move(next_)), members(new InactivityMonitorData(std::move(wireFormat), delay, period)) {
  DECAF_UNUSED_VAR(properties);
  // this->members->keepAliveResponseRequired =
  // Boolean::parseBoolean(properties.getProperty("keepAliveResponseRequired", "false"));
}

////////////////////////////////////////////////////////////////////////////////
InactivityMonitor::~InactivityMonitor() {
  try {
    this->stopMonitorThreads();
  }
  AMQ_CATCHALL_NOTHROW()

  try {
    delete this->members;
  }
  AMQ_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
long long InactivityMonitor::getWriteCheckTime() const { return this->members->writeCheckTime; }

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::setWriteCheckTime(long long value) { this->members->writeCheckTime = value; }

////////////////////////////////////////////////////////////////////////////////
long long InactivityMonitor::getInitialDelayTime() const { return this->members->initialDelayTime; }

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::setInitialDelayTime(long long value) const { this->members->initialDelayTime = value; }

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::afterNextIsStarted() {
  try {
    startMonitorThreads();
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::beforeNextIsStopped() {
  try {
    stopMonitorThreads();
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::doClose() {
  try {
    stopMonitorThreads();
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::onException(const decaf::lang::Exception &ex) {
  if (this->members->failed.compareAndSet(false, true)) {
    stopMonitorThreads();
    TransportFilter::onException(ex);
  }
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::onCommand(Pointer<Command> command) {
  this->members->commandReceived.set(true);
  this->members->inRead.set(true);

  try {
    TransportFilter::onCommand(std::move(command));

    this->members->inRead.set(false);
  } catch (Exception &ex) {
    this->members->inRead.set(false);
    ex.setMark(__FILE__, __LINE__);
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::oneway(Pointer<Command> command) {
  try {
    synchronized(&this->members->inWriteMutex) {
      this->members->inWrite.set(true);
      try {
        if (this->members->failed.get()) {
          throw IOException(__FILE__, __LINE__, (std::string("Channel was inactive for too long: ") + next->getRemoteAddress()).c_str());
        }

        this->next->oneway(std::move(command));

        this->members->commandSent.set(true);
        this->members->inWrite.set(false);

      } catch (Exception &ex) {
        this->members->commandSent.set(true);
        this->members->inWrite.set(false);
        ex.setMark(__FILE__, __LINE__);
        throw;
      }
    }
  }
  AMQ_CATCH_RETHROW(IOException)
  AMQ_CATCH_RETHROW(UnsupportedOperationException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, IOException)
  AMQ_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::readCheck() {
  if (this->members->inRead.get()) {
    return;
  }

  if (!this->members->commandReceived.get()) {
    this->members->asyncTasks->wakeup();
  }

  this->members->commandReceived.set(false);
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::writeCheck() {
  if (this->members->inWrite.get()) {
    return;
  }

  if (!this->members->commandSent.get()) {
    this->members->asyncWriteTask->setWrite(true);
    this->members->asyncTasks->wakeup();
  }

  this->members->commandSent.set(false);
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::startMonitorThreads() {
  if (this->members->monitorStarted.get()) {
    return;
  }

  synchronized(&this->members->monitor) {
    this->members->asyncTasks.reset(new CompositeTaskRunner());
    this->members->asyncWriteTask.reset(new AsyncWriteTask(this));

    this->members->asyncTasks->addTask(this->members->asyncWriteTask.get());
    this->members->asyncTasks->start();

    this->members->monitorStarted.set(true);
    this->members->writeCheckerTask.reset(new WriteChecker(this));

    this->members->writeCheckTimer.scheduleAtFixedRate(
        this->members->writeCheckerTask.dynamicCast<TimerTask>(), this->members->initialDelayTime, this->members->writeCheckTime);
  }
}

////////////////////////////////////////////////////////////////////////////////
void InactivityMonitor::stopMonitorThreads() {
  if (this->members->monitorStarted.compareAndSet(true, false)) {
    synchronized(&this->members->monitor) {
      this->members->writeCheckerTask->cancel();

      this->members->readCheckTimer.purge();
      this->members->readCheckTimer.cancel();
      this->members->writeCheckTimer.purge();
      this->members->writeCheckTimer.cancel();

      this->members->asyncTasks->shutdown();
    }
  }
}
