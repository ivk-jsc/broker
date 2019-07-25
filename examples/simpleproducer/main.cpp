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

#include <iostream>
#include <sstream>
#include <ctime>
#include <chrono>

#include <cms/ConnectionFactory.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>

using perf_clock = std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
using floating_seconds = std::chrono::duration<double>;

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"
#include <iomanip>

void usage(const struct optparse_long *opt_option, size_t count) {
  std::cout << "simpleproducer usage : " << std::endl;
  for (size_t i = 0; i < count; ++i) {
    std::cout << "\t"
              << "--" << std::left << std::setw(16) << opt_option[i].longname << " -" << (char)opt_option[i].shortname << " \t" << opt_option[i].description << " "
              << (opt_option[i].need_arg ? " (need argument)" : "") << std::endl;
  }
}

struct IntProperty {
  std::string key{};
  int value{0};
  IntProperty(std::string key, int value) : key(std::move(key)), value(value) {}
  IntProperty() = default;
  IntProperty(const IntProperty &) = default;
  IntProperty(IntProperty &&) = default;
  IntProperty &operator=(const IntProperty &) = default;
  IntProperty &operator=(IntProperty &&) = default;
  bool isEmpty() const { return key.empty(); };
  static IntProperty fromString(const std::string &line) {
    std::string::size_type pos = line.find('=');
    if (pos == std::string::npos) {
      std::cerr << "invalid int-property " << std::endl;
      throw std::runtime_error(line);
    }
    IntProperty intProperty;
    intProperty.key = line.substr(0, pos);
    intProperty.value = std::stoi(line.substr(pos + 1));
    return intProperty;
  }
};

////////////////////////////////////////////////////////////////////////////////
class SimpleProducer {
 private:
  cms::ConnectionFactory *connectionFactory;
  cms::Connection *connection;
  cms::Session *session;
  cms::Destination *destination;
  cms::MessageProducer *producer;

  std::string brokerURI;
  std::string destURI;
  long numMessages;
  bool useTopic;
  std::string text;
  IntProperty intProperty;
  int priority = cms::Message::DEFAULT_MSG_PRIORITY;
  cms::DeliveryMode::DELIVERY_MODE deliveryMode = cms::DeliveryMode::PERSISTENT;

 public:
  std::chrono::steady_clock::time_point t0{perf_clock::now()};

  SimpleProducer(std::string brokerURI, long numMessages, std::string destURI, bool useTopic)
      : connectionFactory(nullptr),
        connection(nullptr),
        session(nullptr),
        destination(nullptr),
        producer(nullptr),
        brokerURI(std::move(brokerURI)),
        destURI(std::move(destURI)),
        numMessages(numMessages),
        useTopic(useTopic) {}

  ~SimpleProducer() {
    delete destination;
    delete producer;
    delete session;
    delete connection;
    delete connectionFactory;
  }

  void setText(const std::string &newText) { this->text = newText; }
  void setIntProperty(const IntProperty &newIntProperty) { this->intProperty = newIntProperty; }
  void setDeliveryMode(cms::DeliveryMode::DELIVERY_MODE newDeliveryMode) { this->deliveryMode = newDeliveryMode; }
  void open() {
    connectionFactory = cms::ConnectionFactory::createCMSConnectionFactory(brokerURI);

    connection = connectionFactory->createConnection();
    connection->start();

    session = connection->createSession(cms::Session::AUTO_ACKNOWLEDGE);

    if (useTopic) {
      destination = session->createTopic(destURI);
    } else {
      destination = session->createQueue(destURI);
    }

    producer = session->createProducer(destination);
  }

  void setPriority(int aPriority) { priority = aPriority; }

  void close() {
    producer->close();
    session->close();
    connection->close();
  }

