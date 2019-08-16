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

#include "SWFApplication.h"
#include <stdexcept>
#include <thread>
#include <Poco/String.h>
#include "Broker.h"
#include "fake_cpp14.h"
namespace swf {

Application::Application(const std::string &name)
    : _name([&] {
        if (name.size() != 4) {
          throw std::runtime_error("invalid swf task name");
        }
        return name;
      }()) {
  memset(&_swfUPInfo, 0, sizeof(_swfUPInfo));
}
Application::~Application() = default;
const char *Application::name() const { return _name.c_str(); }
void Application::waitTermination() {
  bool isWaiting = false;
  std::string waitName(_name + "end");
  std::thread waitThread([&] {
    if (wait_event) {
      isWaiting = true;
      wait_event(_pstk, (void *)waitName.c_str(), static_cast<int>(waitName.size()), -1);
      isWaiting = false;

      if (BROKER::Instance().connectionsSize() > 0) {
        Poco::Thread::sleep(10000);
      }
      terminate();
    }
  });
  waitForTerminationRequest();
  if (isWaiting && post_event) {
    post_event(_pstk, (void *)waitName.c_str(), static_cast<int>(waitName.size()));
  }
  waitThread.join();
}

std::string Application::getOrgName() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.orgname[0]), sizeof(_swfUPInfo.orgname));
}

std::string Application::getOrgCode() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.orgcode[0]), sizeof(_swfUPInfo.orgcode));
}

std::string Application::getObjectName() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.objname[0]), sizeof(_swfUPInfo.objname));
}

std::string Application::getObjectCode() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.objcode[0]), sizeof(_swfUPInfo.objcode));
}

std::string Application::getModName() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.modname[0]), sizeof(_swfUPInfo.modname));
}

std::string Application::getModCode() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.modcode[0]), sizeof(_swfUPInfo.modcode));
}

std::string Application::getNVU() const { return std::string(reinterpret_cast<const char *>(&_swfUPInfo.nvu[0]), sizeof(_swfUPInfo.nvu)); }

std::string Application::getNetName() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.netname[0]), sizeof(_swfUPInfo.netname));
}

std::string Application::getUserStat() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.userstat[0]), sizeof(_swfUPInfo.userstat));
}

std::string Application::getUserFIO() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.userfio[0]), sizeof(_swfUPInfo.userfio));
}

std::string Application::getLogAddr() const { return std::string(reinterpret_cast<const char *>(&_swfUPInfo.logadr[0]), sizeof(_swfUPInfo.logadr)); }

std::string Application::getUserID() const { return std::string(reinterpret_cast<const char *>(&_swfUPInfo.userid[0]), sizeof(_swfUPInfo.userid)); }

std::string Application::getUserRang() const {
  return std::string(reinterpret_cast<const char *>(&_swfUPInfo.userrang[0]), sizeof(_swfUPInfo.userrang));
}

bool Application::useUpiter() const { return libplot != nullptr && regp != nullptr; }

void Application::initialize(Poco::Util::Application &self) {
  loadConfiguration();
  Poco::Util::ServerApplication::initialize(self);

  std::string path("libplot");
  std::string suffix = Poco::SharedLibrary::suffix();
  Poco::replaceInPlace(suffix, "d.", ".");
  path.append(suffix);
  try {
    libplot = std::make_unique<Poco::SharedLibrary>(path);
    regp = reinterpret_cast<f_regp>(libplot->getSymbol("regp"));
    unregp = reinterpret_cast<f_unregp>(libplot->getSymbol("unregp"));
    wait_event = reinterpret_cast<f_wait_event>(libplot->getSymbol("wait_event"));
    post_event = reinterpret_cast<f_post_event>(libplot->getSymbol("post_event"));
    if (libplot->hasSymbol("GetUPInfo")) {
      get_upinfo = reinterpret_cast<f_GetUPInfo>(libplot->getSymbol("GetUPInfo"));
    }
    if (libplot->hasSymbol("get_UP_info")) {
      get_upinfo = reinterpret_cast<f_GetUPInfo>(libplot->getSymbol("get_UP_info"));
    }
    if (!regp || !unregp || !wait_event || !post_event || !get_upinfo) {
      libplot->unload();
      libplot.reset(nullptr);
      return;
    }

    int rez = regp(const_cast<char *>(name()), &_pca, &_pstk);
    if (rez != 0) {
      regp = nullptr;
      unregp = nullptr;
      wait_event = nullptr;
      post_event = nullptr;
      get_upinfo = nullptr;
      return;
    }
    get_upinfo(_pstk, &_swfUPInfo);
  } catch (...) {
    libplot.reset(nullptr);
    regp = nullptr;
    unregp = nullptr;
    wait_event = nullptr;
    post_event = nullptr;
    get_upinfo = nullptr;
  }
}
void Application::uninitialize() {
  if (unregp) {
    unregp(_pca);
    libplot->unload();
  }
}

}  // namespace swf
