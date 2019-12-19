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

#include <thread>
#include <atomic>

#include <cms/ConnectionFactory.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/ExceptionListener.h>
#include <cms/Message.h>
#include <cms/Utils.h>

using perf_clock =
    std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
using floating_seconds = std::chrono::duration<double>;

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include "optparse.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#include <iomanip>

void usage(const struct optparse_long *opt_option, size_t count) {
  std::cout << "simpleconsumer usage : " << std::endl;
  for (size_t i = 0; i < count; ++i) {
    std::cout << "\t"
              << "--" << std::left << std::setw(16) << opt_option[i].longname << " -" << (char)opt_option[i].shortname << " \t"
              << opt_option[i].description << " " << (opt_option[i].need_arg ? " (need argument)" : "") << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
class SimpleConsumer : public cms::ExceptionListener {
 private:
  std::unique_ptr<cms::ConnectionFactory> connectionFactory;
  std::unique_ptr<cms::Connection> connection;
  std::unique_ptr<cms::Session> session;
  std::unique_ptr<cms::Destination> destination;
  std::unique_ptr<cms::MessageConsumer> consumer;
  std::string brokerURI;
  std::string destURI;
  bool useTopic;
  std::atomic<bool> isStoped;
  std::string consMode;
  std::string selector;
  std::string outFormat{"text"};
  long mod = 1000;

 public:
  std::chrono::steady_clock::time_point t0{perf_clock::now()};

  SimpleConsumer(std::string brokerURI_, std::string destURI_, bool useTopic_, std::string consMode_, long logMod)
      : brokerURI(std::move(brokerURI_)),
        destURI(std::move(destURI_)),
        useTopic(useTopic_),
        isStoped(false),
        consMode(std::move(consMode_)),
        mod(logMod) {}

  ~SimpleConsumer() override {
    try {
      destination.reset();
      consumer.reset();
      session.reset();
      connection.reset();
      connectionFactory.reset();
    } catch (...) {
    }
  }
  void setSelector(const std::string &newSelector) { selector = newSelector; }
  void setOutFormat(const std::string &newOutFormat) { outFormat = newOutFormat; }

  void open() {
    connectionFactory.reset(cms::ConnectionFactory::createCMSConnectionFactory(brokerURI));

    connection.reset(connectionFactory->createConnection());
    connection->setExceptionListener(this);
    connection->start();

    session.reset(connection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
    if (useTopic) {
      destination.reset(session->createTopic(destURI));
    } else {
      destination.reset(session->createQueue(destURI));
    }

    if (consMode == "regular") {
      consumer.reset(session->createConsumer(destination.get(), selector));
    } else {
      consumer.reset(session->createDurableConsumer(dynamic_cast<const cms::Topic *>(destination.get()), "test-durable-consumer", selector));
    }
  }

  void close() {
    isStoped.store(true);

    try {
      consumer->close();
      session->close();
      connection->close();
    } catch (cms::CMSException &e) {
      std::cerr << "Exception occurred: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Exception occurred!" << std::endl;
    }
  }

  void run() {
    bool flag = false;
    try {
      int i = 1;
      while (!isStoped.load()) {
        std::unique_ptr<cms::Message> message(consumer->receive());
        if (!flag) {
          t0 = perf_clock::now();
          flag = true;
        }
        if (message != nullptr) {
          const auto *textMessage = dynamic_cast<const cms::TextMessage *>(message.get());
          if (textMessage != nullptr) {
            if ((i == 1 || (i % mod == 0)) && (outFormat.empty() || outFormat != "json")) {
              std::cout << "recv (" << i << ") <= " << textMessage->getText() << " elapsed [" << floating_seconds(perf_clock::now() - t0).count()
                        << "]" << '\n';
            } else if (outFormat == "json") {
              std::cout << textMessage->getText() << std::endl;
              std::cout << "json => " << std::endl;
              std::cout << cms::Utils::toPrettyJsonString(textMessage) << std::endl;
              std::cout << "-------------------------------" << std::endl;
            }
          }
          message.reset();
          i++;
        }
      }
    } catch (cms::CMSException &e) {
      e.printStackTrace();
    }
  }

  void onException(const cms::CMSException &ex) override { std::cerr << "Exception occurred: " << ex.what() << std::endl; }
};

template <typename F>
int processOption(int option, const char *arg, const F &f) {
  if (arg != nullptr) {
    f(arg);
    return 0;
  }
  std::cerr << "invalid option " << static_cast<char>(option) << std::endl;
  return -1;
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  (void)argc;
  std::string brokerURI = "failover:(tcp://localhost:12345?transport.trace=false)";
  // std::string brokerURI = "tcp://localhost:12345?transport.trace=false";
  std::string destURI = "defaultDestination";
  std::string destType = "queue";
  std::string consMode = "regular";
  std::string selector;
  std::string outFormat = "text";
  bool useTopics = false;
  long logMod = 1000;

  /* API is data structure driven */
  static const struct optparse_long opt_option[] = {
      /* long-option, short-option, has-arg flag, description */
      {"destination", 'd', OPTPARSE_OPTIONAL, true, "destination name or uri, default is defaultDestination"},
      {"type", 't', OPTPARSE_OPTIONAL, true, "destination type [queue or topic], default is queue"},
      {"mode", 'm', OPTPARSE_OPTIONAL, true, "consumer mode [durable or regular], default is regular"},
      {"selector", 's', OPTPARSE_OPTIONAL, true, "consumer selector (sql92-where), default is empty"},
      {"out-format", 'o', OPTPARSE_OPTIONAL, true, "message out format [text or json], default is text"},
      {"uri", 'u', OPTPARSE_OPTIONAL, true, "uri - broker connection string, default is tcp://localhost:12345?transport.trace=false"},
      {"log_mod", 'l', OPTPARSE_OPTIONAL, true, "log_mod - a number of skipped messages before log, default is 1000"},
      {"help", 'h', OPTPARSE_OPTIONAL, false, "show help"},
      {nullptr, 0, OPTPARSE_NONE, false, nullptr}, /* end (a.k.a. sentinel) */
  };

  int option;
  struct optparse options {};
  optparse_init(&options, argv);
  /* parse the all options based on opt_option[] */
  while ((option = optparse_long(&options, opt_option, nullptr)) != -1) {
    int processOptionResult = 0;
    switch (option) {
      case 'd':
        processOptionResult = processOption(option, options.optarg, [&destURI](const char *arg) { destURI.assign(arg); });
        break;
      case 't':
        processOptionResult = processOption(option, options.optarg, [&useTopics](const char *arg) { useTopics = (std::string(arg) == "topic"); });
        break;
      case 'u':
        processOptionResult = processOption(option, options.optarg, [&brokerURI](const char *arg) { brokerURI.assign(arg); });
        break;
      case 'm':
        processOptionResult = processOption(
            option, options.optarg, [&consMode](const char *arg) { consMode = ((std::string(arg) == "durable") ? "durable" : "regular"); });
        break;
      case 'o':
        processOptionResult =
            processOption(option, options.optarg, [&outFormat](const char *arg) { outFormat = (std::string(arg) == "json") ? "json" : "text"; });
        break;
      case 's':
        processOptionResult = processOption(option, options.optarg, [&selector](const char *arg) { selector.assign(arg); });
        break;
      case 'l':
        processOptionResult = processOption(option, options.optarg, [&logMod](const char *arg) { logMod = strtol(arg, nullptr, 10); });
        break;
      case 'h':
        usage(opt_option, 8);
        return 0;
      default:
        break;
    }
    if (processOptionResult != 0) {
      usage(opt_option, 8);
      return -1;
    }
  }
  char *arg = optparse_arg(&options);
  while (arg) {
    printf("%s\n", arg);
    arg = optparse_arg(&options);
  }

  std::cout << "=====================================================\n";
  std::cout << "SimpleConsumer start "
            << "(" << brokerURI << " " << destType << ":" << destURI << ")" << std::endl;
  std::cout << "=====================================================\n";

  try {
    SimpleConsumer consumer(brokerURI, destURI, useTopics, consMode, logMod);
    if (!selector.empty()) {
      consumer.setSelector(selector);
    }
    if (outFormat != "text") {
      consumer.setOutFormat(outFormat);
    }
    consumer.open();
    std::thread thr(&SimpleConsumer::run, &consumer);
    std::cout << "Press 'q' to quit" << std::endl;
    while (std::cin.get() != 'q') {
    }
    consumer.close();
    thr.join();

  } catch (cms::CMSException &e) {
    e.printStackTrace();
  } catch (...) {
    return 1;
  }

  std::cout << "=====================================================\n";
  std::cout << "SimpleConsumer end" << std::endl;
  std::cout << "=====================================================\n";
  return 0;
}
