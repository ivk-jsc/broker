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

#ifndef SWF_APPLICATION_H
#define SWF_APPLICATION_H
#include <Poco/Util/ServerApplication.h>
#include "swf_dynamic.h"
#include <Poco/SharedLibrary.h>
#include <memory>

namespace swf {

class Application : public Poco::Util::ServerApplication {
  std::string _name;
  PSTK_HANDLE _pstk = nullptr;
  PCA_HANDLE _pca = nullptr;
  SWF_UPINFO _swfUPInfo;
  std::unique_ptr<Poco::SharedLibrary> libplot = nullptr;
  f_regp regp = nullptr;
  f_unregp unregp = nullptr;
  f_wait_event wait_event = nullptr;
  f_post_event post_event = nullptr;
  f_GetUPInfo get_upinfo = nullptr;

 public:
  explicit Application(const std::string& name);
  ~Application() override;

  const char* name() const override;

  void waitTermination();

  std::string getOrgName() const;
  std::string getOrgCode() const;
  std::string getObjectName() const;
  std::string getObjectCode() const;
  std::string getModName() const;
  std::string getModCode() const;
  std::string getNVU() const;
  std::string getNetName() const;
  std::string getUserStat() const;
  std::string getUserFIO() const;
  std::string getLogAddr() const;
  std::string getUserID() const;
  std::string getUserRang() const;
  bool useUpiter() const;

 protected:
  void initialize(Poco::Util::Application& self) override;
  void uninitialize() override;
};

}  // namespace swf

#endif  // SWF_APPLICATION_H
