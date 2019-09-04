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

#ifndef _CMS_TRANSACTIONINPROGRESSEXCEPTION_H_
#define _CMS_TRANSACTIONINPROGRESSEXCEPTION_H_

#include <cms/CMSException.h>
#include <cms/Config.h>

namespace cms {

/**
 * This exception is thrown when an operation is invalid because a transaction is in progress.
 * For instance, an attempt to call Session::commit when a session is part of a distributed
 * transaction should throw a TransactionInProgressException.
 *
 * @since 2.3
 */
class CMS_API TransactionInProgressException : public cms::CMSException {
 public:
  TransactionInProgressException();

  TransactionInProgressException(const TransactionInProgressException &ex) noexcept;

  TransactionInProgressException(TransactionInProgressException &&ex) noexcept;

  TransactionInProgressException &operator=(const cms::TransactionInProgressException &) = delete;

  TransactionInProgressException &operator=(cms::TransactionInProgressException &&) = delete;

  TransactionInProgressException(const std::string &message);

  TransactionInProgressException(const std::string &message, const std::exception *cause);

  TransactionInProgressException(const std::string &message,
                                 const std::exception *cause,
                                 const std::vector<triplet<std::string, std::string, int> > &stackTrace);

  ~TransactionInProgressException() noexcept override;

  TransactionInProgressException *clone() override;
};
}  // namespace cms

#endif /* _CMS_TRANSACTIONINPROGRESSEXCEPTION_H_ */
