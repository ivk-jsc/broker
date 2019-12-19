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

#include "CMSProvider.h"

#include <cms/ConnectionFactory.h>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

extern "C" {
#ifdef _WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif
}

using namespace std;
using namespace cms;

bool paramIsTopic = false;
bool paramIsDurable = false;

void cmsSleep(unsigned milliseconds) {
#ifdef _WIN32
  Sleep((DWORD)milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

////////////////////////////////////////////////////////////////////////////////
CMSProvider::CMSProvider(std::string brokerURL_, cms::Session::AcknowledgeMode ackMode_)
    : brokerURL(std::move(brokerURL_)),
      ackMode(ackMode_),
      username(),
      password(),
      clientId(),
      // destinationName(newUUID()),
      topic(paramIsTopic),
      durable(paramIsDurable),
      subscription("subscription_name"),
      connectionFactory(),
      connection(),
      session(),
      consumer(),
      producer(),
      noDestProducer(),
      destination(),
      tempDestination() {
  this->initialize();
}

////////////////////////////////////////////////////////////////////////////////
CMSProvider::CMSProvider(std::string brokerURL_, std::string subscription_, cms::Session::AcknowledgeMode ackMode_)
    : brokerURL(std::move(brokerURL_)),
      ackMode(ackMode_),
      topic(paramIsTopic),
      durable(paramIsDurable),
      subscription(std::move(subscription_)),
      connectionFactory(),
      connection(),
      session(),
      consumer(),
      producer(),
      noDestProducer(),
      destination(),
      tempDestination() {
  this->initialize();
}

////////////////////////////////////////////////////////////////////////////////
CMSProvider::~CMSProvider() {
  try {
    close();
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

////////////////////////////////////////////////////////////////////////////////
void CMSProvider::close() {
  if (this->consumer != nullptr) {
    try {
      this->consumer->close();
      this->consumer.reset(nullptr);
    } catch (cms::CMSException &ex) {
      ex.printStackTrace();
    }
  }
  if (this->producer != nullptr) {
    try {
      this->producer->close();
      this->producer.reset(nullptr);
    } catch (cms::CMSException &ex) {
      ex.printStackTrace();
    }
  }
  if (this->noDestProducer != nullptr) {
    try {
      this->noDestProducer->close();
      this->noDestProducer.reset(nullptr);
    } catch (cms::CMSException &ex) {
      ex.printStackTrace();
    }
  }

  this->tempDestination.reset(nullptr);

  if (this->session != nullptr) {
    try {
      this->session->close();
      this->session.reset(nullptr);
    } catch (cms::CMSException &ex) {
      ex.printStackTrace();
    }
  }

  if (this->connection != nullptr) {
    try {
      this->connection->close();
      this->connection.reset(nullptr);
    } catch (cms::CMSException &ex) {
      ex.printStackTrace();
    }
  }
}

std::string CMSProvider::getBrokerURL() const { return this->brokerURL; }

void CMSProvider::setBrokerURL(const std::string &newBrokerURL) { this->brokerURL = newBrokerURL; }

// void CMSProvider::setDestinationName(const std::string &name) { this->destinationName = name; }
//
// std::string CMSProvider::getDestinationName() const { return this->destinationName; }

void CMSProvider::setSubscription(const std::string &name) { this->subscription = name; }

std::string CMSProvider::getSubscription() const { return this->subscription; }

void CMSProvider::setTopic(bool value) { this->topic = value; }

bool CMSProvider::isTopic() const { return this->topic; }

void CMSProvider::setDurable(bool value) { this->durable = value; }

bool CMSProvider::isDurable() const { return this->durable; }

void CMSProvider::setAckMode(cms::Session::AcknowledgeMode newAckMode) { this->ackMode = newAckMode; }

cms::Session::AcknowledgeMode CMSProvider::getAckMode() const { return this->ackMode; }

std::string CMSProvider::newUUID() {
#ifdef _WIN32
  UUID uuid;
  UuidCreate(&uuid);
  unsigned char *str;
  UuidToStringA(&uuid, &str);
  std::string s(reinterpret_cast<char *>(str));
  RpcStringFreeA(&str);
#else
  uuid_t uuid = {0};
  uuid_generate_random(uuid);
  char ts[37] = {0};
  uuid_unparse(uuid, ts);
  const std::string s((char *)ts);
#endif
  return s;
}

////////////////////////////////////////////////////////////////////////////////
void CMSProvider::initialize(const std::string &aUsername, const std::string &aPassword, const std::string &aClientId) {
  try {
    this->username = aUsername;
    this->password = aPassword;
    this->clientId = aClientId;

    this->connectionFactory.reset(cms::ConnectionFactory::createCMSConnectionFactory(this->brokerURL));

    // Force a connect
    reconnect();

    // Force a new session to be created.
    reconnectSession();
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

////////////////////////////////////////////////////////////////////////////////
void CMSProvider::reconnect() {
  try {
    // Close everything first
    this->close();

    // Now create the connection
    this->connection.reset(getConnectionFactory()->createConnection(username, password, clientId));
    this->connection->start();

    if (this->session != nullptr) {
      reconnectSession();
    }
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

////////////////////////////////////////////////////////////////////////////////
void CMSProvider::reconnectSession() {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    if (this->consumer != nullptr) {
      this->consumer->close();
    }
    if (this->producer != nullptr) {
      this->producer->close();
    }
    if (this->noDestProducer != nullptr) {
      this->noDestProducer->close();
    }

    // Free any previously held resources.
    this->destination.reset(nullptr);
    this->tempDestination.reset(nullptr);
    this->consumer.reset(nullptr);
    this->producer.reset(nullptr);
    this->noDestProducer.reset(nullptr);

    // Create a new session, if there was one here before it will be
    // destroyed.
    this->session.reset(this->connection->createSession(this->ackMode));
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

////////////////////////////////////////////////////////////////////////////////
void CMSProvider::unsubscribe() {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    if (this->consumer && this->durable && this->topic) {
      this->consumer->close();

      this->session->unsubscribe(this->subscription);
    }
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::ConnectionFactory *CMSProvider::getConnectionFactory() {
  try {
    if (this->connectionFactory == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    return this->connectionFactory.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::Connection *CMSProvider::getConnection() {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    return this->connection.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::Session *CMSProvider::getSession() {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    return this->session.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::MessageProducer *CMSProvider::getProducer(const std::string &destinationName) {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    if (this->producer == nullptr) {
      this->producer.reset(this->getSession()->createProducer(this->getDestination(destinationName)));
    }

    return this->producer.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::MessageProducer *CMSProvider::getNoDestProducer() {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    if (this->noDestProducer == nullptr) {
      this->noDestProducer.reset(this->getSession()->createProducer(nullptr));
    }

    return this->noDestProducer.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::MessageConsumer *CMSProvider::getConsumer(const std::string &destinationName) {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    if (this->consumer == nullptr) {
      if (this->durable && this->topic) {
        this->consumer.reset(
            this->getSession()->createDurableConsumer(dynamic_cast<cms::Topic *>(this->getDestination(destinationName)), this->subscription, ""));
      } else {
        this->consumer.reset(this->getSession()->createConsumer(this->getDestination(destinationName)));
      }
    }

    return this->consumer.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::Destination *CMSProvider::getDestination(const std::string &destinationName) {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    if (this->destination == nullptr) {
      if (this->topic) {
        this->destination.reset(this->getSession()->createTopic(destinationName));
      } else {
        this->destination.reset(this->getSession()->createQueue(destinationName));
      }
    }

    return this->destination.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
cms::Destination *CMSProvider::getTempDestination() {
  try {
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }

    if (this->tempDestination == nullptr) {
      if (this->topic) {
        this->tempDestination.reset(this->getSession()->createTemporaryTopic());
      } else {
        this->tempDestination.reset(this->getSession()->createTemporaryQueue());
      }
    }

    return this->tempDestination.get();
  } catch (CMSException &e) {
    e.printStackTrace();
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
void CMSProvider::destroyDestination(const cms::Destination *pdestination) {
  (void)pdestination;
  try {
    // debug no need to destroy
    if (this->connection == nullptr) {
      throw CMSException("CMSProvider has not been Initialized or is closed.");
    }
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

void CMSProvider::cleanUpDestination(const std::string &destinationName) {
  cms::Session *currentSession(getSession());
  cms::MessageConsumer *currentConsumer(getConsumer(destinationName));

  // lets consume any outstanding messages from previous test runs
  cms::Message *message;
  while ((message = currentConsumer->receive(3000)) != nullptr) {
    delete message;
  }
  if (currentSession->isTransacted()) {
    currentSession->commit();
  }
  reconnectSession();
}
