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

#ifndef __MapMessageImpl_H__
#define __MapMessageImpl_H__

#include <cms/MapMessage.h>

#include "MessageTemplate.h"
#include "ProtoHeader.h"

using namespace std;

class MapMessageImpl : public MessageTemplate<cms::MapMessage> {
 public:
  MapMessageImpl();
  MapMessageImpl(Proto::ProtoMessage *header, unsigned char *body_buff, long long body_size, bool pr);
  MapMessageImpl(const UPMQCommand &command);
  MapMessageImpl(MapMessageImpl &other);

  virtual ~MapMessageImpl() noexcept;

  std::vector<std::string> getMapNames() const override;

  bool itemExists(const std::string &name) const override;

  cms::Message::ValueType getValueType(const std::string &name) const override;

  bool getBoolean(const std::string &name) const override;

  void setBoolean(const std::string &name, bool value) override;

  unsigned char getByte(const std::string &name) const override;

  void setByte(const std::string &name, char value) override;

  std::vector<signed char> getBytes(const std::string &name) const override;

  void setBytes(const std::string &name, const std::vector<signed char> &value) override;

  unsigned short getChar(const std::string &name) const override;

  void setChar(const std::string &name, unsigned short value) override;

  double getDouble(const std::string &name) const override;

  void setDouble(const std::string &name, double value) override;

  float getFloat(const std::string &name) const override;

  void setFloat(const std::string &name, float value) override;

  int getInt(const std::string &name) const override;

  void setInt(const std::string &name, int value) override;

  long long getLong(const std::string &name) const override;

  void setLong(const std::string &name, long long value) override;

  short getShort(const std::string &name) const override;

  void setShort(const std::string &name, short value) override;

  std::string getString(const std::string &name) const override;

  void setString(const std::string &name, const std::string &value) override;

  void setNull(const std::string &name);

  cms::MapMessage *clone() const override;
};

#endif /*__MapMessageImpl_H__*/
