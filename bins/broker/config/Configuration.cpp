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

#include "Configuration.h"
#include <Poco/File.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <algorithm>

namespace upmq {
namespace broker {

storage::DBMSType Configuration::Storage::type(const std::string &dbms) {
  storage::DBMSType regestryDBMSType = storage::NO_TYPE;
  std::string localDBMS = dbms;
  auto ctolower = [](int c) -> char { return static_cast<char>(::tolower(c)); };
  std::transform(localDBMS.begin(), localDBMS.end(), localDBMS.begin(), ctolower);
  if (localDBMS == "postgresql") {
    regestryDBMSType = storage::Postgresql;
  } else if (localDBMS == "sqlite") {
    regestryDBMSType = storage::SQLite;
  } else if ((localDBMS == "sqlite-native") || (localDBMS == ":memory:")) {
    regestryDBMSType = storage::SQLiteNative;
  }
  return regestryDBMSType;
}
std::string Configuration::Storage::typeName(storage::DBMSType dbmsType) {
  switch (static_cast<int>(dbmsType)) {
    case storage::Postgresql:
      return "Postgresql";
    case storage::SQLite:
      return "SQLite";
    case storage::SQLiteNative:
      return "SQLite-Native";
    default:
      return "NO TYPE";
  }
}
std::string Configuration::Storage::toString() const {
  return std::string("\n- * \t\tconnection\t=> ")
      .append(connection.toString())
      .append("\n- * \t\tdata\t=>")
      .append(data.toString())
      .append("\n- * \t\tjournal\t: ")
      .append(_messageJournal)
      .append("\n- * \t\tmessages\t: ")
      .append(messages.toString());
}
std::string Configuration::Storage::messageJournal(const std::string &destinationName) const {
  return std::string("\"").append(_messageJournal).append("/").append(destinationName).append("\"");
}
void Configuration::Storage::setMessageJournal(const std::string &brokerName) { _messageJournal = brokerName + "_journal"; }
Configuration::Configuration() = default;

int Configuration::port() const { return _port; }
void Configuration::setPort(int port) { _port = port; }
const std::string &Configuration::name() const { return _name; }
void Configuration::setName(const std::string &name) { _name = name; }

const Configuration::HeartBeat &Configuration::heartbeat() const { return _heartbeat; }
void Configuration::setHeartbeat(const Configuration::HeartBeat &heartbeat) { _heartbeat = heartbeat; }

const Configuration::Http &Configuration::http() const { return _http; }
void Configuration::setHttp(const Configuration::Http &http) { _http = http; }
const Configuration::Net &Configuration::net() const { return _net; }
void Configuration::setNet(const Configuration::Net &net) { _net = net; }
const Configuration::Threads &Configuration::threads() const { return _threads; }
void Configuration::setThreads(const Configuration::Threads &threads) { _threads = threads; }
const Configuration::Log &Configuration::log() const { return _log; }
void Configuration::setLog(const Configuration::Log &log) { _log = log; }
void Configuration::setSessions(const Sessions &sessions) { _sessions = sessions; }
void Configuration::setSubscriptions(const Subscriptions &subscriptions) { _subscriptions = subscriptions; }
const Configuration::Sessions &Configuration::sessions() const { return _sessions; }
const Configuration::Subscriptions &Configuration::subscriptions() const { return _subscriptions; }
const Configuration::Destinations &Configuration::destinations() const { return _destinations; }
void Configuration::setDestinations(const Configuration::Destinations &destinations) { _destinations = destinations; }
const Configuration::Storage &Configuration::storage() const { return _storage; }
void Configuration::setStorage(const Configuration::Storage &storage) { _storage = storage; }
std::string Configuration::toString() const {
  return std::string("\n- * \tport\t\t\t: ")
      .append(std::to_string(_port))
      .append("\n- * \tname\t\t\t: ")
      .append(_name)
      .append("\n- * \thttp\t\t\t=> ")
      .append(_http.toString())
      .append("\n- * \theartbeat\t\t: ")
      .append(_heartbeat.toString())
      .append("\n- * \tnet\t\t\t=> ")
      .append(_net.toString())
      .append("\n- * \tthreads\t\t\t=> ")
      .append(_threads.toString())
      .append("\n- * \tlog\t\t\t=> ")
      .append(_log.toString())
      .append("\n- * \tsessions\t\t=> ")
      .append(_sessions.toString())
      .append("\n- * \tsubscriptions\t\t=> ")
      .append(_subscriptions.toString())
      .append("\n- * \tdestination\t\t=> ")
      .append(_destinations.toString())
      .append("\n- * \tstorage\t\t\t=> ")
      .append(_storage.toString());
}
std::vector<std::string> Configuration::toStringLines() const {
  std::string s = toString();
  Poco::StringTokenizer lines(s, "\n", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
  std::vector<std::string> result(lines.count());
  std::copy(lines.begin(), lines.end(), result.begin());
  return result;
}
std::string Configuration::Http::toString() const {
  return std::string("\n- * \t\tport\t\t: ").append(std::to_string(port)).append("\n- * \t\tsite\t\t: [").append(site.toString()).append("]");
}
std::string Configuration::HeartBeat::toString() const { return std::to_string(sendTimeout).append(".").append(std::to_string(recvTimeout)); }
std::string Configuration::Net::toString() const { return std::string("\n- * \t\tmax-connections\t: ").append(std::to_string(maxConnections)); }
std::string Configuration::Threads::toString() const {
  return std::string("\n- * \t\taccept\t\t: ")
      .append(std::to_string(accepters))
      .append("\n- * \t\tread\t\t: ")
      .append(std::to_string(readers))
      .append("\n- * \t\tsubscribe\t: ")
      .append(std::to_string(subscribers))
      .append("\n- * \t\twrite\t\t: ")
      .append(std::to_string(writers));
}
uint32_t Configuration::Threads::all() const { return accepters + readers + writers + subscribers; }
std::string Configuration::Log::toString() const {
  return std::string("\n- * \t\tlevel\t\t: ")
      .append(std::to_string(level))
      .append("\n- * \t\tinteracive\t: ")
      .append(isInteractive ? "true" : "false")
      .append("\n- * \t\tpath\t\t: [")
      .append(path.toString())
      .append("]");
}
std::string Configuration::Sessions::toString() const { return std::string("\n- * \t\tmax-count\t: ").append(std::to_string(maxCount)); }
std::string Configuration::Subscriptions::toString() const { return std::string("\n- * \t\tmax-count\t: ").append(std::to_string(maxCount)); }
std::string Configuration::Destinations::toString() const {
  return std::string("\n- * \t\tautocreate\t: ")
      .append(autocreate ? "true" : "false")
      .append("\n- * \t\tforward-by-property\t: ")
      .append(forwardByProperty ? "true" : "false")
      .append("\n- * \t\tmax-count\t: ")
      .append(std::to_string(maxCount));
}
std::string Configuration::Storage::Connection::Props::toString() const {
  return std::string("\n- * \t\t\ttype\t: ")
      .append(typeName(dbmsType))
      .append("\n- * \t\t\tpool\t: ")
      .append(std::to_string(connectionPool))
      .append("\n- * \t\t\tsync\t: ")
      .append(useSync ? "true" : "false")
      .append("\n- * \t\t\tjournal-mode\t: ")
      .append(journalMode);
}
std::string Configuration::Storage::Connection::Value::toString() const {
  return std::string("\n- * \t\t\tuse-path\t: ").append(usePath ? "true" : "false").append("\n- * \t\t\tvalue\t: ").append(_v);
}
const std::string &Configuration::Storage::Connection::Value::get() const { return _v; }
void Configuration::Storage::Connection::Value::set(const std::string &connectionString) {
  _v = connectionString;
  Poco::trimInPlace(_v);
  Poco::StringTokenizer options(_v, ";", Poco::StringTokenizer::TOK_TRIM);
  _v.clear();
  std::string sep = (options.count() > 1) ? ";" : "";
  std::for_each(options.begin(), options.end(), [this, &sep](const std::string &item) { _v.append(Poco::trim(item) + sep); });
}
std::string Configuration::Storage::Connection::toString() const {
  return std::string("\n- * \t\tproperties\t=>")
      .append(props.toString())
      .append("\n- * \t\t\tvalue\t=>")
      .append(value.toString())
      .append("\n- * \t\t\tpath\t: [")
      .append(path.toString())
      .append("]");
}
std::string Configuration::Storage::Data::toString() const { return std::string("\n- * \t\t\tpath\t\t: [").append(_path.toString()).append("]"); }
void Configuration::Storage::Data::set(const std::string &path) {
  Poco::Path storageDataPath(path);
  if (storageDataPath.isRelative()) {
    storageDataPath.makeAbsolute();
  }
  _path = storageDataPath;

  _path.makeDirectory();
  Poco::File tmp(_path);
  if (!tmp.exists()) {
    tmp.createDirectories();
  }
}
const Poco::Path &Configuration::Storage::Data::get() const { return _path; }
std::string Configuration::Storage::Messages::toString() const {
  return std::string("\n- * \t\t\tnon-persistent-size\t\t: ").append(std::to_string(nonPresistentSize));
}
}  // namespace broker
}  // namespace upmq
