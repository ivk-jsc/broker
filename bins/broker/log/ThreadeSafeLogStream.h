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

#ifndef THREADESAFELOGSTREAM_H
#define THREADESAFELOGSTREAM_H
#include <Poco/LogStream.h>
#include <Poco/Message.h>
#include <Poco/Thread.h>
#include <Poco/RWLock.h>

#include <unordered_map>
#include <string>
#include <iosfwd>
#include <mutex>
#include "MoveableRWLock.h"

// Forward Declare
namespace Poco {
class Logger;
}

namespace upmq {
namespace broker {

class ThreadSafeLogStreamBuf : public Poco::LogStreamBuf {
 public:
  /// Constructor
  ThreadSafeLogStreamBuf(Poco::Logger &logger, Poco::Message::Priority priority);

 public:
  int overflow(char c);
  using Poco::LogStreamBuf::overflow;

 private:
  /// Overridden fron base to write to the device in a thread-safe manner.
  int writeToDevice(char c) override;
  int writeToDeviceImpl(std::string &item, char c);

 private:
  /// Store a map of thread indices to messages
  std::unordered_map<Poco::Thread::TID, std::string> m_messages;
  /// mutex protecting logstream
  upmq::MRWLock _rwlock;
};

class ThreadSafeLogIOS : public virtual std::ios {
 public:
  /// Constructor
  ThreadSafeLogIOS(Poco::Logger &logger, Poco::Message::Priority priority);
  // Return a pointer to the stream buffer object
  Poco::LogStreamBuf *rdbuf();

 protected:
  /// The log stream buffer object
  ThreadSafeLogStreamBuf m_buf;
};

class ThreadSafeLogStream : public ThreadSafeLogIOS, public std::ostream {
 public:
  /// Creates the ThreadSafeLogStream, using the given logger and priority.
  explicit ThreadSafeLogStream(Poco::Logger &logger, Poco::Message::Priority priority = Poco::Message::PRIO_INFORMATION);

  /// Creates the ThreadSafeLogStream, using the logger identified
  /// by loggerName, and sets the priority.
  explicit ThreadSafeLogStream(const std::string &loggerName, Poco::Message::Priority priority = Poco::Message::PRIO_INFORMATION);
  /// Sets the priority for log messages to Poco::Message::PRIO_FATAL.
  ThreadSafeLogStream &fatal();
  /// Sets the priority for log messages to Poco::Message::PRIO_FATAL
  /// and writes the given message.
  ThreadSafeLogStream &fatal(const std::string &message);
  /// Sets the priority for log messages to Poco::Message::PRIO_CRITICAL.
  ThreadSafeLogStream &critical();
  /// Sets the priority for log messages to Poco::Message::PRIO_CRITICAL
  /// and writes the given message.
  ThreadSafeLogStream &critical(const std::string &message);
  /// Sets the priority for log messages to Poco::Message::PRIO_ERROR.
  ThreadSafeLogStream &error();
  /// Sets the priority for log messages to Poco::Message::PRIO_ERROR
  /// and writes the given message.
  ThreadSafeLogStream &error(const std::string &message);
  /// Sets the priority for log messages to Poco::Message::PRIO_WARNING.
  ThreadSafeLogStream &warning();
  /// Sets the priority for log messages to Poco::Message::PRIO_WARNING
  /// and writes the given message.
  ThreadSafeLogStream &warning(const std::string &message);
  /// Sets the priority for log messages to Poco::Message::PRIO_NOTICE.
  ThreadSafeLogStream &notice();
  /// Sets the priority for log messages to Poco::Message::PRIO_NOTICE
  /// and writes the given message.
  ThreadSafeLogStream &notice(const std::string &message);
  /// Sets the priority for log messages to Poco::Message::PRIO_INFORMATION.
  ThreadSafeLogStream &information();
  /// Sets the priority for log messages to Poco::Message::PRIO_INFORMATION
  /// and writes the given message.
  ThreadSafeLogStream &information(const std::string &message);
  /// Sets the priority for log messages to Poco::Message::PRIO_DEBUG.
  ThreadSafeLogStream &debug();
  /// Sets the priority for log messages to Poco::Message::PRIO_DEBUG
  /// and writes the given message.
  ThreadSafeLogStream &debug(const std::string &message);
  /// Sets the priority for log messages to Poco::Message::PRIO_TRACE.
  ThreadSafeLogStream &trace();
  /// Sets the priority for log messages to Poco::Message::PRIO_TRACE
  /// and writes the given message.
  ThreadSafeLogStream &trace(const std::string &message);
  /// Sets the priority for log messages.
  ThreadSafeLogStream &priority(Poco::Message::Priority priority);
};
}  // namespace broker
}  // namespace upmq

void ASYNCLOG_FATAL(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

void ASYNCLOG_CRITICAL(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

void ASYNCLOG_ERROR(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

void ASYNCLOG_WARNING(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

void ASYNCLOG_NOTICE(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

void ASYNCLOG_INFORMATION(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

void ASYNCLOG_INFORMATION(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const char &__msg__);

void ASYNCLOG_DEBUG(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

void ASYNCLOG_TRACE(std::unique_ptr<upmq::broker::ThreadSafeLogStream> &__logger__, const std::string &__msg__);

#endif /* THREADESAFELOGSTREAM_H */
