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

#ifndef _CMS_MESSAGEEOFEXCEPTION_H_
#define _CMS_MESSAGEEOFEXCEPTION_H_

#include <cms/CMSException.h>
#include <cms/Config.h>

namespace cms {

/**
 * This exception must be thrown when an unexpected end of stream has been
 * reached when a StreamMessage or BytesMessage is being read.
 *
 * @since 1.3
 */
class CMS_API MessageEOFException : public CMSException {
 public:
  MessageEOFException();

  MessageEOFException(const MessageEOFException &ex) noexcept;

  MessageEOFException(MessageEOFException &&ex) noexcept;

  MessageEOFException &operator=(const cms::MessageEOFException &) = delete;

  MessageEOFException &operator=(cms::MessageEOFException &&) = delete;

  MessageEOFException(const std::string &message);

  MessageEOFException(const std::string &message, const std::exception *cause);

  MessageEOFException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace);

  ~MessageEOFException() noexcept override;

  MessageEOFException *clone() override;
};
}  // namespace cms

#endif /*_CMS_MESSAGEEOFEXCEPTION_H_*/
