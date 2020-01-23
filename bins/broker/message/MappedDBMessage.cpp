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

#include "MappedDBMessage.h"
#include <Session.h>
#include "MiscDefines.h"

namespace upmq {
namespace broker {
namespace storage {

template <typename T>
T getPropertyValue(const std::string &propName, const std::string &messageID, const Storage &storage) {
  std::stringstream sql;
  T result;

  sql << "select " << propName << " from " << storage.messageTableID() << " where message_id = \'" << messageID << "\'";
  // not necessary to begin transaction, because this function always using in transaction
  storage::DBMSSession dbSession = dbms::Instance().dbmsSession();
  TRY_POCO_DATA_EXCEPTION { dbSession << sql.str(), Poco::Data::Keywords::into(result), Poco::Data::Keywords::now; }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't get message property : " + propName, sql.str(), ERROR_ON_GET_MESSAGE)
  return result;
}

MappedDBMessage::MappedDBMessage(std::string messageID, Storage &storage) : _messageID(std::move(messageID)), _storage(storage) {}

bool MappedDBMessage::persistent() const { return getPropertyValue<bool>("persistent", _messageID, _storage); }

std::string MappedDBMessage::type() const { return getPropertyValue<std::string>("type", _messageID, _storage); }

bool MappedDBMessage::redelivered() const { return (getPropertyValue<int>("delivery_count", _messageID, _storage) > 1); }

int MappedDBMessage::priority() const { return getPropertyValue<int>("priority", _messageID, _storage); }

std::string MappedDBMessage::correlationID() const { return getPropertyValue<std::string>("correlation_id", _messageID, _storage); }

const std::string &MappedDBMessage::messageID() const { return _messageID; }

const std::string &MappedDBMessage::destinationURI() const { return _storage.uri(); }

std::string MappedDBMessage::replyTo() const { return getPropertyValue<std::string>("reply_to", _messageID, _storage); }

int64_t MappedDBMessage::expiration() const { return getPropertyValue<Poco::Int64>("expiration", _messageID, _storage); }

int64_t MappedDBMessage::creationTime() const { return getPropertyValue<Poco::Int64>("created_time", _messageID, _storage); }

void MappedDBMessage::processProperties(PropertyHandler &handler, const std::string &identifier) const {
  std::vector<MsgProperty> properties;
  std::stringstream sql;

  sql << "select message_id"
      << ", property_name "
      << ", property_type"
      << ", value_string"
      << ", value_char"
      << ", value_bool"
      << ", value_byte"
      << ", value_short"
      << ", value_int"
      << ", value_long"
      << ", value_float"
      << ", value_double"
      << " from " << _storage.propertyTableID() << " where message_id = \'" << _messageID << "\'"
      << " and property_name = \'" << identifier << "\'"
      << ";";

  TRY_POCO_DATA_EXCEPTION {
    *dbmsConnection << sql.str(), Poco::Data::Keywords::into(properties), Poco::Data::Keywords::now;

    for (const auto &property : properties) {
      switch (property.get<PropVals::prop_type>()) {
        case Property::kValueString: {
          handler.handleString(property.get<PropVals::prop_name>(), property.get<PropVals::string_val>().value());
        } break;

        case Property::kValueChar: {
          handler.handleInt8(property.get<PropVals::prop_name>(), static_cast<int8_t>(property.get<PropVals::char_val>().value()));
        } break;

        case Property::kValueBool: {
          handler.handleBool(property.get<PropVals::prop_name>(), property.get<PropVals::bool_val>().value());
        } break;

        case Property::kValueByte: {
          handler.handleUint8(property.get<PropVals::prop_name>(), static_cast<uint8_t>(property.get<PropVals::byte_val>().value()));
        } break;

        case Property::kValueShort: {
          handler.handleInt16(property.get<PropVals::prop_name>(), static_cast<int16_t>(property.get<PropVals::short_val>().value()));
        } break;

        case Property::kValueInt: {
          handler.handleInt32(property.get<PropVals::prop_name>(), property.get<PropVals::int_val>().value());
        } break;

        case Property::kValueLong: {
          handler.handleInt64(property.get<PropVals::prop_name>(), property.get<PropVals::long_val>().value());
        } break;

        case Property::kValueFloat: {
          handler.handleFloat(property.get<PropVals::prop_name>(), property.get<PropVals::float_val>().value());
        } break;

        case Property::kValueDouble: {
          handler.handleDouble(property.get<PropVals::prop_name>(), property.get<PropVals::double_val>().value());
        } break;

        default:
          break;
      }
    }
  }
  CATCH_POCO_DATA_EXCEPTION_PURE("can't get message properties", sql.str(), ERROR_ON_GET_MESSAGE)
}
}  // namespace storage
}  // namespace broker
}  // namespace upmq
