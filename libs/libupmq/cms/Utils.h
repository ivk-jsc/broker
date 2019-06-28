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

#ifndef _CMS_UTILS_H_
#define _CMS_UTILS_H_

#include <cms/Config.h>
#include <cms/Message.h>

namespace cms {

class CMS_API Utils {
 public:
  static std::vector<char> serialize(const cms::Message *m);
  static void serialize(const cms::Message *message, std::ostream &outStream);
  template <typename T>
  static T *deserialize(const char *data, long long size);
  template <typename T>
  static T *deserialize(std::istream &inputStream);
  static std::string toJsonString(const cms::Message *message);
  static std::string toPrettyJsonString(const cms::Message *message);

 private:
  static bool isMessageValid(const cms::Message *message);
};
}  // namespace cms

#endif /*_CMS_UTILS_H_*/
