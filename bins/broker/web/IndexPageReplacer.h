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

#ifndef IVK_UPMQ_INDEXPAGEREPLACER_H
#define IVK_UPMQ_INDEXPAGEREPLACER_H

#include "TemplateParamReplacer.h"

class IndexPageReplacer : public TemplateParamReplacer {
 public:
  enum IndexParam { indexH1 = 0, indexContent };

  explicit IndexPageReplacer(std::string pageName, TemplateParamReplacer *templateReplacer = nullptr);

  void setChildReplacer(TemplateParamReplacer *templateReplacer);

  std::string h1Replacer();

  std::string contentReplacer();

 private:
  TemplateParamReplacer *_templateReplacer;
};

#endif  // IVK_UPMQ_INDEXPAGEREPLACER_H