  void run() {
    try {
      cms::TextMessage *message = message = session->createTextMessage();
      std::string messageText;
      if (!intProperty.isEmpty()) {
        message->setIntProperty(intProperty.key, intProperty.value);
      }
      for (long ix = 1; ix <= numMessages; ++ix) {
        if (text.empty()) {
          messageText = (std::string("Message ").append("number : ").append(std::to_string(ix)));
        } else {
          messageText = (text);
        }
        message->setText(messageText);

        producer->send(destination, message, deliveryMode, priority, cms::Message::DEFAULT_TIME_TO_LIVE);

        message->setReadable();

        std::cout << "sent => "
                  << ": " << message->getText() << " elapsed [" << floating_seconds(perf_clock::now() - t0).count() << "]" << '\n';
      }
      delete message;
    } catch (cms::CMSException &e) {
      e.printStackTrace();
    } catch (...) {
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  (void)argc;
  //std::string brokerURI = "failover:(tcp://localhost:12345?transport.trace=false)";
  std::string brokerURI = "tcp://localhost:12345?transport.trace=false";
  std::string destURI = "defaultDestination";
  std::string destType = "queue";
  int priority = cms::Message::DEFAULT_MSG_PRIORITY;
  long numMessages = 1;
  bool useTopics = false;
  cms::DeliveryMode::DELIVERY_MODE deliveryMode = cms::DeliveryMode::PERSISTENT;
  IntProperty intProperty;
  std::string text;

  /* API is data structure driven */
  static const struct optparse_long opt_option[] = {
      /* long-option, short-option, has-arg flag, description */
      {"destination", 'd', OPTPARSE_OPTIONAL, true, "destination name or uri, default is defaultDestination"},
      {"type", 't', OPTPARSE_OPTIONAL, true, "destination type [queue or topic], default is queue"},
      {"count", 'c', OPTPARSE_OPTIONAL, true, "message count, default is 1"},
      {"delivery-mode", 'm', OPTPARSE_OPTIONAL, true, "message delivery mode [persistent or not-persistent], default is persistent"},
      {"body-text", 'b', OPTPARSE_OPTIONAL, true, "message body text, default is digit which is message sequence number"},
      {"priority", 'p', OPTPARSE_OPTIONAL, true, "message priority [0..9], default is 4"},
      {"int-property", 'i', OPTPARSE_OPTIONAL, true, "message int property, set with key=value pattern, for ex., --int-property=\"a=10\""},
      {"uri", 'u', OPTPARSE_OPTIONAL, true, "uri - broker connection string, default is tcp://localhost:12345?transport.trace=false"},
      {"help", 'h', OPTPARSE_OPTIONAL, false, "show help"},
      {nullptr, 0, OPTPARSE_NONE, 0, nullptr}, /* end (a.k.a. sentinel) */
  };
  int option;
  struct optparse options {};
  optparse_init(&options, argv);
  /* parse the all options based on opt_option[] */
  while ((option = optparse_long(&options, opt_option, NULL)) != -1) {
    switch (option) {
      case 'd':
        destURI.assign(options.optarg);
        break;
      case 't':
        useTopics = (std::string(options.optarg) == "topic");
        break;
      case 'u':
        brokerURI.assign(options.optarg);
        break;
      case 'c':
        numMessages = std::stol(options.optarg);
        break;
      case 'm':
        deliveryMode = ((std::string(options.optarg) == "not-persistent") ? cms::DeliveryMode::NON_PERSISTENT : cms::DeliveryMode::PERSISTENT);
        break;
      case 'p':
        priority = std::stoi(options.optarg);
        break;
      case 'i':
        intProperty = IntProperty::fromString(options.optarg);
        break;
      case 'b':
        text = std::string(options.optarg);
        break;
      case 'h':
        usage(opt_option, 9);
        return 0;
      default:
        break;
    }
  }
  char *arg = optparse_arg(&options);
  while (arg) {
    printf("%s\n", arg);
    arg = optparse_arg(&options);
  }

  std::cout << "=====================================================\n";
  std::cout << "Simpleproducer start "
            << "(" << brokerURI << " " << destType << ":" << destURI << " message count [" << numMessages << "])" << std::endl;
  std::cout << "=====================================================\n";

  try {
    SimpleProducer producer(brokerURI, numMessages, destURI, useTopics);
    if (!intProperty.isEmpty()) {
      producer.setIntProperty(intProperty);
    }
    producer.setDeliveryMode(deliveryMode);
    producer.setText(text);
    producer.setPriority(priority);
    producer.open();
    producer.run();
    producer.close();
    std::cout << "finish = elapsed [" << floating_seconds(perf_clock::now() - producer.t0).count() << "]" << '\n';
  } catch (cms::CMSException &e) {
    e.printStackTrace();
  } catch (...) {
    return -1;
  }

  std::cout << "=====================================================\n";
  std::cout << "Simpleproducer end" << std::endl;
  std::cout << "=====================================================\n";
  return numMessages;
}
