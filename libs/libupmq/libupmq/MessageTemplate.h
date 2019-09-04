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

#ifndef __MessageTemplate_H__
#define __MessageTemplate_H__

#include <cms/DeliveryMode.h>
#include <float.h>
#include <limits.h>
#include <iomanip>  //std::setprecision

#ifndef WIN32
#include <math.h>
#endif

#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

#include <cms/IllegalStateException.h>
#include <cms/MessageFormatException.h>
#include <cms/MessageNotReadableException.h>
#include <cms/MessageNotWriteableException.h>
#include <cms/NullFormatException.h>
#include <cms/NumberFormatException.h>
#include <cms/UnsupportedOperationException.h>

#include "ConnectionImpl.h"
#include "ConsumerImpl.h"
#include "DestinationImpl.h"
#include "ProtoHeader.h"
#include "SessionImpl.h"

#include <decaf/lang/System.h>
#include <transport/UPMQCommand.h>

using namespace std;
using namespace decaf::lang;
using namespace upmq::transport;

template <typename T>
class MessageTemplate : public T, public UPMQCommand {
 public:
  std::unique_ptr<DestinationImpl> _destination;
  std::unique_ptr<DestinationImpl> _reply_to;

  string::size_type _messageOffset;

  bool _bodyWritable;
  bool _bodyReadable;

  bool _propWritable;
  bool _propReadable;

  string _convertedValue;

  MessageTemplate() : UPMQCommand(), _messageOffset(0), _bodyWritable(true), _bodyReadable(false), _propWritable(true), _propReadable(false) {
    MessageTemplate<T>::setCMSTimeToLive(cms::Message::DEFAULT_TIME_TO_LIVE);
    MessageTemplate<T>::setCMSPriority(cms::Message::DEFAULT_MSG_PRIORITY);
    MessageTemplate<T>::setCMSDeliveryMode(cms::Message::DEFAULT_DELIVERY_MODE);
  }

  MessageTemplate(const MessageTemplate &o)
      : UPMQCommand(o),
        _messageOffset(0),
        _bodyWritable(o._bodyWritable),
        _bodyReadable(o._bodyReadable),
        _propWritable(o._propWritable),
        _propReadable(o._propReadable) {}

  MessageTemplate(const UPMQCommand &command)
      : UPMQCommand(command), _messageOffset(0), _bodyWritable(false), _bodyReadable(true), _propWritable(false), _propReadable(true) {}

  MessageTemplate(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size)
      : UPMQCommand(header, body_buff, body_size),
        _messageOffset(0),
        _bodyWritable(false),
        _bodyReadable(true),
        _propWritable(false),
        _propReadable(true) {}

  ~MessageTemplate() noexcept override {}

  void acknowledge() const override {
    if (static_cast<ConsumerImpl *>(_consumer) != nullptr) {
      static_cast<ConsumerImpl *>(_consumer)->checkClosed();

      try {
        Pointer<UPMQCommand> request(new UPMQCommand());
        request->getProtoMessage().set_object_id(static_cast<ConsumerImpl *>(_consumer)->getObjectId());

        Proto::Ack &ack = request->getAck();
        ack.set_receipt_id(_header->message().message_id());
        ack.set_message_id(_header->message().message_id());
        ack.set_session_id(static_cast<ConsumerImpl *>(_consumer)->_session->getObjectId());
        ack.set_destination_uri(_header->message().destination_uri());
        ack.set_subscription_name(static_cast<ConsumerImpl *>(_consumer)->getSubscription());

        if (!ack.IsInitialized()) {
          throw cms::CMSException("request not initialized");
        }

        static_cast<ConsumerImpl *>(_consumer)->_session->_connection->syncRequest(request.dynamicCast<Command>())->processReceipt();

        // cout << "> ack id " << _header->message().message_id() << endl;
      }
      CATCH_ALL_THROW_CMSEXCEPTION
    }
  }

