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

#include <sstream>
#include "MessagesPageReplacer.h"
#include "MessagesRowPageReplacer.h"

MessagesPageReplacer::MessagesPageReplacer(std::string pageName, std::string destinationName, int destinationType)
    : TemplateParamReplacer(std::move(pageName)), _destination(std::move(destinationName)), _type(destinationType) {
  addReplacer(MakeStringify(rows), (TemplateParamReplacer::Callback)&MessagesPageReplacer::rowsReplacer);
}

MessagesPageReplacer::~MessagesPageReplacer() = default;

std::string MessagesPageReplacer::getH1() const { return "Destination " + _destination; }

std::string MessagesPageReplacer::rowsReplacer() {
  std::stringstream ss;
  MessagesRowPageReplacer rowPageReplacer("messages/row.html", "", 0, 0, 0, "", "", 0, "", "", 0, 0, "");
  ss << rowPageReplacer.replace();
  return ss.str();
}

std::string MessagesPageReplacer::type() const {
  // FIXME : replace with destination typename
  return std::to_string(_type);
}
