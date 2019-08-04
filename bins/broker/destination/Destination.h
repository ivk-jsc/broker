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

#ifndef BROKER_DESTINATION_H
#define BROKER_DESTINATION_H

#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/Session.h>
#else
#include <Poco/Data/Session.h>
#endif

#include <Poco/FIFOEvent.h>
#include <memory>
#include <utility>
#include "DestinationOwner.h"
#include "Subscription.h"

namespace upmq {
namespace broker {

class Exchange;

class Destination {
 public:
  enum class Type : int { NONE = 0, QUEUE = 1, TOPIC = 2, TEMPORARY_QUEUE = 3, TEMPORARY_TOPIC = 4 };
  /// @brief SubscriptionsList - map<subs-name, subs>
  using SubscriptionsList = std::unordered_map<std::string, Subscription>;
  /// @brief RoutingList - map<routingKey, message>
  using RoutingList = std::unordered_map<std::string, std::unique_ptr<Poco::FIFOEvent<const MessageDataContainer *>>>;
  /// @brief NotAckConsumersInfoList - map<object_id, message_count>
  using NotAckConsumersInfoList = std::unordered_map<std::string, std::unique_ptr<std::atomic_int>>;
  /// @brief Session2SubscriptionMap - map<session_id, {subs-name}>
  /// ** used for binding destinations to clients
  using Session2SubsList = std::unordered_multimap<std::string, std::string>;
  /// @brief PredefinedClients - set<client-id>
  /// ** used for binding destinations to clients
  using PredefinedClients = std::unordered_map<std::string, bool>;

  struct Info {
    Info(std::string uri, std::string id, std::string name, Type type, std::string created, std::string dataPath, uint64_t messages)
        : uri(std::move(uri)), id(std::move(id)), name(std::move(name)), type(type), created(std::move(created)), dataPath(std::move(dataPath)), messages(messages) {}
    Info() = default;
    Info(Info &&) = default;
    Info(const Info &) = default;
    Info &operator=(Info &&) = default;
    Info &operator=(const Info &) = default;
    ~Info() = default;

    std::string uri;
    std::string id;
    std::string name;
    Type type = Type::NONE;
    std::string created;
    std::string dataPath;
    uint64_t messages = 0;
    std::vector<Subscription::Info> subscriptions;
  };

 protected:
  std::string _id;
  std::string _uri;
  std::string _name;
  SubscriptionsList _subscriptions;
  mutable upmq::MRWLock _subscriptionsLock;
  mutable Storage _storage;
  Type _type;
  mutable RoutingList _routing;
  const Exchange &_exchange;
  std::string _subscriptionsT;
  mutable NotAckConsumersInfoList _notAckList;
  mutable upmq::MRWLock _notAckLock;
  mutable Session2SubsList _s2subsList;
  mutable upmq::MRWLock _s2subsLock;
  PredefinedClients _predefinedSubscribers;
  mutable upmq::MRWLock _predefSubscribersLock;
  PredefinedClients _predefinedPublisher;
  mutable upmq::MRWLock _predefPublishersLock;
  std::unique_ptr<DestinationOwner> _owner;
  std::unique_ptr<Poco::Timestamp> _created{new Poco::Timestamp};
  Subscription::ConsumerMode _consumerMode{Subscription::ConsumerMode::ROUND_ROBIN};

 private:
  void eraseSubscription(SubscriptionsList::iterator &it);
  void addS2Subs(const std::string &sesionID, const std::string &subsID);
  void remS2Subs(const std::string &sessionID, const std::string &subsID);
  void createSubscriptionsTable(storage::DBMSSession &dbSession);
  static std::string getStoredDestinationID(const Exchange &exchange, const std::string &name, Destination::Type type);
  static void saveDestinationId(const std::string &id, storage::DBMSSession &dbSession, const Exchange &exchange, const std::string &name, Destination::Type type);

 public:
  Destination(const Exchange &exchange, const std::string &uri, Type type);
  virtual ~Destination();
  Destination(const Destination &o) = delete;
  Destination &operator=(const Destination &o) = delete;
  Destination(Destination &&o) = default;
  Destination &operator=(Destination &&o) = delete;

