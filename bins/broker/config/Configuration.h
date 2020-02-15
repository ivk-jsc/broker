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

#ifndef BROKER_CONFIGURATION_H
#define BROKER_CONFIGURATION_H

#include <Defines.h>
#include <Poco/Path.h>
#include <Singleton.h>
#include <string>
#ifdef ENABLE_WEB_ADMIN
#include <TemplateParamReplacer.h>
#endif
#include <memory>
#include <unordered_map>

namespace upmq {
namespace broker {

class Configuration {
 public:
#ifdef ENABLE_WEB_ADMIN
  using ReplacerMapType = std::unordered_map<std::string, std::unique_ptr<TemplateParamReplacer>>;
  ReplacerMapType replacerMap;
#endif

  struct Http {
    int port{9090};
    Poco::Path site;
    std::string toString() const;
  };

  struct HeartBeat {
    int sendTimeout{0};
    int recvTimeout{0};
    std::string toString() const;
  };

  struct Net {
    int maxConnections{1024};
    std::string toString() const;
  };

  struct Threads {
    uint32_t accepters{8};
    uint32_t readers{8};
    uint32_t writers{8};
    uint32_t subscribers{8};
    std::string toString() const;
    uint32_t all() const;
  };

  struct Log {
    int level{4};
    std::string name{"broker"};
    Poco::Path path;
    bool isInteractive{true};
    std::string toString() const;
  };

  struct Sessions {
    size_t maxCount{1024};
    std::string toString() const;
  };

  struct Subscriptions {
    size_t maxCount{1024};
    std::string toString() const;
  };

  struct Destinations {
    bool autocreate{true};
    bool forwardByProperty{false};
    size_t maxCount{1024};
    std::string toString() const;
  };

  struct Storage {
    struct Connection {
      struct Props {
        storage::DBMSType dbmsType{storage::NO_TYPE};
        int connectionPool{64};
        bool useSync{false};
        std::string journalMode{"WAL"};
        std::string toString() const;
      };
      struct Value {
       private:
        std::string _v{"file::memory:?cache=shared"};

       public:
        const std::string &get() const;
        void set(const std::string &connectionString);
        bool usePath{true};
        std::string toString() const;
      };
      Props props;
      Value value;
      Poco::Path path;
      std::string toString() const;
    };
    struct Data {
     private:
      Poco::Path _path;

     public:
      const Poco::Path &get() const;
      void set(const std::string &path);
      std::string toString() const;
    };
    struct Messages {
      size_t nonPresistentSize{100000};
      std::string toString() const;
    };

   private:
    std::string _messageJournal;

   public:
    Connection connection;
    Data data;
    Messages messages;
    std::string messageJournal(const std::string &destinationName) const;
    void setMessageJournal(const std::string &brokerName);
    std::string toString() const;

    static storage::DBMSType type(const std::string &dbms);
    static std::string typeName(storage::DBMSType dbmsType);
  };

  Configuration();
  virtual ~Configuration() = default;

  int port() const;
  void setPort(int port);

  const std::string &name() const;
  void setName(const std::string &name);

  const Http &http() const;
  void setHttp(const Http &http);

  const HeartBeat &heartbeat() const;
  void setHeartbeat(const HeartBeat &heartbeat);

  const Net &net() const;
  void setNet(const Net &net);
  const Threads &threads() const;
  void setThreads(const Threads &threads);
  const Log &log() const;
  void setLog(const Log &log);
  void setSessions(const Sessions &sessions);
  void setSubscriptions(const Subscriptions &subscriptions);
  const Sessions &sessions() const;
  const Subscriptions &subscriptions() const;
  const Destinations &destinations() const;
  void setDestinations(const Destinations &destinations);
  const Storage &storage() const;
  void setStorage(const Storage &storage);

  std::string toString() const;
  std::vector<std::string> toStringLines() const;

 private:
  int _port{12345};
  std::string _name{"broker"};
  Http _http;
  HeartBeat _heartbeat;
  Net _net;
  Threads _threads;
  Log _log;
  Sessions _sessions;
  Subscriptions _subscriptions;
  Destinations _destinations;
  Storage _storage;
};
}  // namespace broker
}  // namespace upmq

typedef Singleton<upmq::broker::Configuration> CONFIGURATION;

#define STORAGE_CONFIG CONFIGURATION::Instance().storage()
#define SESSIONS_CONFIG CONFIGURATION::Instance().sessions()
#define SUBSCRIPTIONS_CONFIG CONFIGURATION::Instance().subscriptions()
#define DESTINATION_CONFIG CONFIGURATION::Instance().destinations()
#define THREADS_CONFIG CONFIGURATION::Instance().threads()
#define NET_CONFIG CONFIGURATION::Instance().net()
#define LOG_CONFIG CONFIGURATION::Instance().log()

#endif  // BROKER_CONFIGURATION_H