  std::string getCMSMessageID() const override { return _header->message().message_id(); }

  void setCMSMessageID(const std::string &value) override { _header->mutable_message()->set_message_id(value); }

  std::string getCMSCorrelationID() const override { return _header->message().correlation_id(); }

  void setCMSCorrelationID(const std::string &newCorrelationId) override { _header->mutable_message()->set_correlation_id(newCorrelationId); }

  int getCMSDeliveryMode() const override {
    int ret = cms::DeliveryMode::NON_PERSISTENT;
    if (_header->message().persistent()) {
      ret = cms::DeliveryMode::PERSISTENT;
    }
    return ret;
  }

  void setCMSDeliveryMode(int mode) override {
    _header->mutable_message()->set_persistent(false);
    if (mode == cms::DeliveryMode::PERSISTENT) {
      _header->mutable_message()->set_persistent(true);
    }
  }

  long long getCMSExpiration() const override { return _header->message().expiration(); }

  void setCMSExpiration(long long expireTime) override {
    if (expireTime) {
      _header->mutable_message()->set_expiration(expireTime);
    } else {
      _header->mutable_message()->set_expiration(0);
      setCMSTimeToLive(0);
    }
  }

  virtual long long getCMSTimeToLive() const { return _header->message().timetolive(); }

  virtual void setCMSTimeToLive(long long expireTime) {
    if (expireTime) {
      _header->mutable_message()->set_timetolive(expireTime);
    } else {
      _header->mutable_message()->set_timetolive(0);
    }
  }

  int getCMSPriority() const override { return _header->message().priority(); }

  void setCMSPriority(int priority) override { _header->mutable_message()->set_priority(priority); }

  bool getCMSRedelivered() const override { return _header->message().redelivered(); }

  void setCMSRedelivered(bool redelivered) override { _header->mutable_message()->set_redelivered(redelivered); }

  long long getCMSTimestamp() const override { return _header->message().timestamp(); }

  void setCMSTimestamp(long long timeStamp) override { _header->mutable_message()->set_timestamp(timeStamp); }

  std::string getCMSType() const override { return _header->message().type(); }

  void setCMSType(const std::string &stype) override { _header->mutable_message()->set_type(stype); }

  const cms::Destination *getCMSDestination() const override {
    MessageTemplate *foo = const_cast<MessageTemplate *>(this);
    if (_header->message().has_destination_uri() && foo->_destination == nullptr) {
      foo->_destination.reset(
          new DestinationImpl(static_cast<ConsumerImpl *>(_consumer) == nullptr ? nullptr : static_cast<ConsumerImpl *>(_consumer)->_session,
                              _header->message().destination_uri()));
    }
    return foo->_destination.get();
  }

  void setCMSDestination(const cms::Destination *destination) override {
    if (destination != nullptr) {
      DestinationImpl *dest = dynamic_cast<DestinationImpl *>(const_cast<cms::Destination *>(destination));
      if (dest) {
        _destination.reset(
            new DestinationImpl(static_cast<ConsumerImpl *>(_consumer) == nullptr ? nullptr : static_cast<ConsumerImpl *>(_consumer)->_session,
                                dest->getName(),
                                dest->getType()));
        _header->mutable_message()->set_destination_uri(dest->getUri());
      } else {
        throw decaf::lang::exceptions::ClassCastException(__FILE__, __LINE__, "can't cast Destination to DestinationImpl");
      }
    } else {
      _header->mutable_message()->clear_destination_uri();
      _destination.reset(nullptr);
    }
  }

  const cms::Destination *getCMSReplyTo() const override {
    MessageTemplate *foo = const_cast<MessageTemplate *>(this);
    if (_header->mutable_message()->has_reply_to() && foo->_reply_to == nullptr) {
      foo->_reply_to.reset(
          new DestinationImpl(static_cast<ConsumerImpl *>(_consumer) == nullptr ? nullptr : static_cast<ConsumerImpl *>(_consumer)->_session,
                              _header->message().reply_to()));
    }
    return foo->_reply_to.get();
  }

