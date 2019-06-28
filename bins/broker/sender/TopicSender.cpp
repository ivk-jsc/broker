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

#include "TopicSender.h"
#include "Subscription.h"

namespace upmq {
namespace broker {
TopicSender::TopicSender(const std::string &id, const Session &session, const Subscription &subscription) : Sender(id, session), _subscription(subscription) {}
TopicSender::~TopicSender() {}
void TopicSender::fixMessageInGroup(const Session &session, const MessageDataContainer &sMessage) const { Sender::fixMessageInGroup(session, _subscription.storage(), sMessage); }
void TopicSender::closeGroup(const Session &session) const { Sender::closeGroup(session, _subscription.storage()); }
}  // namespace broker
}  // namespace upmq
