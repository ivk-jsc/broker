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

#include "CMSListener.h"
#include <gtest/gtest.h>

#include "CMSProvider.h"

using namespace cms;

////////////////////////////////////////////////////////////////////////////////
CMSListener::CMSListener(cms::Session *session_) : numReceived(), session(session_) {  // mutex(),
  this->reset();
}

////////////////////////////////////////////////////////////////////////////////
CMSListener::~CMSListener() = default;

////////////////////////////////////////////////////////////////////////////////
void CMSListener::reset() {
  this->numReceived = 0;
  this->inputMessges.clear();
}

////////////////////////////////////////////////////////////////////////////////
void CMSListener::asyncWaitForMessages(unsigned int count) {
  try {
    unsigned int retryNum = 0;
    while (true) {  //
      if (numReceived >= count) {
        break;
      }
      if (retryNum >= count) {
        break;
      }
      cmsSleep(3000);
      retryNum++;
    }

  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

////////////////////////////////////////////////////////////////////////////////
void CMSListener::onMessage(const cms::Message *message) {
  if (session->getAcknowledgeMode() == cms::Session::CLIENT_ACKNOWLEDGE) {
    try {
      message->acknowledge();
    } catch (CMSException &ex) {
      EXPECT_TRUE(false) << ex.getStackTraceString();
    }
  }

  // Got a text message.
  const auto *txtMsg = dynamic_cast<const cms::TextMessage *>(message);
  std::pair<std::string, std::string> msg;
  msg.first = message->getCMSMessageID();
  if (txtMsg != nullptr) {
    numReceived++;
    msg.second = txtMsg->getText();
  }
  inputMessges.emplace_back(std::move(msg));
}

std::string CMSListener::inputMessagesToString() const {
  std::string result;
  std::for_each(inputMessges.begin(), inputMessges.end(), [&result](const std::pair<std::string, std::string> &msg) {
    result.append("\n[").append(msg.first).append(":").append(msg.second).append("]");
  });
  return result;
}

////////////////////////////////////////////////////////////////////////////////
void CMSListener::onException(const cms::CMSException &error) { EXPECT_TRUE(false) << error.getStackTraceString(); }