  void setCMSReplyTo(const cms::Destination *destination) override {
    if (destination != nullptr) {
      DestinationImpl *dest = dynamic_cast<DestinationImpl *>(const_cast<cms::Destination *>(destination));
      if (dest) {
        _reply_to.reset(
            new DestinationImpl(static_cast<ConsumerImpl *>(_consumer) == nullptr ? nullptr : static_cast<ConsumerImpl *>(_consumer)->_session,
                                dest->getName(),
                                dest->getType()));
        _header->mutable_message()->set_reply_to(dest->getUri());
      } else {
        throw decaf::lang::exceptions::ClassCastException(__FILE__, __LINE__, "can't cast Destination to DestinationImpl");
      }
    } else {
      _header->mutable_message()->clear_reply_to();
      _reply_to.reset(nullptr);
    }
  }

  // add
  virtual int getCMSMessageType() const {
    // return _message->body().BodyType_case();
    return _header->message().body_type();
  }

  void clearBody() override {
    MessageTemplate *foo = const_cast<MessageTemplate *>(this);
    foo->unpackBody();

    switch (_body->BodyType_case()) {
      case Proto::Body::kTextBody:
        _body->mutable_text_body()->Clear();
        break;
      case Proto::Body::kMapBody:
        _body->mutable_map_body()->Clear();
        break;
      case Proto::Body::kObjectBody:
        _body->mutable_object_body()->Clear();
        break;
      case Proto::Body::kBytesBody:
        _body->mutable_bytes_body()->Clear();
        break;
      case Proto::Body::kStreamBody:
        _body->mutable_stream_body()->Clear();
        break;
      case Proto::Body::BODYTYPE_NOT_SET:
        _body->mutable_unknown_fields()->Clear();
        break;
      default:
        break;
    }

    this->_messageOffset = 0;
    this->_bodyWritable = true;
  }

  void clearProperties() override {
    _header->mutable_message()->clear_property();

    this->_propWritable = true;
    this->_propReadable = true;
  }

  std::vector<std::string> getPropertyNames() const override {
    std::vector<std::string> names;
    for (google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().begin();
         it != _header->message().property().end();
         ++it) {
      names.push_back(it->first);
    }
    return names;
  }

  void getPropertyNames(std::vector<std::string> &vect) const {
    for (google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().begin();
         it != _header->message().property().end();
         ++it) {
      vect.push_back(it->first);
    }
  }

