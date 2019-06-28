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

#ifndef _UPMQ_COMMANDS_RESPONSE_H_
#define _UPMQ_COMMANDS_RESPONSE_H_

// Turn off warning message for ignored exception specification
#ifdef _MSC_VER
#pragma warning(disable : 4290)
#endif

#include <transport/Command.h>
#include <transport/Config.h>

#include <libupmq/ProtoHeader.h>

namespace upmq {
namespace transport {

class UPMQCPP_API Response : public Command {
 public:
  virtual ~Response() {}

  virtual int getCorrelationId() const = 0;
  virtual void setCorrelationId(int newCorrelationId) = 0;

  virtual std::string getCurrId() = 0;
  virtual std::string getParentId() = 0;

  virtual bool isResponse() const { return true; }

  virtual void processReceipt() = 0;
  virtual Proto::Connected processConnected() = 0;
  virtual Proto::BrowserInfo processBrowserInfo() = 0;
};
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_COMMANDS_RESPONSE_H_*/
