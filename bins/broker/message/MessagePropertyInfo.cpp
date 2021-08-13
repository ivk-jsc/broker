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
#include <sstream>

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

MessagePropertyInfo::MessagePropertyInfo(const std::string &messageID) { tuple.set<message::property::MessageId::POSITION>(messageID); }
MessagePropertyInfo::MessagePropertyInfo(MessagePropertyInfo::MsgTuple msgTuple) : tuple(std::move(msgTuple)) {}
const std::string &MessagePropertyInfo::messageID() const { return tuple.get<message::property::MessageId::POSITION>(); }
const std::string &MessagePropertyInfo::propertyName() const { return tuple.get<message::property::Name::POSITION>(); }
int MessagePropertyInfo::propertyType() const { return (tuple.get<message::property::Type::POSITION>()) + static_cast<int>(Field::property_type); }
const std::string &MessagePropertyInfo::valueString() const { return tuple.get<message::property::value::String::POSITION>().value(); }
int MessagePropertyInfo::valueChar() const { return tuple.get<message::property::value::Char::POSITION>().value(); }
bool MessagePropertyInfo::valueBool() const { return tuple.get<message::property::value::Bool::POSITION>().value(); }
int MessagePropertyInfo::valueByte() const { return tuple.get<message::property::value::Byte::POSITION>().value(); }
int MessagePropertyInfo::valueShort() const { return tuple.get<message::property::value::Short::POSITION>().value(); }
int MessagePropertyInfo::valueInt() const { return tuple.get<message::property::value::Int::POSITION>().value(); }
int64_t MessagePropertyInfo::valueLong() const { return tuple.get<message::property::value::Long::POSITION>().value(); }
float MessagePropertyInfo::valueFloat() const { return tuple.get<message::property::value::Float::POSITION>().value(); }
double MessagePropertyInfo::valueDouble() const { return tuple.get<message::property::value::Double::POSITION>().value(); }
const Poco::Data::BLOB &MessagePropertyInfo::valueBytes() const { return tuple.get<message::property::value::Bytes::POSITION>().value(); }
const Poco::Data::BLOB &MessagePropertyInfo::valueObject() const { return tuple.get<message::property::value::Object::POSITION>().value(); }
bool MessagePropertyInfo::valueNull() const { return tuple.get<message::property::value::IsNull::POSITION>(); }
bool MessagePropertyInfo::isNull() const {
  switch (static_cast<MessagePropertyInfo::Field>(propertyType())) {
    case Field::message_id:
    case Field::property_name:
    case Field::property_type:
      return false;
    case Field::value_string:
      return tuple.get<message::property::value::String::POSITION>().isNull();
    case Field::value_char:
      return tuple.get<message::property::value::Char::POSITION>().isNull();
    case Field::value_bool:
      return tuple.get<message::property::value::Bool::POSITION>().isNull();
    case Field::value_byte:
      return tuple.get<message::property::value::Byte::POSITION>().isNull();
    case Field::value_short:
      return tuple.get<message::property::value::Short::POSITION>().isNull();
    case Field::value_int:
      return tuple.get<message::property::value::Int::POSITION>().isNull();
    case Field::value_long:
      return tuple.get<message::property::value::Long::POSITION>().isNull();
    case Field::value_float:
      return tuple.get<message::property::value::Float::POSITION>().isNull();
    case Field::value_double:
      return tuple.get<message::property::value::Double::POSITION>().isNull();
    case Field::value_bytes:
      return tuple.get<message::property::value::Bytes::POSITION>().isNull();
    case Field::value_object:
      return tuple.get<message::property::value::Object::POSITION>().isNull();
    case Field::is_null:
      return true;
  }
  return false;
}
MessagePropertyInfo::Field MessagePropertyInfo::getNotNullValInfo() const {
  if (!tuple.get<message::property::value::String::POSITION>().isNull()) {
    return message::property::value::String::TYPE;
  }
  if (!tuple.get<message::property::value::Char::POSITION>().isNull()) {
    return message::property::value::Char::TYPE;
  }
  if (!tuple.get<message::property::value::Bool::TYPE>().isNull()) {
    return message::property::value::Bool::TYPE;
  }
  if (!tuple.get<message::property::value::Byte::TYPE>().isNull()) {
    return message::property::value::Byte::TYPE;
  }
  if (!tuple.get<message::property::value::Short::TYPE>().isNull()) {
    return message::property::value::Short::TYPE;
  }
  if (!tuple.get<message::property::value::Int::TYPE>().isNull()) {
    return message::property::value::Int::TYPE;
  }
  if (!tuple.get<message::property::value::Long::TYPE>().isNull()) {
    return message::property::value::Long::TYPE;
  }
  if (!tuple.get<message::property::value::Float::TYPE>().isNull()) {
    return message::property::value::Float::TYPE;
  }
  if (!tuple.get<message::property::value::Double::TYPE>().isNull()) {
    return message::property::value::Double::TYPE;
  }
  if (!tuple.get<message::property::value::Bytes::TYPE>().isNull()) {
    return message::property::value::Bytes::TYPE;
  }
  if (!tuple.get<message::property::value::Object::TYPE>().isNull()) {
    return message::property::value::Object::TYPE;
  }
  return message::property::MessageId::TYPE;
}
void MessagePropertyInfo::setMessageID(const std::string &value) { tuple.set<message::property::MessageId::POSITION>(value); }
void MessagePropertyInfo::setPropertyName(const std::string &value) { tuple.set<message::property::Name::POSITION>(value); }
void MessagePropertyInfo::setPropertyType(int value) { tuple.set<message::property::Type::POSITION>(value); }
void MessagePropertyInfo::setValueString(const std::string &value) { tuple.set<message::property::value::String::POSITION>(value); }
void MessagePropertyInfo::setValueChar(int value) { tuple.set<message::property::value::Char::POSITION>(value); }
void MessagePropertyInfo::setValueBool(bool value) { tuple.set<message::property::value::Bool::POSITION>(value); }
void MessagePropertyInfo::setValueByte(int value) { tuple.set<message::property::value::Byte::POSITION>(value); }
void MessagePropertyInfo::setValueShort(int value) { tuple.set<message::property::value::Short::POSITION>(value); }
void MessagePropertyInfo::setValueInt(int value) { tuple.set<message::property::value::Int::POSITION>(value); }
void MessagePropertyInfo::setValueLong(int64_t value) { tuple.set<message::property::value::Long::POSITION>(value); }
void MessagePropertyInfo::setValueFloat(float value) { tuple.set<message::property::value::Float::POSITION>(value); }
void MessagePropertyInfo::setValueDouble(double value) { tuple.set<message::property::value::Double::POSITION>(value); }
void MessagePropertyInfo::setValueNull(bool value) { tuple.set<message::property::value::IsNull::POSITION>(value); }
void MessagePropertyInfo::setValueBytes(const Poco::Data::BLOB &value) { tuple.set<message::property::value::Bytes::POSITION>(value); }
void MessagePropertyInfo::setValueObject(const Poco::Data::BLOB &value) { tuple.set<message::property::value::Object::POSITION>(value); }
void MessagePropertyInfo::setValue(const Proto::Property &property) {
  switch (property.PropertyValue_case()) {
    case Proto::Property::kValueString: {
      setValueString(property.value_string());
    } break;

    case Proto::Property::kValueChar: {
      setValueChar(property.value_char());
    } break;

    case Proto::Property::kValueBool: {
      setValueBool(property.value_bool());
    } break;

    case Proto::Property::kValueByte: {
      setValueByte(property.value_byte());
    } break;

    case Proto::Property::kValueShort: {
      setValueShort(property.value_short());
    } break;

    case Proto::Property::kValueInt: {
      setValueInt(property.value_int());
    } break;

    case Proto::Property::kValueLong: {
      setValueLong(property.value_long());
    } break;

    case Proto::Property::kValueFloat: {
      setValueFloat(property.value_float());
    } break;

    case Proto::Property::kValueDouble: {
      setValueDouble(property.value_double());
    } break;

    case Proto::Property::kValueBytes: {
      setValueBytes(Poco::Data::BLOB((const unsigned char *)property.value_bytes().c_str(), property.value_bytes().size()));
    } break;

    case Proto::Property::kValueObject: {
      setValueObject(Poco::Data::BLOB((const unsigned char *)property.value_object().c_str(), property.value_object().size()));
    } break;

    default:
      break;
  }
}
std::string MessagePropertyInfo::dump() const {
  std::stringstream ss;
  ss << "<" << messageID() << "|" << propertyName() << "|" << propertyType() << "|";
  switch (static_cast<MessagePropertyInfo::Field>(propertyType())) {
    case Field::message_id:
    case Field::property_name:
    case Field::property_type:
      ss << "";
      break;
    case Field::value_string:
      if (!tuple.get<message::property::value::String::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::String::POSITION>().value();
      }
      break;
    case Field::value_char:
      if (!tuple.get<message::property::value::Char::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Char::POSITION>().value();
      }
      break;
    case Field::value_bool:
      if (!tuple.get<message::property::value::Bool::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Bool::POSITION>().value();
      }
      break;
    case Field::value_byte:
      if (!tuple.get<message::property::value::Byte::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Byte::POSITION>().value();
      }
      break;
    case Field::value_short:
      if (!tuple.get<message::property::value::Short::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Short::POSITION>().value();
      }
      break;
    case Field::value_int:
      if (!tuple.get<message::property::value::Int::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Int::POSITION>().value();
      }
      break;
    case Field::value_long:
      if (!tuple.get<message::property::value::Long::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Long::POSITION>().value();
      }
      break;
    case Field::value_float:
      if (!tuple.get<message::property::value::Float::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Float::POSITION>().value();
      }
      break;
    case Field::value_double:
      if (!tuple.get<message::property::value::Double::POSITION>().isNull()) {
        ss << tuple.get<message::property::value::Double::POSITION>().value();
      }
      break;
    case Field::value_bytes:
      if (!tuple.get<message::property::value::Bytes::POSITION>().isNull()) {
        ss << "xxxx:blob";
      }
      break;
    case Field::value_object:
      if (!tuple.get<message::property::value::Object::POSITION>().isNull()) {
        ss << "xxxx:blob";
      }
      break;
    case Field::is_null:
      ss << "is_null";
      break;
  }
  ss << ">";
  return ss.str();
}
std::string MessagePropertyInfo::dump(const MsgTuple &tuple) {
  MessagePropertyInfo messagePropertyInfo(tuple);
  return messagePropertyInfo.dump();
}
}  // namespace broker
}  // namespace upmq
