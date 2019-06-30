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

#ifndef _WIN32
#include <sys/resource.h>
#endif

#include <AsyncHandlerRegestry.h>
#include <Poco/DateTime.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <cstdlib>
#include <fake_cpp14.h>
#include <Poco/File.h>
#include "About.h"
#include "Broker.h"
#include "Exception.h"
#include "Exchange.h"
#include "MainApplication.h"
#include "Version.hpp"

#ifdef ENABLE_WEB_ADMIN
#include "WebAdminRequestHandlerFactory.h"
#endif

namespace upmq {
namespace broker {

MainApplication::MainApplication() : swf::Application("brkr"), _helpRequested(false), _versionRequested(false), _name("broker"), _port(12345) {}

void MainApplication::defineOptions(Poco::Util::OptionSet &options) {
  ServerApplication::defineOptions(options);

  options.addOption(Poco::Util::Option("help", "h", "display help information on command line arguments").required(false).repeatable(false));

  options.addOption(
      Poco::Util::Option("version", "v", "display version information").required(false).repeatable(false).callback(Poco::Util::OptionCallback<MainApplication>(this, &MainApplication::handleVersion)));

  options.addOption(Poco::Util::Option("fileconfig", "f", "path to config file")
                        .required(false)
                        .repeatable(false)
                        .argument("file path")
                        .callback(Poco::Util::OptionCallback<MainApplication>(this, &MainApplication::handleFile)));
}

void MainApplication::handleOption(const std::string &name, const std::string &value) {
  if (name == "help") {
    _helpRequested = true;
  } else if (name == "version") {
    handleVersion(name, value);
  } else if (name == "fileconfig") {
    handleFile(name, value);
  } else {
    ServerApplication::handleOption(name, value);
  }
}

void MainApplication::displayHelp() {
  Poco::Util::HelpFormatter helpFormatter(options());
  helpFormatter.setCommand(commandName());
  helpFormatter.setUsage("OPTIONS");
  helpFormatter.setHeader("MQ server - queue broker.");
  helpFormatter.format(std::cout);
  stopOptionsProcessing();
}

int MainApplication::main(const std::vector<std::string> &args) {
  UNUSED_VAR(args);

  if (_helpRequested) {
    displayHelp();
    return Application::EXIT_OK;
  }
  if (_versionRequested) {
    return Application::EXIT_OK;
  }

  loadBrokerConfiguration();

  // TODO : wrap with macro ifdef..
  if (useUpiter()) {
    std::string LM = getLogAddr();
    LM[2] = '$';
    LM[3] = '$';
    CONFIGURATION::Instance().setName(LM);
  }

  try {
    EXCHANGE::Instance().start();
    BROKER::Instance().start();
  } catch (Exception &ex) {
    ASYNCLOG_CRITICAL(logStream, (ex.message() += non_std_endl));
    return Application::EXIT_CANTCREAT;
  }

#ifdef ENABLE_WEB_ADMIN
  Poco::Net::HTTPServer s(new WebAdminRequestHandlerFactory, ServerSocket(static_cast<Poco::UInt16>(CONFIGURATION::Instance().http().port)), new Poco::Net::HTTPServerParams);
  Poco::File wwwDir(CONFIGURATION::Instance().http().site);
  if (wwwDir.exists()) {
    s.start();
  }

#endif

  ASYNCLOG_CRITICAL(logStream, (std::string("-").append(" * ").append("<<========= start =========>>") += non_std_endl));
  ASYNCLOG_CRITICAL(logStream, (std::string("-").append(" * ").append("version\t\t\t: ").append(About::version()) += non_std_endl));
  ASYNCLOG_CRITICAL(logStream, (std::string("-").append(" * ").append("configuration\t\t=> ").append(CONFIGURATION::Instance().toString()) += non_std_endl));

  std::string webuiStatus = "disabled";
#ifdef ENABLE_WEB_ADMIN
  if (wwwDir.exists()) {
    webuiStatus = "enabled";
  }
#endif
  ASYNCLOG_CRITICAL(logStream, (std::string("-").append(" * ").append("webui\t\t: ").append(webuiStatus) += non_std_endl));
  // TODO(bas) : refactor this
  Poco::Util::AbstractConfiguration::Keys destinationsKeys;
  std::string destinationsSection = "broker.destinations";
  config().keys(destinationsSection, destinationsKeys);
  for (const auto &destItm : destinationsKeys) {
    if (destItm == "autocreate" || destItm == "forward") {
      continue;
    }
    std::string uriKeyType;
    std::string uriKeyName;
    uriKeyType.append(destinationsSection).append(".").append(destItm).append("[@type]");
    uriKeyName.append(destinationsSection).append(".").append(destItm).append("[@name]");
    std::string uri = config().getString(uriKeyType) + "://" + config().getString(uriKeyName);
    Destination &dest = EXCHANGE::Instance().destination(uri);
    Poco::Util::AbstractConfiguration::Keys subscribersKeys;
    std::string subscribers;
    subscribers.append(destinationsSection).append(".").append(destItm).append(".bind.clients.subscribers");
    config().keys(subscribers, subscribersKeys);
    for (const auto &subscriber : subscribersKeys) {
      std::string subscriberKey;
      subscriberKey.append(subscribers).append(".").append(subscriber);
      dest.bindWithSubscriber(config().getString(subscriberKey), config().getBool(subscriberKey + "[@use-file-link]", false));
    }
    Poco::Util::AbstractConfiguration::Keys publishersKeys;
    std::string publishers;
    publishers.append(destinationsSection).append(".").append(destItm).append(".bind.clients.publishers");
    config().keys(publishers, publishersKeys);
    for (const auto &publisher : publishersKeys) {
      std::string publisherKey;
      publisherKey.append(publishers).append(".").append(publisher);
      dest.bindWithPublisher(config().getString(publisherKey), config().getBool(publisherKey + "[@use-file-link]", false));
    }
  }

  AHRegestry::Instance().start();
  ServerSocket svs(static_cast<Poco::UInt16>(CONFIGURATION::Instance().port()), CONFIGURATION::Instance().net().maxConnections);
  svs.setReusePort(false);
  svs.setReuseAddress(false);

  Poco::Net::SocketReactor reactor;
  Poco::Net::ParallelSocketAcceptor<AsyncTCPHandler, Poco::Net::SocketReactor> acceptor(svs, reactor, CONFIGURATION::Instance().threads().accepters);

  Thread thread;
  thread.start(reactor);

  //            #ifdef _DEBUG
  //                  while (true) {
  //                    Poco::Thread::sleep(100000);
  //                  }
  //            #else
  waitTermination();
  //            #endif

#ifdef ENABLE_WEB_ADMIN
  s.stop();
#endif

  reactor.stop();
  thread.join();
  BROKER::Instance().stop();
  EXCHANGE::Instance().stop();
  AHRegestry::Instance().stop();

  ASYNCLOG_CRITICAL(logStream, (std::string("-").append(" * ").append(">>========= stop =========<<") += non_std_endl));
  logStream.reset(nullptr);
  return Application::EXIT_OK;
}

void MainApplication::loadBrokerConfiguration() {
  CONFIGURATION::Instance().setPort(config().getInt("broker.port", _port));
  if ((CONFIGURATION::Instance().name() == "broker") || CONFIGURATION::Instance().name().empty()) {
    CONFIGURATION::Instance().setName(config().getString("broker.name", _name));
  }

  loadHttpConfig();

  loadHeartBeatConfig();

  loadNetConfig();

  loadThreadsConfig();

  loadLogConfig();

  loadDestinationConfig();

  loadStorageConfig();
}
void MainApplication::loadStorageConfig() const {
  Configuration::Storage storage;
  storage.connection.props.dbmsType = Configuration::Storage::type(config().getString("broker.storage.connection[@dbms]", "sqlite-native"));
  storage.connection.props.connectionPool = config().getInt("broker.storage.connection[@pool]", storage.connection.props.connectionPool);
  storage.connection.props.useSync = config().getBool("broker.storage.connection[@sync]", storage.connection.props.useSync);
  storage.connection.props.journalMode = config().getString("broker.storage.connection[@journal-mode]", storage.connection.props.journalMode);

  storage.connection.value.usePath = config().getBool("broker.storage.connection.value[@use-path]", storage.connection.value.usePath);
  storage.connection.value.set(config().getString("broker.storage.connection.value", storage.connection.value.get()));

  Poco::Path prefix;
#ifdef _WIN32
  prefix = config().getString("broker.storage.connection.path[@windows]", "C:/ProgramData");
#else
  prefix = config().getString("broker.storage.connection.path[@_nix]", "../share");
#endif
  storage.connection.path.assign(prefix).makeDirectory();
  storage.connection.path.append(config().getString("broker.storage.connection.path", "upmq/db")).makeDirectory();

  prefix.clear();
#ifdef _WIN32
  prefix = config().getString("broker.storage.data[@windows]", "C:/ProgramData");
#else
  prefix = config().getString("broker.storage.data[@_nix]", "../share");
#endif
  storage.data.set(prefix.append(config().getString("broker.storage.data", "upmq/data")).toString());

  storage.setMessageJournal(CONFIGURATION::Instance().name());
  CONFIGURATION::Instance().setStorage(storage);
}
void MainApplication::loadDestinationConfig() const {
  Configuration::Destinations destinations;
  destinations.autocreate = config().getBool("broker.destinations.autocreate", destinations.autocreate);
  destinations.forwardByProperty = config().getBool("broker.destinations.forward[@by-property]", destinations.forwardByProperty);
  CONFIGURATION::Instance().setDestinations(destinations);
}
void MainApplication::loadLogConfig() {
  Configuration::Log log;
  log.level = config().getInt("broker.log.level", log.level) % 9;
  log.isInteractive = config().getBool("broker.log.interactive", log.isInteractive);
  log.name = CONFIGURATION::Instance().name();
  Poco::Path prefix;
#ifdef _WIN32
  prefix = config().getString("broker.log.path[@windows]", "C:/ProgramData");
#else
  prefix = config().getString("broker.log.path[@_nix]", "/var/log");
#endif
  log.path.assign(prefix);
  log.path.append(config().getString("broker.log.path", "upmq/log")).makeDirectory();
  CONFIGURATION::Instance().setLog(log);

  ASYNCLOGGER::Instance().logPriority = log.level;

  if (config().getBool("application.runAsDaemon", false)) {
    ASYNCLOGGER::Instance().isInteractive = false;
  } else {
    ASYNCLOGGER::Instance().isInteractive = log.isInteractive;
  }

  logStream = std::make_unique<ThreadSafeLogStream>(ASYNCLOGGER::Instance().add(log.name, log.path.toString()));
}
void MainApplication::loadThreadsConfig() const {
  Configuration::Threads threads;
  const auto procCount = Poco::Environment::processorCount();
  threads.readers = config().getUInt("broker.threads.reader", procCount);
  threads.writers = config().getUInt("broker.threads.writer", procCount);
  threads.accepters = config().getUInt("broker.threads.accepter", procCount);
  threads.subscribers = config().getUInt("broker.threads.subscriber", procCount);
  CONFIGURATION::Instance().setThreads(threads);
}
void MainApplication::loadNetConfig() const {
  Configuration::Net net;
  net.maxConnections = config().getInt("broker.net.max-connections", net.maxConnections);
  CONFIGURATION::Instance().setNet(net);
}
void MainApplication::loadHeartBeatConfig() const {
  Configuration::HeartBeat heartBeat;
  heartBeat.sendTimeout = config().getInt("broker.heartbeat.send", 0);
  heartBeat.recvTimeout = config().getInt("broker.heartbeat.recv", 0);
  CONFIGURATION::Instance().setHeartbeat(heartBeat);
}
void MainApplication::loadHttpConfig() const {
  Configuration::Http http;
  http.port = config().getInt("broker.http.port", http.port);
  // TODO : verify path by app-bin path
  http.site = config().getString("broker.http.site", "../share/upmq/www");
  CONFIGURATION::Instance().setHttp(http);
}

void MainApplication::handleVersion(const std::string &name, const std::string &value) {
  UNUSED_VAR(value);
  if (name == "version") {
    _versionRequested = true;
    std::cout << "MQ server - queue broker - version : " << About::version() << non_std_endl;
    std::cout << "MQ server - queue broker - commit  : " << About::commit() << non_std_endl;
    stopOptionsProcessing();
  }
}
void MainApplication::handleFile(const std::string &name, const std::string &value) {
  UNUSED_VAR(name);
  Poco::Path configFilePath(value);
  if ((configFilePath.getExtension() == "xml")) {
    loadConfiguration(value);
  }
}
}  // namespace broker
}  // namespace upmq