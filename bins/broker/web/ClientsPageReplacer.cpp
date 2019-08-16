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
#include "ClientsPageReplacer.h"
#include "Defines.h"
#include "Exchange.h"
#include "ClientsRowPageReplacer.h"

ClientsPageReplacer::ClientsPageReplacer(std::string pageName, std::string destinationName, int destinationType)
    : TemplateParamReplacer(std::move(pageName)), _destination(std::move(destinationName)), _type(destinationType) {
  addReplacer(MakeStringify(rows), (TemplateParamReplacer::Callback)&ClientsPageReplacer::rowsReplacer);
}

std::string ClientsPageReplacer::rowsReplacer() {
  std::stringstream ss;
  auto infos = EXCHANGE::Instance().info();
  for (const auto &dest : infos) {
    if ((dest.name == _destination) && (_type == static_cast<int>(dest.type))) {
      for (const auto &subs : dest.subscriptions) {
        ClientsRowPageReplacer rowPageReplacer(
            "clients/row.html", subs.id, static_cast<int>(subs.type), static_cast<int>(subs.messages), subs.name, static_cast<int>(subs.running), "");
        ss << rowPageReplacer.replace();
      }
    }
  }
  return ss.str();
}

ClientsPageReplacer::~ClientsPageReplacer() = default;

std::string ClientsPageReplacer::getH1() const { return "Destination " + _destination; }
