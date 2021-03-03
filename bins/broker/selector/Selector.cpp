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

#include "Selector.h"

#include "SelectorExpression.h"
#include "SelectorValue.h"

#include <chrono>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <Poco/DateTime.h>
#include "Exception.h"
#include "MessageDataContainer.h"
#include "ProtoBuf.h"

namespace upmq {
namespace broker {
namespace storage {

using std::string;

constexpr char PERSISTENT[] = ("PERSISTENT");
constexpr char NON_PERSISTENT[] = ("NON_PERSISTENT");

namespace {
using Aliases = std::unordered_map<std::string, std::string>;
Aliases define_aliases() {
  Aliases aliases;
  aliases["JMSType"] = "subject";
  aliases["JMSCorrelationID"] = "correlation_id";
  aliases["JMSMessageID"] = "message_id";
  aliases["JMSDeliveryMode"] = "delivery_mode";
  aliases["JMSRedelivered"] = "redelivered";
  aliases["JMSPriority"] = "priority";
  aliases["JMSDestination"] = "to";
  aliases["JMSReplyTo"] = "reply_to";
  aliases["JMSTimestamp"] = "creation_time";
  aliases["JMSExpiration"] = "absolute_expiry_time";
  return aliases;
}
const Aliases aliases = define_aliases();
}  // namespace

class MessageSelectorEnv : public SelectorEnv {
  const upmq::broker::MessageDataContainer &msg;
  mutable std::vector<std::shared_ptr<std::string>> returnedStrings;
  mutable std::unordered_map<std::string, Value> returnedValues;

  const Value &value(const std::string &identifier) const override;
  Value specialValue(const std::string &id) const;

 public:
  explicit MessageSelectorEnv(const upmq::broker::MessageDataContainer &m);
};

MessageSelectorEnv::MessageSelectorEnv(const upmq::broker::MessageDataContainer &m) : msg(m) {}

Value MessageSelectorEnv::specialValue(const std::string &id) const {
  Value v;
  const Proto::Message &protoMessage = msg.message();
  // TODO(bas): Just use a simple if chain for now - improve this later
  if (id == "delivery_mode") {
    v = protoMessage.persistent() ? std::string(PERSISTENT) : std::string(NON_PERSISTENT);
  } else if (id == "subject") {
    std::string s = protoMessage.type();
    if (!s.empty()) {
      returnedStrings.push_back(std::make_shared<std::string>(s));
      v = *returnedStrings[returnedStrings.size() - 1];
    }
  } else if (id == "redelivered") {
    // Although redelivered is defined to be true delivery-count>0 if it is 0
    // now
    // it will be 1 by the time the message is delivered
    v = protoMessage.redelivered();
  } else if (id == "priority") {
    v = int64_t(protoMessage.priority());
  } else if (id == "correlation_id") {
    std::string cId = protoMessage.correlation_id();
    if (!cId.empty()) {
      returnedStrings.push_back(std::make_shared<std::string>(cId));
      v = *returnedStrings[returnedStrings.size() - 1];
    }
  } else if (id == "message_id") {
    std::string mId = protoMessage.message_id();
    if (!mId.empty()) {
      returnedStrings.push_back(std::make_shared<std::string>(mId));
      v = *returnedStrings[returnedStrings.size() - 1];
    }
  } else if (id == "to") {
    std::string s = protoMessage.destination_uri();
    if (!s.empty()) {
      returnedStrings.push_back(std::make_shared<std::string>(s));
      v = *returnedStrings[returnedStrings.size() - 1];
    }
  } else if (id == "reply_to") {
    std::string s = protoMessage.reply_to();
    if (!s.empty()) {
      returnedStrings.push_back(std::make_shared<std::string>(s));
      v = *returnedStrings[returnedStrings.size() - 1];
    }
  } else if (id == "absolute_expiry_time") {
    int64_t expiry = protoMessage.expiration();
    Poco::DateTime currentDateTime;

    // Java property has value of 0 for no expiry
    v = (expiry == std::numeric_limits<int64_t>::max()) ? 0 : (expiry - currentDateTime.timestamp().epochTime()) / 1000 * 1000 * 1000;
  } else if (id == "creation_time") {
    // Use the time put on queue (if it is enabled) as 0-10 has no standard way
    // to get message
    // creation time and we're not paying attention to the 1.0 creation time
    // yet.
    v = int64_t(msg.created() / 1000);
  } else if (id == "jms_type") {
    // Currently we can't distinguish between an empty JMSType and no JMSType
    // We'll assume for now that setting an empty JMSType doesn't make a lot of
    // sense
    const string jmsType = protoMessage.type();
    if (!jmsType.empty()) {
      returnedStrings.push_back(std::make_shared<std::string>(jmsType));
      v = *returnedStrings[returnedStrings.size() - 1];
    }
  } else {
    v = Value();
  }
  return v;
}

struct ValueHandler : public PropertyHandler {
  std::unordered_map<string, Value> &values;
  std::vector<std::shared_ptr<string>> &strings;

