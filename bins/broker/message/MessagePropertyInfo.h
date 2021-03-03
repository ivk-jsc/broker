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
#include "MessageInfo.h"

namespace upmq {
namespace broker {

class MessagePropertyInfo {
 public:
  enum Field {
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
  template <MessagePropertyInfo::Field field>
  struct FieldInfo {
    static constexpr int POSITION = static_cast<int>(field);
    static constexpr Field TYPE = field;
  };

  using MsgTuple = Poco::Tuple<std::string,                       // messageID;
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
                               >;

  MsgTuple tuple{};

  explicit MessagePropertyInfo();
  explicit MessagePropertyInfo(MsgTuple msgTuple);
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
  void setMessageID(const std::string &value);
  void setPropertyName(const std::string &value);
  void setPropertyType(int value);
  void setValueString(const std::string &value);
  void setValueChar(int value);
  void setValueBool(bool value);
  void setValueByte(int value);
  void setValueShort(int value);
  void setValueInt(int value);
  void setValueLong(int64_t value);
  void setValueFloat(float value);
  void setValueDouble(double value);
  void setValueNull(bool value);
  void setValueBytes(const Poco::Data::BLOB &value);
  void setValueObject(const Poco::Data::BLOB &value);
  bool isNull() const;
  Field getNotNullValInfo() const;
  std::string dump() const;
  static std::string dump(const MsgTuple &tuple);
};
}  // namespace broker
}  // namespace upmq

namespace message {
namespace property {
using upmq::broker::MessagePropertyInfo;
using MessageId = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::message_id>;
using Name = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::property_name>;
using Type = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::property_type>;
namespace value {
using String = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_string>;
using Char = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_char>;
using Bool = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_bool>;
using Byte = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_byte>;
using Short = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_short>;
using Int = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_int>;
using Long = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_long>;
using Float = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_float>;
using Double = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_double>;
using Bytes = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_bytes>;
using Object = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::value_object>;
using IsNull = MessagePropertyInfo::FieldInfo<MessagePropertyInfo::is_null>;
}  // namespace value
}  // namespace property
}  // namespace message

#endif  // BROKER_MESSAGEPROPERTYINFO_H
