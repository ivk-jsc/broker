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

#include "MessagePropertyInfo.h"

namespace upmq {
namespace broker {

MessagePropertyInfo::MessagePropertyInfo() = default;
MessagePropertyInfo::MessagePropertyInfo(const std::string &messageID,
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
                                         bool isNull)
    : tuple(messageID,
            propertyName,
            propertyType,
            valueString,
            valueChar,
            valueBool,
            valueByte,
            valueShort,
            valueInt,
            valueLong,
            valueFloat,
            valueDouble,
            valueBytes,
            valueObject,
            isNull) {}

MessagePropertyInfo::MessagePropertyInfo(const std::string &messageID) { tuple.set<message::field_prop_message_id.position>(messageID); }
MessagePropertyInfo::MessagePropertyInfo(const MessagePropertyInfo::MsgTuple &tuple_) : tuple(tuple_) {}
MessagePropertyInfo::MessagePropertyInfo(MessagePropertyInfo::MsgTuple &&tuple_) : tuple(std::move(tuple_)) {}
const std::string &MessagePropertyInfo::messageID() const { return tuple.get<message::field_prop_message_id.position>(); }
const std::string &MessagePropertyInfo::propertyName() const { return tuple.get<message::field_prop_name.position>(); }
int MessagePropertyInfo::propertyType() const { return (tuple.get<message::field_prop_type.position>()) + static_cast<int>(Field::property_type); }
const std::string &MessagePropertyInfo::valueString() const { return tuple.get<message::field_val_string.position>().value(); }
int MessagePropertyInfo::valueChar() const { return tuple.get<message::field_val_char.position>().value(); }
bool MessagePropertyInfo::valueBool() const { return tuple.get<message::field_val_bool.position>().value(); }
int MessagePropertyInfo::valueByte() const { return tuple.get<message::field_val_byte.position>().value(); }
int MessagePropertyInfo::valueShort() const { return tuple.get<message::field_val_short.position>().value(); }
int MessagePropertyInfo::valueInt() const { return tuple.get<message::field_val_int.position>().value(); }
int64_t MessagePropertyInfo::valueLong() const { return tuple.get<message::field_val_long.position>().value(); }
float MessagePropertyInfo::valueFloat() const { return tuple.get<message::field_val_float.position>().value(); }
double MessagePropertyInfo::valueDouble() const { return tuple.get<message::field_val_double.position>().value(); }
const Poco::Data::BLOB &MessagePropertyInfo::valueBytes() const { return tuple.get<message::field_val_bytes.position>().value(); }
const Poco::Data::BLOB &MessagePropertyInfo::valueObject() const { return tuple.get<message::field_val_object.position>().value(); }
bool MessagePropertyInfo::valueNull() const { return tuple.get<message::field_is_null.position>(); }
bool MessagePropertyInfo::isNull() const {
  switch (static_cast<MessagePropertyInfo::Field>(propertyType())) {
    case Field::message_id:
    case Field::property_name:
    case Field::property_type:
      return false;
    case Field::value_string:
      return tuple.get<message::field_val_string.position>().isNull();
    case Field::value_char:
      return tuple.get<message::field_val_char.position>().isNull();
    case Field::value_bool:
      return tuple.get<message::field_val_bool.position>().isNull();
    case Field::value_byte:
      return tuple.get<message::field_val_byte.position>().isNull();
    case Field::value_short:
      return tuple.get<message::field_val_short.position>().isNull();
    case Field::value_int:
      return tuple.get<message::field_val_int.position>().isNull();
    case Field::value_long:
      return tuple.get<message::field_val_long.position>().isNull();
    case Field::value_float:
      return tuple.get<message::field_val_float.position>().isNull();
    case Field::value_double:
      return tuple.get<message::field_val_double.position>().isNull();
    case Field::value_bytes:
      return tuple.get<message::field_val_bytes.position>().isNull();
    case Field::value_object:
      return tuple.get<message::field_val_object.position>().isNull();
    case Field::is_null:
      return true;
  }
  return false;
}
MessagePropertyInfo::FieldInfo MessagePropertyInfo::getNotNullValInfo() const {
  if (!tuple.get<message::field_val_string.position>().isNull()) {
    return message::field_val_string;
  }
  if (!tuple.get<message::field_val_char.position>().isNull()) {
    return message::field_val_char;
  }
  if (!tuple.get<message::field_val_bool.position>().isNull()) {
    return message::field_val_bool;
  }
  if (!tuple.get<message::field_val_byte.position>().isNull()) {
    return message::field_val_byte;
  }
  if (!tuple.get<message::field_val_short.position>().isNull()) {
    return message::field_val_short;
  }
  if (!tuple.get<message::field_val_int.position>().isNull()) {
    return message::field_val_int;
  }
  if (!tuple.get<message::field_val_long.position>().isNull()) {
    return message::field_val_long;
  }
  if (!tuple.get<message::field_val_float.position>().isNull()) {
    return message::field_val_float;
  }
  if (!tuple.get<message::field_val_double.position>().isNull()) {
    return message::field_val_double;
  }
  if (!tuple.get<message::field_val_bytes.position>().isNull()) {
    return message::field_val_bytes;
  }
  if (!tuple.get<message::field_val_object.position>().isNull()) {
    return message::field_val_object;
  }
  return message::field_prop_message_id;
}
}  // namespace broker
}  // namespace upmq
