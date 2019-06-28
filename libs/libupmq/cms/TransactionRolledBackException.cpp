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

#include "TransactionRolledBackException.h"

using namespace cms;

////////////////////////////////////////////////////////////////////////////////
TransactionRolledBackException::TransactionRolledBackException() = default;

////////////////////////////////////////////////////////////////////////////////
TransactionRolledBackException::TransactionRolledBackException(const TransactionRolledBackException &ex) noexcept = default;

TransactionRolledBackException::TransactionRolledBackException(TransactionRolledBackException &&ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
TransactionRolledBackException::TransactionRolledBackException(const std::string &message) : CMSException(message, nullptr) {}

////////////////////////////////////////////////////////////////////////////////
TransactionRolledBackException::TransactionRolledBackException(const std::string &message, const std::exception *cause) : CMSException(message, cause) {}

////////////////////////////////////////////////////////////////////////////////
TransactionRolledBackException::TransactionRolledBackException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace)
    : CMSException(message, cause, stackTrace) {}

////////////////////////////////////////////////////////////////////////////////
TransactionRolledBackException::~TransactionRolledBackException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
TransactionRolledBackException *TransactionRolledBackException::clone() { return new TransactionRolledBackException(*this); }
