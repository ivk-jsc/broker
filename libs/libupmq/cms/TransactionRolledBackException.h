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

#ifndef _CMS_TRANSACTIONROLLEDBACKEXCEPTION_H_
#define _CMS_TRANSACTIONROLLEDBACKEXCEPTION_H_

#include <cms/CMSException.h>
#include <cms/Config.h>

namespace cms {

/**
 * This exception must be thrown when a call to Session.commit results in a rollback of the
 * current transaction.
 *
 * @since 2.3
 */
class CMS_API TransactionRolledBackException : public cms::CMSException {
 public:
  TransactionRolledBackException();

  TransactionRolledBackException(const TransactionRolledBackException &ex) noexcept;

  TransactionRolledBackException(TransactionRolledBackException &&ex) noexcept;

  TransactionRolledBackException &operator=(const cms::TransactionRolledBackException &) = delete;

  TransactionRolledBackException &operator=(cms::TransactionRolledBackException &&) = delete;

  TransactionRolledBackException(const std::string &message);

  TransactionRolledBackException(const std::string &message, const std::exception *cause);

  TransactionRolledBackException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace);

  ~TransactionRolledBackException() noexcept override;

  TransactionRolledBackException *clone() override;
};
}  // namespace cms

#endif /* _CMS_TRANSACTIONROLLEDBACKEXCEPTION_H_ */
