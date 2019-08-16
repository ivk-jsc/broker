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

#ifndef IVK_UPMQ_TOPICSPAGEREPLACER_H
#define IVK_UPMQ_TOPICSPAGEREPLACER_H

#include "TemplateParamReplacer.h"
#include "DestinationRowPageReplacer.h"

class StorageRegestry;

class TopicsPageReplacer : public TemplateParamReplacer {
 public:
  enum TopicParam { rows = 0 };

  explicit TopicsPageReplacer(std::string pageName);

  std::string rowsReplacer();

  std::string getH1() const override;
};

#endif  // IVK_UPMQ_TOPICSPAGEREPLACER_H
