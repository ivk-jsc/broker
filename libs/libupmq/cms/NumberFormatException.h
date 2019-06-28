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

#ifndef _CMS_NUMBERFORMATEXCEPTION_H_
#define _CMS_NUMBERFORMATEXCEPTION_H_

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
class CMS_API NumberFormatException : public CMSException {
 public:
  NumberFormatException();

  NumberFormatException(const NumberFormatException &ex) noexcept;

  NumberFormatException(NumberFormatException &&ex) noexcept;

  NumberFormatException &operator=(const cms::NumberFormatException &) = delete;

  NumberFormatException &operator=(cms::NumberFormatException &&) = delete;

  NumberFormatException(const std::string &message);

  NumberFormatException(const std::string &message, const std::exception *cause);

  NumberFormatException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace);

  ~NumberFormatException() noexcept override;

  NumberFormatException *clone() override;
};
}  // namespace cms

#endif /*_CMS_NUMBERFORMATEXCEPTION_H_*/
