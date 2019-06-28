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

#include "Exception.h"

namespace upmq {
namespace broker {

Exception::Exception(const std::string& info, const std::string& errDescription, int err, const std::string& file, int line)
    : runtime_error(std::string("exception : ") + file + std::string(" : ") + std::to_string(static_cast<long long>(line)) + std::string(" => ") + errDescription + std::string("(") +
                    std::to_string(static_cast<long long>(err)) + std::string(")") + std::string(" : ") + info),
      _error(err) {}

Exception::Exception(const Exception&) = default;

Exception::Exception(Exception&&) noexcept = default;

const char* Exception::what() const noexcept { return std::runtime_error::what(); }

std::string Exception::message() const { return std::runtime_error::what(); }

int Exception::error() const { return _error; }

Exception::~Exception() noexcept = default;
}  // namespace broker
}  // namespace upmq