  virtual void save(const Session &session, const MessageDataContainer &sMessage);
  virtual void ack(const Session &session, const MessageDataContainer &sMessage);
  virtual void addSender(const Session &session, const MessageDataContainer &sMessage) = 0;
  virtual void removeSender(const Session &session, const MessageDataContainer &sMessage) = 0;
  virtual void removeSenders(const Session &session) = 0;
  virtual void removeSenderByID(const Session &session, const std::string &senderID) = 0;
  bool isSubscriptionExists(const std::string &name) const;
  bool isSubscriptionBrowser(const std::string &name) const;
  virtual Subscription &subscription(const Session &session, const MessageDataContainer &sMessage);
  static Subscription::ConsumerMode makeConsumerMode(const std::string &uri);
  Subscription::ConsumerMode consumerMode() const;
  static std::string consumerModeName(Subscription::ConsumerMode mode);
  void subscribe(const MessageDataContainer &sMessage);
  void unsubscribe(const MessageDataContainer &sMessage);
  virtual void begin(const Session &session);
  virtual void commit(const Session &session);
  void resetConsumersCache();
  virtual void abort(const Session &session);
  const std::string &id() const;
  const std::string &uri() const;
  bool isTemporary() const;
  bool isTopic() const;
  bool isQueue() const;
  bool isTempTopic() const;
  bool isTempQueue() const;
  bool isTopicFamily() const;
  bool isQueueFamily() const;
  static std::string routingKey(const std::string &uri);
  const std::string &subscriptionsT() const;
  size_t subscriptionsCount() const;
  size_t subscriptionsTrueCount(bool noLock = false) const;
  const std::string &name() const;
  void doAck(const Session &session, const MessageDataContainer &sMessage, Storage &storage, bool browser, const std::vector<MessageInfo> &messages);
  void increaseNotAcknowledged(const std::string &objectID);
  void increaseNotAcknowledgedAll();
  void decreesNotAcknowledged(const std::string &objectID) const;
  bool canSendNextMessages(const std::string &objectID) const;
  void addToNotAckList(const std::string &objectID, int count) const;
  void remFromNotAck(const std::string &objectID) const;
  void postNewMessageEvent() const;
  bool removeConsumer(const std::string &sessionID, const std::string &subscriptionID, size_t tcpNum);
  void subscribeOnNotify(Subscription &subscription) const;
  void unsubscribeFromNotify(Subscription &subscription) const;
  int64_t initBrowser(const std::string &subscriptionName);
  Storage &storage() const;
  void copyMessagesTo(Subscription &subscription);
  virtual Subscription createSubscription(const std::string &name, const std::string &routingKey, Subscription::Type type) = 0;
  virtual void addSendersFromCache(const Session &session, const MessageDataContainer &sMessage, Subscription &subscription) = 0;
  void closeAllSubscriptions(const Session &session, size_t tcpNum);
  void bindWithSubscriber(const std::string &clientID, bool useFileLink);
  void unbindFromSubscriber(const std::string &clientID);
  bool isBindToSubscriber(const std::string &clientID) const;
  bool isSubscriberUseFileLink(const std::string &clientID) const;
  void bindWithPublisher(const std::string &clientID, bool useFileLink);
  void unbindFromPublisher(const std::string &clientID);
  bool isBindToPublisher(const std::string &clientID) const;
  bool isPublisherUseFileLink(const std::string &clientID) const;
  void setOwner(const std::string &clientID, size_t tcpID);
  const DestinationOwner &owner() const;
  bool hasOwner() const;
  bool getNexMessageForAllSubscriptions();
  Info info() const;
  static std::string typeName(Type type);
  static Destination::Type type(const std::string &typeName);

 protected:
  void loadDurableSubscriptions();

 private:
  message::GroupStatus getMsgGroupStatus(const MessageInfo &msg) const;
  void removeMessageOrGroup(const Session &session, Storage &storage, const MessageInfo &msg, message::GroupStatus groupStatus);
};
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_DESTINATION_H
