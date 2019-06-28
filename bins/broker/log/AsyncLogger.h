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

#ifndef BROKERSTORAGELOGGER_H
#define BROKERSTORAGELOGGER_H

#include <Poco/Logger.h>
#include <Poco/FileChannel.h>
#include <Poco/AsyncChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/AutoPtr.h>
#include <Poco/Path.h>

#ifdef POCO_OS_FAMILY_UNIX
#include <Poco/SyslogChannel.h>
#endif

#ifdef POCO_OS_FAMILY_WINDOWS
#include <Poco/EventLogChannel.h>
#include <Poco/WindowsConsoleChannel.h>
#endif

#include "Singleton.h"
#include <memory>

using Poco::SplitterChannel;
#ifdef POCO_OS_FAMILY_WINDOWS
using Poco::EventLogChannel;
using Poco::WindowsColorConsoleChannel;
using Poco::WindowsConsoleChannel;
#else
using Poco::ColorConsoleChannel;
using Poco::ConsoleChannel;
using Poco::SyslogChannel;
#endif
using Poco::AsyncChannel;
using Poco::AutoPtr;
using Poco::FileChannel;
using Poco::FormattingChannel;
using Poco::Logger;
using Poco::PatternFormatter;

namespace upmq {
namespace broker {

class AsyncLogger {
  AutoPtr<FormattingChannel> _formattingChannel;

 public:
  AsyncLogger() = default;

  static Logger &get(const std::string &name);

  static Logger &get(const Poco::Path &name);

  Logger &add(const std::string &name, const std::string &subdir = ".");

  void destroy(const std::string &name);

  void destroy(const Poco::Path &name);

  void remove(const std::string &name, const std::string &subdir = ".");

  bool isInteractive = true;

  int logPriority = Poco::Message::PRIO_TRACE;

  Poco::FastMutex logLock;

  static bool exists(const std::string &name);

  static AutoPtr<FormattingChannel> createFormatter(const std::string &name, bool interactive);
};

}  // namespace broker
}  // namespace upmq
using ASYNCLOGGER = Singleton<upmq::broker::AsyncLogger>;

#endif  // BROKERSTORAGELOGGER_H
