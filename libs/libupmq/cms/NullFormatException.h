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

#ifndef _CMS_NULLFORMATEXCEPTION_H_
#define _CMS_NULLFORMATEXCEPTION_H_

#include <cms/CMSException.h>
#include <cms/Config.h>

namespace cms {

/**
 * This exception must be thrown when a CMS client attempts to use a data type not
 * supported by a message or attempts to read data in a message as the wrong type. It
 * must also be thrown when equivalent type errors are made with message property values.
 *
 * @since 1.3
 */
class CMS_API NullFormatException : public CMSException {
 public:
  NullFormatException();

  NullFormatException(const NullFormatException &ex) noexcept;

  NullFormatException(NullFormatException &&ex) noexcept;

  NullFormatException &operator=(const cms::NullFormatException &) = delete;

  NullFormatException &operator=(cms::NullFormatException &&) = delete;

  NullFormatException(const std::string &message);

  NullFormatException(const std::string &message, const std::exception *cause);

  NullFormatException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace);

  ~NullFormatException() noexcept override;

  NullFormatException *clone() override;
};
}  // namespace cms

#endif /*_CMS_NULLFORMATEXCEPTION_H_*/
