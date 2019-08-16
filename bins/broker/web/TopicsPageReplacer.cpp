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

#include "TopicsPageReplacer.h"
#include <Defines.h>
#include <sstream>
#include <Exchange.h>

TopicsPageReplacer::TopicsPageReplacer(std::string pageName) : TemplateParamReplacer(std::move(pageName)) {
  addReplacer(MakeStringify(rows), (TemplateParamReplacer::Callback)&TopicsPageReplacer::rowsReplacer);
}

std::string TopicsPageReplacer::rowsReplacer() {
  auto infos = EXCHANGE::Instance().info();
  std::stringstream ss;
  for (const auto &dest : infos) {
    if ((dest.type == upmq::broker::Destination::Type::TOPIC) || (dest.type == upmq::broker::Destination::Type::TEMPORARY_TOPIC)) {
      DestinationRowPageReplacer rowPageReplacer("destinations/row.html",
                                                 dest.name,
                                                 static_cast<int>(dest.type),
                                                 static_cast<int>(dest.subscriptions.size()),
                                                 dest.created,
                                                 dest.dataPath,
                                                 dest.uri,
                                                 dest.messages);
      ss << rowPageReplacer.replace();
    }
  }
  return ss.str();
}

std::string TopicsPageReplacer::getH1() const { return "Destinations"; }
