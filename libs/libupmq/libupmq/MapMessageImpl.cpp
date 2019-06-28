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

#ifndef __MapMessageImpl_CPP__
#define __MapMessageImpl_CPP__

#include "MapMessageImpl.h"
#include "math.h"

MapMessageImpl::MapMessageImpl() : MessageTemplate<MapMessage>() {
  _header->mutable_message()->set_body_type(Proto::Body::kMapBody);
  _header->mutable_message()->set_sender_id("");
  _header->mutable_message()->set_session_id("");
  _header->mutable_message()->set_timestamp(0);
  _header->mutable_object_id()->assign("");
  _header->set_request_reply_id(-1);
  _body->mutable_map_body();  //-V773
}

MapMessageImpl::MapMessageImpl(MapMessageImpl &other) : MessageTemplate<MapMessage>(other) {}

MapMessageImpl::MapMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr) : MessageTemplate<cms::MapMessage>(header, body_buff, body_size) {
  DECAF_UNUSED_VAR(pr);
}

MapMessageImpl::MapMessageImpl(const UPMQCommand &command) : MessageTemplate<cms::MapMessage>(command) {}

MapMessageImpl::~MapMessageImpl() noexcept {}

std::vector<std::string> MapMessageImpl::getMapNames() const {
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  std::vector<std::string> names;
  for (google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().begin(); it != _body->map_body().value().end(); ++it) {
    names.push_back(it->first);
  }
  return names;
}

bool MapMessageImpl::itemExists(const std::string &name) const {
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  return it != _body->map_body().value().end();
}

cms::Message::ValueType MapMessageImpl::getValueType(const std::string &name) const {
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.is_null()) {
      return cms::Message::NULL_TYPE;
    }

    switch (it->second.PropertyValue_case()) {
      case Proto::Property::kValueString:
        return cms::Message::STRING_TYPE;

      case Proto::Property::kValueChar:
        return cms::Message::CHAR_TYPE;

      case Proto::Property::kValueBool:
        return cms::Message::BOOLEAN_TYPE;

      case Proto::Property::kValueByte:
        return cms::Message::BYTE_TYPE;

      case Proto::Property::kValueShort:
        return cms::Message::SHORT_TYPE;

      case Proto::Property::kValueInt:
        return cms::Message::INTEGER_TYPE;

      case Proto::Property::kValueLong:
        return cms::Message::LONG_TYPE;

      case Proto::Property::kValueFloat:
        return cms::Message::FLOAT_TYPE;

      case Proto::Property::kValueDouble:
        return cms::Message::DOUBLE_TYPE;

      case Proto::Property::kValueBytes:
        return cms::Message::BYTE_ARRAY_TYPE;

      case Proto::Property::PROPERTYVALUE_NOT_SET:
      default:
        throw cms::NullFormatException();
    }
  }
  throw cms::NullFormatException();
}

bool MapMessageImpl::getBoolean(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueBool) {
      return it->second.value_bool();
    } else {
      try {
        return this->convertPropertyToBool(it->second.PropertyValue_case(), it->second);
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setBoolean(const std::string &name, bool value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_bool(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

unsigned char MapMessageImpl::getByte(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueByte) {
      return static_cast<unsigned char>(it->second.value_byte());
    }
    try {
      return this->convertPropertyToByte(it->second.PropertyValue_case(), it->second);
    } catch (...) {
      throw cms::MessageFormatException("message: no possible conversion");
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setByte(const std::string &name, char value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_byte(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

std::vector<signed char> MapMessageImpl::getBytes(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueBytes) {
      string val = it->second.value_bytes();
      std::vector<signed char> vec(val.begin(), val.end());
      return vec;
    } else {
      try {
        // debug
        // return this->convertPropertyToString(it->second.PropertyValue_case(), it->second);
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setBytes(const std::string &name, const std::vector<signed char> &value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_bytes(string(value.begin(), value.end()));
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

unsigned short MapMessageImpl::getChar(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueChar) {
      return static_cast<unsigned short>(it->second.value_char());
    }
    try {
      // debug
      // convertPropertyToChar
    } catch (...) {
      throw cms::MessageFormatException("message: no possible conversion");
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setChar(const std::string &name, unsigned short value) {
  failIfReadOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_char(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

double MapMessageImpl::getDouble(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueDouble) {
      return it->second.value_double();
    } else {
      try {
        return this->convertPropertyToDouble(it->second.PropertyValue_case(), it->second);
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setDouble(const std::string &name, double value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_double(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

float MapMessageImpl::getFloat(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueFloat) {
      return it->second.value_float();
    } else {
      try {
        return this->convertPropertyToFloat(it->second.PropertyValue_case(), it->second);
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setFloat(const std::string &name, float value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_float(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

int MapMessageImpl::getInt(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueInt) {
      return it->second.value_int();
    } else {
      try {
        return this->convertPropertyToInt(it->second.PropertyValue_case(), it->second);
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setInt(const std::string &name, int value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_int(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

long long MapMessageImpl::getLong(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueLong) {
      return it->second.value_long();
    } else {
      try {
        return this->convertPropertyToLong(it->second.PropertyValue_case(), it->second);
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setLong(const std::string &name, long long value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_long(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

short MapMessageImpl::getShort(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueShort) {
      return static_cast<short>(it->second.value_short());
    }
    try {
      return this->convertPropertyToShort(it->second.PropertyValue_case(), it->second);
    } catch (...) {
      throw cms::MessageFormatException("message: no possible conversion");
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setShort(const std::string &name, short value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_short(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

std::string MapMessageImpl::getString(const std::string &name) const {
  failIfWriteOnlyBody();
  MapMessageImpl *foo = const_cast<MapMessageImpl *>(this);
  foo->unpackBody();

  google::protobuf::Map<string, Proto::Property>::const_iterator it = _body->map_body().value().find(name);
  if (it != _body->map_body().value().end()) {
    if (it->second.PropertyValue_case() == Proto::Property::kValueString) {
      return it->second.value_string();
    } else {
      try {
        convertPropertyToString(it->second.PropertyValue_case(), it->second);
        return _convertedValue;
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
  }
  throw cms::NullFormatException();
}

void MapMessageImpl::setString(const std::string &name, const std::string &value) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }
  try {
    Proto::Property property;
    property.set_value_string(value);
    property.set_is_null(false);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

void MapMessageImpl::setNull(const std::string &name) {
  failIfReadOnlyBody();
  if (name.empty()) {
    throw cms::CMSException("Message Property names must not be empty", nullptr);
  }

  try {
    Proto::Property property;
    property.set_is_null(true);

    (*_body->mutable_map_body()->mutable_value())[name] = property;
  } catch (...) {
    throw cms::CMSException("message: error");
  }
}

cms::MapMessage *MapMessageImpl::clone() const { return new MapMessageImpl(*const_cast<MapMessageImpl *>(this)); }

#endif  //__MapMessageImpl_CPP__
