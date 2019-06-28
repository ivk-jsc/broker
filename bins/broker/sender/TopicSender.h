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
 
#ifndef BROKER_TOPICSENDER_H
#define BROKER_TOPICSENDER_H

#include "Sender.h"

namespace upmq {
namespace broker {

class Subscription;

class TopicSender : public Sender {
 private:
  const Subscription &_subscription;

 public:
  TopicSender(const std::string &id, const Session &session, const Subscription &subscription);
  ~TopicSender() override;
  void fixMessageInGroup(const Session &session, const MessageDataContainer &sMessage) const override;
  void closeGroup(const Session &session) const override;
};
}  // namespace broker
}  // namespace upmq
#endif  // BROKER_TOPICSENDER_H
