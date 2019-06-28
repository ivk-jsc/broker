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

#ifndef _CMSLISTENER_H_
#define _CMSLISTENER_H_

#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <cms/Session.h>
#include <deque>

class CMSListener : public cms::MessageListener, public cms::ExceptionListener {
 private:
  unsigned int numReceived;

  cms::Session *session;

  std::deque<std::pair<std::string, std::string>> inputMessges;

 public:
  explicit CMSListener(cms::Session *session);

  CMSListener(const CMSListener &) = delete;

  CMSListener &operator=(const CMSListener &) = delete;

  CMSListener(CMSListener &&) = delete;

  CMSListener &operator=(CMSListener &&) = delete;

  ~CMSListener() override;

  unsigned int getNumReceived() const { return this->numReceived; }

  virtual void reset();

  void asyncWaitForMessages(unsigned int count);

  void onException(const cms::CMSException &error) override;

  void onMessage(const cms::Message *message) override;

  std::string inputMessagesToString() const;
};

#endif /* _CMSLISTENER_H_ */
