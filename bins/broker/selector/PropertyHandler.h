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

#ifndef UPMQSELECTOR_PROPERTYHANDLER_H
#define UPMQSELECTOR_PROPERTYHANDLER_H
#include <string>

namespace upmq {
namespace broker {

/**
 * Interface for processing entries in some map-like object
 */
class PropertyHandler {
 public:
  virtual ~PropertyHandler() {}
  virtual void handleVoid(const std::string &key) = 0;
  virtual void handleBool(const std::string &key, bool value) = 0;
  virtual void handleUint8(const std::string &key, uint8_t value) = 0;
  virtual void handleUint16(const std::string &key, uint16_t value) = 0;
  virtual void handleUint32(const std::string &key, uint32_t value) = 0;
  virtual void handleUint64(const std::string &key, uint64_t value) = 0;
  virtual void handleInt8(const std::string &key, int8_t value) = 0;
  virtual void handleInt16(const std::string &key, int16_t value) = 0;
  virtual void handleInt32(const std::string &key, int32_t value) = 0;
  virtual void handleInt64(const std::string &key, int64_t value) = 0;
  virtual void handleFloat(const std::string &key, float value) = 0;
  virtual void handleDouble(const std::string &key, double value) = 0;
  virtual void handleString(const std::string &key, const std::string &value) = 0;

 private:
};
}  // namespace broker
}  // namespace upmq

#endif  // UPMQSELECTOR_PROPERTYHANDLER_H
