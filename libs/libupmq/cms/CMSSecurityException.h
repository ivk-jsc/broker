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

#ifndef _CMS_CMSSECURITYEXCEPTION_H_
#define _CMS_CMSSECURITYEXCEPTION_H_

#include <cms/CMSException.h>
#include <cms/Config.h>

namespace cms {

/**
 * This exception must be thrown when a provider rejects a user name/password submitted
 * by a client. It may also be thrown for any case where a security restriction prevents
 * a method from completing.
 *
 * @since 1.3
 */
class CMS_API CMSSecurityException : public CMSException {
 public:
  CMSSecurityException();

  CMSSecurityException(const CMSSecurityException &ex) noexcept;

  CMSSecurityException(CMSSecurityException &&ex) noexcept;

  CMSSecurityException &operator=(const cms::CMSSecurityException &) = delete;

  CMSSecurityException &operator=(cms::CMSSecurityException &&) = delete;

  CMSSecurityException(const std::string &message);

  CMSSecurityException(const std::string &message, const std::exception *cause);

  CMSSecurityException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace);

  ~CMSSecurityException() noexcept override;

  CMSSecurityException *clone() override;
};
}  // namespace cms

#endif /*_CMS_CMSSECURITYEXCEPTION_H_*/
