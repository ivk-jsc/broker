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

#ifndef _CMSPROVIDER_H_
#define _CMSPROVIDER_H_

#include <memory>
#include <string>

#include <cms/CMSException.h>
#include <cms/Connection.h>
#include <cms/ConnectionFactory.h>
#include <cms/Destination.h>
#include <cms/InvalidClientIdException.h>
#include <cms/MessageConsumer.h>
#include <cms/MessageListener.h>
#include <cms/MessageProducer.h>
#include <cms/Session.h>
#include <gtest/gtest.h>

extern bool paramIsTopic;
extern bool paramIsDurable;

class CMSProvider {
 private:
  std::string brokerURL;
  cms::Session::AcknowledgeMode ackMode;
  std::string username;
  std::string password;
  std::string clientId;

  // std::string destinationName;
  bool topic;
  bool durable;
  std::string subscription;

  std::unique_ptr<cms::ConnectionFactory> connectionFactory;
  std::unique_ptr<cms::Connection> connection;
  std::unique_ptr<cms::Session> session;
  std::unique_ptr<cms::MessageConsumer> consumer;
  std::unique_ptr<cms::MessageProducer> producer;
  std::unique_ptr<cms::MessageProducer> noDestProducer;
  std::unique_ptr<cms::Destination> destination;
  std::unique_ptr<cms::Destination> tempDestination;

 public:
  CMSProvider(std::string brokerURL, cms::Session::AcknowledgeMode ackMode = cms::Session::AUTO_ACKNOWLEDGE);

  CMSProvider(std::string brokerURL, std::string subscription, cms::Session::AcknowledgeMode ackMode = cms::Session::AUTO_ACKNOWLEDGE);

  ~CMSProvider();

  void close();

  std::string getBrokerURL() const;

  void setBrokerURL(const std::string &newBrokerURL);

  void setSubscription(const std::string &name);

  std::string getSubscription() const;

  void setTopic(bool value);

  bool isTopic() const;

  void setDurable(bool value);

  bool isDurable() const;

  void setAckMode(cms::Session::AcknowledgeMode newAckMode);

  cms::Session::AcknowledgeMode getAckMode() const;

  static std::string newUUID();
  /**
   * Initializes a CMSProvider with the Login data for the session that
   * this provider is managing.  Once called a new Connection to the broker
   * is made and will remain open until a reconnect is requested or until
   * the CMSProvider instance is closed.
   */
  void initialize(const std::string &aUsername = "", const std::string &aPassword = "", const std::string &aClientId = "");

  /**
   * Forces a reconnect of the Connection and then of the Session and its
   * associated resources.
   */
  void reconnect();

  /**
   * Forces a Recreation of a Session and any of its Resources.
   */
  void reconnectSession();

  /**
   * Unsubscribes a durable consumer if one has been created and the chosen
   * wireformat supports it.  The consumer is closed as a result any calls to
   * it after calling this method will result in an error.
   */
  void unsubscribe();

  /**
   * Returns the ConnectionFactory object that this Provider has allocated.
   */
  cms::ConnectionFactory *getConnectionFactory();

  /**
   * Returns the Connection object that this Provider has allocated.
   */
  cms::Connection *getConnection();

  /**
   * Returns the Session object that this Provider has allocated.
   */
  cms::Session *getSession();

  /**
   * Returns the MessageConsumer object that this Provider has allocated.
   */
  cms::MessageConsumer *getConsumer(const std::string &destinationName = ::testing::UnitTest::GetInstance()->current_test_info()->name());

  /**
   * Returns the MessageProducer object that this Provider has allocated.
   */
  cms::MessageProducer *getProducer(const std::string &destinationName = ::testing::UnitTest::GetInstance()->current_test_info()->name());

  /**
   * Returns the MessageProducer object that this Provider has allocated that has
   * no assigned Destination, message sent must be assigned one at send time.
   */
  cms::MessageProducer *getNoDestProducer();

  /**
   * Returns the Destination object that this Provider has allocated.
   */
  cms::Destination *getDestination(const std::string &destinationName = ::testing::UnitTest::GetInstance()->current_test_info()->name());

  /**
   * Returns the Temporary Destination object that this Provider has allocated.
   */
  cms::Destination *getTempDestination();

  /**
   * Destroys a Destination at the Broker side, freeing the resources associated with it.
   */
  void destroyDestination(const cms::Destination *pdestination);

  void cleanUpDestination(const std::string &destinationName = ::testing::UnitTest::GetInstance()->current_test_info()->name());
};

void cmsSleep(unsigned milliseconds);

#endif /*_CMSPROVIDER_H_*/
