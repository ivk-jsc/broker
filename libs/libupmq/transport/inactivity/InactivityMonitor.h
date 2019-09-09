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

#ifndef _UPMQ_TRANSPORT_INACTIVITY_INACTIVITYMONITOR_H_
#define _UPMQ_TRANSPORT_INACTIVITY_INACTIVITYMONITOR_H_

#include <transport/Config.h>

#include <transport/Command.h>
#include <transport/TransportFilter.h>
#include <transport/UPMQWireFormat.h>

#include <decaf/lang/Pointer.h>
#include <decaf/util/Properties.h>

namespace upmq {
namespace transport {
namespace inactivity {

using decaf::lang::Pointer;

class WriteChecker;
class AsyncWriteTask;
class InactivityMonitorData;

class UPMQCPP_API InactivityMonitor : public upmq::transport::TransportFilter {
 private:
  // Internal Class used to house the data structures for this object
  InactivityMonitorData *members;

  friend class WriteChecker;
  friend class AsyncWriteTask;

public:
  InactivityMonitor(const InactivityMonitor &) = delete;
  InactivityMonitor operator=(const InactivityMonitor &) = delete;

  InactivityMonitor(Pointer<Transport> next, Pointer<transport::WireFormat> wireFormat, long long delay, long long period);
  InactivityMonitor(Pointer<Transport> next,
                    const decaf::util::Properties &properties,
                    Pointer<transport::WireFormat> wireFormat,
                    long long delay,
                    long long period);

  ~InactivityMonitor() override;

 public:  // TransportFilter Methods
  void onException(const decaf::lang::Exception &ex) override;

  void onCommand(Pointer<Command> command) override;

  void oneway(Pointer<Command> command) override;

 public:
  long long getWriteCheckTime() const;

  void setWriteCheckTime(long long value);

  long long getInitialDelayTime() const;

  void setInitialDelayTime(long long value) const;

 protected:
  void afterNextIsStarted() override;

  void beforeNextIsStopped() override;

  void doClose() override;

 private:
  // Performs a Read Check on the current connection, called from a separate Thread.
  void readCheck();

  // Perform a Write Check on the current connection, called from a separate Thread.
  void writeCheck();

  // Stops all the monitoring Threads, cannot restart once called.
  void stopMonitorThreads();

  // Starts the monitoring Threads,
  void startMonitorThreads();
};
}  // namespace inactivity
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_TRANSPORT_INACTIVITY_INACTIVITYMONITOR_H_ */
