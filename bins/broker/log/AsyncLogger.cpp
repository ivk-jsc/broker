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

#include "AsyncLogger.h"
#include <iostream>
#include <Poco/File.h>
#include "Defines.h"
#include <Poco/Util/Application.h>
#include <poco_pointers_helper.h>
namespace upmq {
namespace broker {

constexpr char DEFAULT_PATTERN[] = "%Y-%m-%d %H:%M:%S:%i %t";

AutoPtr<FormattingChannel> AsyncLogger::createFormatter(const std::string &name, bool interactive) {
#if !defined(WIN32) && !defined(WIN32)
  AutoPtr<ColorConsoleChannel> colorChannel;
#else
  AutoPtr<WindowsColorConsoleChannel> colorChannel;
#endif

  AutoPtr<SplitterChannel> splitter = Poco::MakeAuto<SplitterChannel>();
  if (interactive) {
#if defined(POCO_OS_FAMILY_UNIX)  // POCO_OS == POCO_OS_LINUX
    colorChannel = Poco::MakeAuto<Poco::ColorConsoleChannel>();
#elif defined(POCO_OS_FAMILY_WINDOWS)  // POCO_OS == POCO_OS POCO_OS_WINDOWS_NT
    colorChannel = Poco::MakeAuto<Poco::WindowsColorConsoleChannel>();
#endif
    colorChannel->setProperty("traceColor", "gray");
    colorChannel->setProperty("debugColor", "brown");
    colorChannel->setProperty("informationColor", "green");
    colorChannel->setProperty("noticeColor", "blue");
    colorChannel->setProperty("warningColor", "lightMagenta");
    colorChannel->setProperty("errorColor", "magenta");
    colorChannel->setProperty("criticalColor", "lightRed");
    colorChannel->setProperty("fatalColor", "red");
    AutoPtr<Poco::AsyncChannel> colorAsyncChannel = Poco::MakeAuto<Poco::AsyncChannel>(colorChannel);
    colorAsyncChannel->setProperty("priority", "lowest");
    splitter->addChannel(colorAsyncChannel);
  }
  AutoPtr<FileChannel> fileChannel = Poco::MakeAuto<FileChannel>(name + ".log");
  AutoPtr<Poco::AsyncChannel> fileAsyncChannel = Poco::MakeAuto<Poco::AsyncChannel>(fileChannel);

  fileChannel->setProperty("rotation", "10 M");
  fileChannel->setProperty("times", "local");
  fileAsyncChannel->setProperty("priority", "lowest");
  splitter->addChannel(fileAsyncChannel);

  AutoPtr<PatternFormatter> patternFormatter = Poco::MakeAuto<PatternFormatter>(DEFAULT_PATTERN);

  AutoPtr<FormattingChannel> formattingChannel = Poco::MakeAuto<FormattingChannel>(patternFormatter);
  formattingChannel->setChannel(splitter);
  return formattingChannel;
}

Poco::Logger &AsyncLogger::get(const std::string &name) { return Logger::get(name); }

Poco::Logger &AsyncLogger::get(const Poco::Path &name) { return Logger::get(name.toString()); }

Logger &AsyncLogger::add(const std::string &name, const std::string &subdir) {
  try {
    Poco::ScopedLock<Poco::FastMutex> lock(logLock);
    const Poco::Logger *emptyLoggerPtr = nullptr;
#if POCO_VERSION_MAJOR > 1 || (POCO_VERSION_MAJOR == 1 && POCO_VERSION_MINOR > 9)
    Poco::AutoPtr<Poco::Logger>
#else
    Poco::Logger *
#endif
        loggetPtr = Logger::has(name);
    if (loggetPtr == emptyLoggerPtr) {
      Poco::Path dirpath;
      dirpath.parse(subdir);
      if (isInteractive || !Poco::Util::Application::instance().config().getBool("application.runAsDaemon", false)) {
        dirpath.makeAbsolute();
      }
      dirpath.append(name).makeFile();
      Poco::File f(dirpath.parent());
      if (!f.exists()) {
        f.createDirectories();
      }
      _formattingChannel = createFormatter(dirpath.toString(), isInteractive);
      return Logger::create(name, _formattingChannel, logPriority);
    }
    return *loggetPtr;
  } catch (Poco::Exception &ex) {
    std::cerr << ex.displayText() << non_std_endl;
  }
  return get(name);
}

void AsyncLogger::destroy(const std::string &name) {
  Poco::ScopedLock<Poco::FastMutex> lock(logLock);
  Logger::destroy(name);
}

void AsyncLogger::destroy(const Poco::Path &name) {
  Poco::ScopedLock<Poco::FastMutex> lock(logLock);
  Logger::destroy(name.toString());
}

void AsyncLogger::remove(const std::string &name, const std::string &subdir) {
  destroy(name);
  Poco::Path dirpath;
  dirpath.append(subdir).makeAbsolute().append(name).makeFile().setExtension("log");
  Poco::File f(dirpath);
  if (f.exists()) {
    f.remove();
  }
}

bool AsyncLogger::exists(const std::string &name) {
  Poco::AutoPtr<Poco::Logger> loggetPtr = Logger::has(name);
  return loggetPtr.isNull();
}

}  // namespace broker
}  // namespace upmq
