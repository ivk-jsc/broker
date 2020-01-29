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

#include "Consumer.h"
#include <Poco/Hash.h>

#include <utility>

upmq::broker::Consumer::Consumer(int _num,
                                 std::string _clientID,
                                 size_t _tcpNum,
                                 std::string _objectID,
                                 std::string _sessionID,
                                 Proto::Acknowledge _sessionType,
                                 const std::string &_selector,
                                 bool _noLocal,
                                 bool _browser,
                                 int _maxNotAxkMsg,
                                 std::shared_ptr<std::deque<std::shared_ptr<MessageDataContainer>>> selectCache)
    : num(_num),
      selector((_selector.empty() ? nullptr : (new storage::Selector(_selector)))),
      objectID(std::move(_objectID)),
      session(std::move(_sessionID), _sessionType),
      tcpNum(_tcpNum),
      clientID(std::move(_clientID)),
      noLocal(_noLocal),
      browser(_browser),
      isRunning(true),
      id(genConsumerID(clientID, std::to_string(tcpNum), session.id, (selector ? selector->expression() : ""))),
      maxNotAckMsg(_maxNotAxkMsg),
      select(std::move(selectCache)) {}
upmq::broker::Consumer::~Consumer() {}
void upmq::broker::Consumer::stop() const { isRunning = false; }
void upmq::broker::Consumer::start() const { isRunning = true; }
upmq::broker::Consumer upmq::broker::Consumer::makeFakeConsumer() {
  return Consumer(0,
                  "",
                  0,
                  "",
                  "",
                  Proto::Acknowledge::AUTO_ACKNOWLEDGE,
                  "",
                  false,
                  false,
                  0,
                  std::make_shared<std::deque<std::shared_ptr<MessageDataContainer>>>());
}
std::string upmq::broker::Consumer::genConsumerID(const std::string &_clientID,
                                                  const std::string &_tcpID,
                                                  const std::string &_sessionID,
                                                  const std::string &_selector) {
  size_t hsh = Poco::hash(_selector);
  std::string shash = (hsh != 0) ? std::to_string(hsh) : "";
  return _clientID + _sessionID + _tcpID + shash;
};
