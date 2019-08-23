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

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/Path.h>
#include "TemplateParamReplacer.h"
#include "Defines.h"
#include "Configuration.h"

TemplateParamReplacer::TemplateParamReplacer(std::string pageName) : _pageName(std::move(pageName)) {}

TemplateParamReplacer::~TemplateParamReplacer() = default;

void TemplateParamReplacer::addReplacer(const std::string &param, TemplateParamReplacer::Callback callback) {
  _handlerMap.insert(make_pair(param, callback));
}

std::string TemplateParamReplacer::operator[](const std::string &param) { return (this->*_handlerMap[param])(); }

std::string TemplateParamReplacer::loadTemplate(const std::string &fileName) {
  std::string result;
  Poco::Path wwwPath;
  wwwPath.assign(CONFIGURATION::Instance().http().site).makeDirectory().append(fileName).makeFile();

  Poco::File f(wwwPath.toString());
  if (f.exists()) {
    Poco::FileInputStream fileIS(wwwPath.toString(), std::ios::binary);
    auto sz = f.getSize();
    std::vector<char> data;
    data.resize(static_cast<size_t>(sz) + 1);
    fileIS.read(&data[0], static_cast<std::streamsize>(sz));
    result.assign(&data[0], static_cast<size_t>(sz));
  }
  return result;
}

std::string TemplateParamReplacer::replace() {
  std::string htmlTemplate = loadTemplate(_pageName);
  if (!htmlTemplate.empty()) {
    for (auto &it : _handlerMap) {
      replaceStringInPlace(htmlTemplate, ("{" + it.first + "}"), (this->*it.second)());
    }
  }
  return htmlTemplate;
}

void TemplateParamReplacer::replaceStringInPlace(std::string &subject, const std::string &search, const std::string &replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

std::string TemplateParamReplacer::getH1() const { return _pageName; }
