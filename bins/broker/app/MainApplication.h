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

#ifndef SERVER_H
#define SERVER_H

#include "AsyncLogger.h"
#include "AsyncTCPHandler.h"
#include "Configuration.h"
#include "Defines.h"

#include <Poco/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/NObserver.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/ServerSocket.h>
#include "ReactorHeaders.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Notification.h>
#include <Poco/NotificationCenter.h>
#include <Poco/PatternFormatter.h>
#include <Poco/Thread.h>
#include <Poco/UUID.h>
#include <Poco/Logger.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include "SWFApplication.h"
#include <Poco/Util/XMLConfiguration.h>

#include <iostream>
#include <memory>

namespace upmq {
namespace broker {

class MainApplication : public swf::Application {
 public:
  MainApplication();
  ~MainApplication() override = default;
  Poco::Logger *log{nullptr};

 protected:
  void defineOptions(Poco::Util::OptionSet &options) override;
  void handleOption(const std::string &name, const std::string &value) override;
  void handleVersion(const std::string &name, const std::string &value);
  void handleFile(const std::string &name, const std::string &value);
  void displayHelp();
  int main(const std::vector<std::string> &args) override;

 private:
  bool _helpRequested;
  bool _versionRequested;
  std::string _name;
  int32_t _port;

  void loadBrokerConfiguration();
  void loadHttpConfig() const;
  void loadHeartBeatConfig() const;
  void loadNetConfig() const;
  void loadSessionsConfig() const;
  void loadSubscriptionsConfig() const;
  void loadThreadsConfig() const;
  void loadLogConfig();
  void loadDestinationConfig() const;
  void loadStorageConfig() const;
};
}  // namespace broker
}  // namespace upmq

#endif  // SERVER_H
