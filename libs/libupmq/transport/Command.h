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

#ifndef _UPMQ_COMMANDS_COMMAND_H_
#define _UPMQ_COMMANDS_COMMAND_H_

#include <transport/Config.h>
#include <string>

namespace upmq {
namespace transport {

class UPMQCPP_API Command {
 public:
  virtual ~Command() {}

  /**
   * Sets the Command Id of this Message
   * @param id Command Id
   */
  virtual void setCommandId(int id) = 0;

  /**
   * Gets the Command Id of this Message
   * @return Command Id
   */
  virtual int getCommandId() const = 0;

  virtual std::string getCurrId() = 0;
  virtual std::string getParentId() = 0;

  /**
   * Set if this Message requires a Response
   * @param required true if response is required
   */
  virtual void setResponseRequired(const bool required) = 0;

  /**
   * Is a Response required for this Command
   * @return true if a response is required.
   */
  virtual bool isResponseRequired() const = 0;

  /**
   * Returns a provider-specific string that provides information
   * about the contents of the command.
   */
  virtual std::string toString() const = 0;

  /*
   * This section contains a set of short-cut methods for determining if a
   * Command is of a certain type.  These are the most commonly used Commands
   * and we save several casts and some ugly code by just adding these here.
   */
  virtual bool isConnect() const = 0;
  virtual bool isConnected() const = 0;
  virtual bool isDisconnect() const = 0;
  virtual bool isClientInfo() const = 0;
  virtual bool isSession() const = 0;
  virtual bool isUnsession() const = 0;
  virtual bool isSubscription() const = 0;
  virtual bool isUnsubscription() const = 0;
  virtual bool isDestination() const = 0;
  virtual bool isUndestination() const = 0;
  virtual bool isSubscribe() const = 0;
  virtual bool isUnsubscribe() const = 0;
  virtual bool isSender() const = 0;
  virtual bool isUnsender() const = 0;
  virtual bool isBegin() const = 0;
  virtual bool isCommit() const = 0;
  virtual bool isAbort() const = 0;
  virtual bool isMessage() const = 0;
  virtual bool isAck() const = 0;
  virtual bool isReceipt() const = 0;
  virtual bool isError() const = 0;
  virtual bool isBrowser() const = 0;
  virtual bool isBrowserInfo() const = 0;
  virtual bool isPing() const = 0;
  virtual bool isPong() const = 0;
  virtual bool isResponse() const = 0;

  virtual Command *duplicate() = 0;
};
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_COMMANDS_COMMAND_H_*/
