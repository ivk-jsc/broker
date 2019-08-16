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

#ifndef BROKER_MESSAGEPROPERTYINFO_H
#define BROKER_MESSAGEPROPERTYINFO_H

#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/LOB.h>
namespace Poco {
namespace Data = SQL;
}
#else
#include <Poco/Data/LOB.h>
#endif
#include <Poco/Nullable.h>
#include <Poco/Tuple.h>
#include <string>
#include "MessageDefines.h"

namespace upmq {
namespace broker {

class MessagePropertyInfo {
 public:
  enum class Field : int {
    message_id = 0,
    property_name,  // 1
    property_type,  // 2
    value_string,   // 3
    value_char,     // 4
    value_bool,     // 5
    value_byte,     // 6
    value_short,    // 7
    value_int,      // 8
    value_long,     // 9
    value_float,    // 10
    value_double,   // 11
    value_bytes,    // 12
    value_object,   // 13
    is_null
  };
  struct FieldInfo {
    constexpr explicit FieldInfo(Field ec) : ec(ec), position(static_cast<int>(ec)) {}
    const Field ec;
    const int position;
  };

  typedef Poco::Tuple<std::string,                       // messageID;
                      std::string,                       // property_name;
                      int,                               // property_type;
                      Poco::Nullable<std::string>,       // value_string;
                      Poco::Nullable<int>,               // value_char;
                      Poco::Nullable<bool>,              // value_bool;
                      Poco::Nullable<int>,               // value_byte;
                      Poco::Nullable<int>,               // value_short;
                      Poco::Nullable<int>,               // value_int;
                      Poco::Nullable<Poco::Int64>,       // value_long;
                      Poco::Nullable<float>,             // value_float;
                      Poco::Nullable<double>,            // value_double;
                      Poco::Nullable<Poco::Data::BLOB>,  // value_bytes;
                      Poco::Nullable<Poco::Data::BLOB>,  // value_object;
                      bool                               // is_null;
                      >
      MsgTuple;

  MsgTuple tuple{};

  explicit MessagePropertyInfo();
  explicit MessagePropertyInfo(const MsgTuple &tuple);
  explicit MessagePropertyInfo(MsgTuple &&tuple);
  explicit MessagePropertyInfo(const std::string &messageID);
  MessagePropertyInfo(const std::string &messageID,
                      const std::string &propertyName,
                      int propertyType,
                      const std::string &valueString,
                      int valueChar,
                      bool valueBool,
                      int valueByte,
                      int valueShort,
                      int valueInt,
                      int64_t valueLong,
                      float valueFloat,
                      double valueDouble,
                      const Poco::Data::BLOB &valueBytes,
                      const Poco::Data::BLOB &valueObject,
                      bool isNull);

  const std::string &messageID() const;
  const std::string &propertyName() const;
  int propertyType() const;
  const std::string &valueString() const;
  int valueChar() const;
  bool valueBool() const;
  int valueByte() const;
  int valueShort() const;
  int valueInt() const;
  int64_t valueLong() const;
  float valueFloat() const;
  double valueDouble() const;
  bool valueNull() const;
  const Poco::Data::BLOB &valueBytes() const;
  const Poco::Data::BLOB &valueObject() const;
  bool isNull() const;
  FieldInfo getNotNullValInfo() const;
};
}  // namespace broker
}  // namespace upmq

namespace message {
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_prop_message_id =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::message_id);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_prop_name =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::property_name);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_prop_type =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::property_type);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_string =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_string);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_char =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_char);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_bool =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_bool);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_byte =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_byte);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_short =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_short);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_int =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_int);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_long =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_long);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_float =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_float);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_double =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_double);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_bytes =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_bytes);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_val_object =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::value_object);
static constexpr upmq::broker::MessagePropertyInfo::FieldInfo field_is_null =
    upmq::broker::MessagePropertyInfo::FieldInfo(upmq::broker::MessagePropertyInfo::Field::is_null);
}  // namespace message

#endif  // BROKER_MESSAGEPROPERTYINFO_H