  bool propertyExists(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
      return true;
    } else {
      return false;
    }
  }

  cms::Message::ValueType getPropertyValueType(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
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

  signed char getByteProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
      if (it->second.PropertyValue_case() == Proto::Property::kValueByte) {
        return static_cast<signed char>(it->second.value_byte());
      }
      try {
        return this->convertPropertyToByte(it->second.PropertyValue_case(), it->second);
      } catch (...) {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
    throw cms::NullFormatException();
  }

  float getFloatProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
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

  double getDoubleProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
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

  bool getBooleanProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
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

  int getIntProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
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

  long long getLongProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
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

  short getShortProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
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

  std::string getStringProperty(const std::string &name) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
      if (it->second.PropertyValue_case() == Proto::Property::kValueString) {
        return it->second.value_string();
      } else {
        try {
          this->convertPropertyToString(it->second.PropertyValue_case(), it->second);
          return _convertedValue;
        } catch (...) {
          throw cms::MessageFormatException("message: no possible conversion");
        }
      }
    }
    throw cms::NullFormatException();
  }

  unsigned short getCharProperty(const std::string &name) const {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
      if (it->second.PropertyValue_case() == Proto::Property::kValueChar) {
        return it->second.value_char();
      } else {
        try {
          // debug
          // return this->convertPropertyToChar(it->second.PropertyValue_case(), it->second);
        } catch (...) {
          throw cms::MessageFormatException("message: no possible conversion");
        }
      }
    }
    throw cms::NullFormatException();
  }

  virtual void setNullProperty(const std::string &name) {
    if (name.empty()) {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_is_null(true);
      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setBooleanProperty(const std::string &name, bool value) override {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_bool(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setByteProperty(const std::string &name, signed char value) override {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_byte(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setDoubleProperty(const std::string &name, double value) override {
    if (name.empty()) {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_double(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setFloatProperty(const std::string &name, float value) override {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_float(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setIntProperty(const std::string &name, int value) override {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_int(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setLongProperty(const std::string &name, long long value) override {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_long(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setShortProperty(const std::string &name, short value) override {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_short(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setStringProperty(const std::string &name, const std::string &value) override {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_string(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  void setCharProperty(const std::string &name, unsigned short value) {
    if (name == "") {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_char(value);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  int getBytesProperty(const std::string &name, unsigned char *buffer, int length) const override {
    google::protobuf::Map<string, Proto::Property>::const_iterator it = _header->message().property().find(name);
    if (it != _header->message().property().end()) {
      if (it->second.PropertyValue_case() == Proto::Property::kValueBytes) {
        const size_t valueSize = it->second.value_bytes().length();
        if (valueSize > size_t(length)) {
          throw cms::CMSException("IndexOutOfBoundsException");
        }
        memcpy(buffer, it->second.value_bytes().c_str(), valueSize);
        return static_cast<int>(valueSize);
      } else {
        throw cms::MessageFormatException("message: no possible conversion");
      }
    }
    throw cms::NullFormatException();
  }

  void setBytesProperty(const std::string &name, const unsigned char *value, int length) override {
    if (name.empty()) {
      throw cms::CMSException("Message Property names must not be empty", nullptr);
    }
    failIfReadOnlyProperties();

    try {
      Proto::Property property;
      property.set_value_bytes(value, length);
      property.set_is_null(false);

      (*_header->mutable_message()->mutable_property())[name] = property;
    } catch (...) {
      throw cms::CMSException("message: error");
    }
  }

  virtual void failIfReadOnlyProperties() const {
    if (!_propWritable) {
      throw cms::MessageNotWriteableException();
    }
  }

  virtual void failIfWriteOnlyProperties() const {
    if (!_propReadable) {
      throw cms::MessageNotReadableException();
    }
  }

  virtual void failIfReadOnlyBody() const {
    if (!_bodyWritable) {
      throw cms::MessageNotWriteableException();
    }
  }

  virtual void failIfWriteOnlyBody() const {
    if (!_bodyReadable) {
      throw cms::MessageNotReadableException();
    }
  }

  bool _isClosed() const { return this->_closed; }

  void _checkClosed() const {
    if (_isClosed()) {
      throw cms::IllegalStateException();
    }
  }

  void convertPropertyToString(Proto::Property::PropertyValueCase type, Proto::Property property) const {
    MessageTemplate *foo = const_cast<MessageTemplate *>(this);
    std::stringstream s;
    int i = 0;
    double valDouble;
    float valFloat;
    switch (type) {
      case Proto::Property::kValueBool:
        s << std::boolalpha << property.value_bool();
        break;
      case Proto::Property::kValueByte:
        s << property.value_byte();
        break;
      case Proto::Property::kValueShort:
        s << property.value_short();
        break;
      case Proto::Property::kValueInt:
        s << property.value_int();
        break;
      case Proto::Property::kValueLong:
        s << property.value_long();
        break;
      case Proto::Property::kValueFloat:
        valFloat = property.value_float();
#ifdef WIN32
        i = _fpclass(valFloat);
        switch (i) {
          case _FPCLASS_QNAN:
            s << "NaN";
            break;
          case _FPCLASS_NINF:
            s << " - Infinity";
            break;
          case _FPCLASS_PINF:
            s << "Infinity";
            break;
          default:
            s << std::setprecision(std::numeric_limits<float>::digits10 + 2) << valFloat;
            break;
        }
#else
        i = fpclassify(valFloat);
        switch (i) {
          case FP_NAN:
            s << "NaN";
            break;
          case FP_INFINITE:
            s << "Infinity";
            break;
          default:
            s << std::setprecision(std::numeric_limits<float>::digits10 + 2) << valFloat;
            break;
        }
#endif
        break;
      case Proto::Property::kValueDouble:
        valDouble = property.value_double();
#ifdef WIN32
        i = _fpclass(valDouble);
        switch (i) {
          case _FPCLASS_QNAN:
            s << "NaN";
            break;
          case _FPCLASS_NINF:
            s << " - Infinity";
            break;
          case _FPCLASS_PINF:
            s << "Infinity";
            break;
          default:
            s << std::setprecision(std::numeric_limits<double>::digits10 + 2) << valDouble;
            break;
        }
#else
        i = fpclassify(valDouble);
        switch (i) {
          case FP_NAN:
            s << "NaN";
            break;
          case FP_INFINITE:
            s << "Infinity";
            break;
          default:
            s << std::setprecision(std::numeric_limits<double>::digits10 + 2) << valDouble;
            break;
        }
#endif
        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }

    foo->_convertedValue.clear();
    foo->_convertedValue.assign(s.str());
  }

  int convertPropertyToInt(Proto::Property::PropertyValueCase type, Proto::Property property) const {
    int s;
    long long longVal;
    char *pEnd;
    string pStart;
    switch (type) {
      case Proto::Property::kValueByte:
        s = (int)property.value_byte();
        break;
      case Proto::Property::kValueShort:
        s = (int)property.value_short();
        break;
      case Proto::Property::kValueString:
        pStart = property.value_string();
        longVal = strtoll(pStart.c_str(), &pEnd, 10);
        if (pEnd == pStart.c_str() || *pEnd != '\0') {
          throw cms::NumberFormatException("message: no possible conversion");
        } else if (longVal > INT_MAX || longVal < INT_MIN) {
          throw cms::NumberFormatException("message: no possible conversion");
        } else {
          s = (int)longVal;
        }
        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }
    return s;
  }

  long long convertPropertyToLong(Proto::Property::PropertyValueCase type, Proto::Property property) const {
    long long s;
    long long longVal;
    char *pEnd;
    string pStart;
    switch (type) {
      case Proto::Property::kValueByte:
        s = static_cast<long long>(property.value_byte());
        break;
      case Proto::Property::kValueShort:
        s = static_cast<long long>(property.value_short());
        break;
      case Proto::Property::kValueInt:
        s = static_cast<long long>(property.value_int());
        break;
      case Proto::Property::kValueString:
        pStart = property.value_string();
        longVal = strtoll(pStart.c_str(), &pEnd, 10);
        if (pEnd == pStart.c_str() || *pEnd != '\0') {
          throw cms::NumberFormatException("message: no possible conversion");
        }

        s = static_cast<long long>(longVal);

        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }
    return s;
  }

  short convertPropertyToShort(Proto::Property::PropertyValueCase type, Proto::Property property) const {
    short s;
    long longVal;
    char *pEnd;
    string pStart;
    switch (type) {
      case Proto::Property::kValueByte:
        s = static_cast<short>(property.value_byte());
        break;
      case Proto::Property::kValueString:
        pStart = property.value_string();
        longVal = strtol(pStart.c_str(), &pEnd, 10);
        if (pEnd == pStart.c_str() || *pEnd != '\0') {
          throw cms::NumberFormatException("message: no possible conversion");
        }
        if ((longVal == LONG_MAX || longVal == LONG_MIN) && errno == ERANGE) {
          throw cms::NumberFormatException("message: no possible conversion");
        }
        if (longVal > SHRT_MAX || longVal < SHRT_MIN) {
          throw cms::NumberFormatException("message: no possible conversion");
        }
        s = static_cast<short>(longVal);
        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }
    return s;
  }

  double convertPropertyToDouble(Proto::Property::PropertyValueCase type, const Proto::Property &property) const {
    //    MessageTemplate *foo = const_cast<MessageTemplate *>(this);
    double s;
    double longVal;
    char *pEnd;
    string pStart;
    switch (type) {
      case Proto::Property::kValueFloat:
        s = property.value_float();
        break;
      case Proto::Property::kValueString:
        pStart = property.value_string();
        longVal = strtod(pStart.c_str(), &pEnd);
        if (pEnd == pStart.c_str() || *pEnd != '\0') {
          throw cms::NumberFormatException("message: no possible conversion");
        }
        s = static_cast<double>(longVal);
        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }
    return s;
  }

  float convertPropertyToFloat(Proto::Property::PropertyValueCase type, const Proto::Property &property) const {
    float s;
    double longVal;
    char *pEnd;
    string pStart;

    switch (type) {
      case Proto::Property::kValueString:
        pStart = property.value_string();
        longVal = strtod(pStart.c_str(), &pEnd);
        if (pEnd == pStart.c_str() || *pEnd != '\0') {
          throw cms::NumberFormatException("message: no possible conversion");
        }
        s = static_cast<float>(longVal);
        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }
    return s;
  }

  unsigned char convertPropertyToByte(Proto::Property::PropertyValueCase type, const Proto::Property &property) const {
    unsigned char s;
    long longVal;
    char *pEnd;
    string pStart;
    switch (type) {
      case Proto::Property::kValueString:
        pStart = property.value_string();
        longVal = strtol(pStart.c_str(), &pEnd, 10);
        if (pEnd == pStart.c_str() || *pEnd != '\0') {
          throw cms::NumberFormatException("message: no possible conversion");
        }
        if (longVal > SCHAR_MAX || longVal < SCHAR_MIN) {
          throw cms::NumberFormatException("message: no possible conversion");
        }
        s = static_cast<uint8_t>(longVal);

        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }
    return s;
  }

  bool convertPropertyToBool(Proto::Property::PropertyValueCase type, const Proto::Property &property) const {
    string s;
    switch (type) {
      case Proto::Property::kValueString:
        s = property.value_string();
        if (!s.compare("true")) {
          return true;
        } else if (!s.compare("false")) {
          return false;
        } else if (!s.compare("True")) {
          return true;
        } else if (!s.compare("False")) {
          return false;
        } else if (!s.compare("TRUE")) {
          return true;
        } else if (!s.compare("FALSE")) {
          return false;
        }
        break;
      case Proto::Property::PROPERTYVALUE_NOT_SET:
        throw cms::NullFormatException("null: no possible conversion");
      default:
        throw cms::MessageFormatException("message: no possible conversion");
    }
    return false;
  }

  void *getProtoUnpackedBody() override { return _bodyBuff; }

  long long getProtoUnpackedBodySize() override { return _bodyBuffSize; }

  void unpackBody() {
    if (_body == nullptr && _bodyBuff != nullptr && _bodyBuffSize != 0) {
      _body = new Proto::Body();
      _body->ParseFromArray(_bodyBuff, int(_bodyBuffSize));

      delete[] _bodyBuff;

      _bodyBuff = nullptr;
      _bodyBuffSize = 0;
    } else if (_body != nullptr && _bodyBuff == nullptr && _bodyBuffSize == 0) {
    } else {
      throw cms::CMSException("message format error");
    }
  }

  bool isExpired() override {
    long long expireTime = getCMSExpiration();
    bool expired = expireTime > 0 && System::currentTimeMillis() > expireTime;
    if (expired) {
      acknowledge();
    }
    return expired;
  }

  void setReadable() override {
    this->_propReadable = true;
    this->_bodyReadable = true;
  }

  void setWritable() override {
    this->_propWritable = true;
    this->_bodyWritable = true;
  }
};

#endif /* __MessageTemplate_H__ */