  ValueHandler(std::unordered_map<string, Value> &v, std::vector<std::shared_ptr<string>> &s) : values(v), strings(s) {}

  template <typename T>
  void handle(const std::string &key, const T &value) {
    values[key] = value;
  }

  void handleVoid(const std::string &) override {}
  void handleBool(const std::string &key, bool value) override { handle<bool>(key, value); }
  void handleUint8(const std::string &key, uint8_t value) override { handle<int64_t>(key, value); }
  void handleUint16(const std::string &key, uint16_t value) override { handle<int64_t>(key, value); }
  void handleUint32(const std::string &key, uint32_t value) override { handle<int64_t>(key, value); }
  void handleUint64(const std::string &key, uint64_t value) override {
    if (value > uint64_t(std::numeric_limits<int64_t>::max())) {
      handle<double>(key, double(value));
    } else {
      handle<int64_t>(key, value);
    }
  }
  void handleInt8(const std::string &key, int8_t value) override { handle<int64_t>(key, value); }
  void handleInt16(const std::string &key, int16_t value) override { handle<int64_t>(key, value); }
  void handleInt32(const std::string &key, int32_t value) override { handle<int64_t>(key, value); }
  void handleInt64(const std::string &key, int64_t value) override { handle<int64_t>(key, value); }
  void handleFloat(const std::string &key, float value) override { handle<double>(key, value); }
  void handleDouble(const std::string &key, double value) override { handle<double>(key, value); }
  void handleString(const std::string &key, const std::string &value) override {
    strings.push_back(std::make_shared<std::string>(value));
    handle(key, *strings[strings.size() - 1]);
  }
};

const Value &MessageSelectorEnv::value(const string &identifier) const {
  // Check for amqp prefix and strip it if present
  if (identifier.substr(0, 3) == "JMS") {
    Aliases::const_iterator equivalent = aliases.find(identifier);
    if (equivalent != aliases.end()) {
      returnedValues[identifier] = specialValue(equivalent->second);
    }
  } else if (returnedValues.find(identifier) == returnedValues.end()) {
    // Iterate over all the message properties
    ValueHandler handler(returnedValues, returnedStrings);
    msg.processProperties(handler, identifier);
    // Anything that wasn't found will have a void value now
  }
  const Value &v = returnedValues[identifier];
  return v;
}

Selector::Selector(const string &e) try : _parse(TopExpression::parse(e)), _expression(e) {
} catch (std::range_error &ex) {
  throw EXCEPTION("selector error", std::string(ex.what()), Proto::ERROR_INVALID_SELECTOR);
}

Selector::~Selector(){};

bool Selector::eval(const SelectorEnv &env) { return _parse->eval(env); }

bool Selector::filter(const upmq::broker::MessageDataContainer &msg) {
  const MessageSelectorEnv env(msg);
  return eval(env);
}
const std::string &Selector::expression() const { return _expression; }

std::shared_ptr<Selector> returnSelector(const std::string &e) { return std::make_shared<Selector>(e); }
}  // namespace storage
}  // namespace broker
}  // namespace upmq
