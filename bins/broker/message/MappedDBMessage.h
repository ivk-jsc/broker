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

#ifndef BROKER_MAPPEDDBMESSAGE_H
#define BROKER_MAPPEDDBMESSAGE_H

#include <string>
#include "DBMSConnectionPool.h"
#include "MessageStorage.h"
#include "PropertyHandler.h"

namespace upmq {
namespace broker {
namespace storage {

namespace PropVals {
constexpr int message_id = 0;
constexpr int prop_name = 1;
constexpr int prop_type = 2;
constexpr int string_val = 3;
constexpr int char_val = 4;
constexpr int bool_val = 5;
constexpr int byte_val = 6;
constexpr int short_val = 7;
constexpr int int_val = 8;
constexpr int long_val = 9;
constexpr int float_val = 10;
constexpr int double_val = 11;
}  // namespace PropVals

class MappedDBMessage {
  typedef Poco::Tuple<std::string,                  // message_id
                      std::string,                  // prop_name
                      int,                          // prop_type
                      Poco::Nullable<std::string>,  // string_val
                      Poco::Nullable<int>,          // char_val
                      Poco::Nullable<bool>,         // bool_val
                      Poco::Nullable<int>,          // byte_val
                      Poco::Nullable<int>,          // short_val
                      Poco::Nullable<int>,          // int_val
                      Poco::Nullable<Poco::Int64>,  // long_val
                      Poco::Nullable<float>,        // float_val
                      Poco::Nullable<double> >      // double_val
      MsgProperty;

  std::string _messageID;
  Storage &_storage;

 public:
  MappedDBMessage(std::string messageID, Storage &storage);
  virtual ~MappedDBMessage() = default;
  bool persistent() const;
  std::string type() const;
  bool redelivered() const;
  int priority() const;
  std::string correlationID() const;
  const std::string &messageID() const;
  const std::string &destinationURI() const;
  std::string replyTo() const;
  int64_t expiration() const;
  int64_t creationTime() const;
  void processProperties(upmq::broker::PropertyHandler &handler, const std::string &identifier) const;
  DBMSConnection dbmsConnection;
};
}  // namespace storage
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_MAPPEDDBMESSAGE_H
