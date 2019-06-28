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

 private:
  InactivityMonitor(const InactivityMonitor &);
  InactivityMonitor operator=(const InactivityMonitor &);

 public:
  InactivityMonitor(const Pointer<Transport> next, const Pointer<transport::WireFormat> wireFormat, long long delay, long long period);
  InactivityMonitor(const Pointer<Transport> next, const decaf::util::Properties &properties, const Pointer<transport::WireFormat> wireFormat, long long delay, long long period);

  virtual ~InactivityMonitor();

 public:  // TransportFilter Methods
  virtual void onException(const decaf::lang::Exception &ex);

  virtual void onCommand(const Pointer<Command> command);

  virtual void oneway(const Pointer<Command> command);

 public:
  long long getWriteCheckTime() const;

  void setWriteCheckTime(long long value);

  long long getInitialDelayTime() const;

  void setInitialDelayTime(long long value) const;

 protected:
  virtual void afterNextIsStarted();

  virtual void beforeNextIsStopped();

  virtual void doClose();

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
