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
    : _message(std::string("exception : ")
                   .append(file)
                   .append(" : ")
                   .append(std::to_string(line))
                   .append(" => ")
                   .append(errDescription)
                   .append("(")
                   .append(std::to_string(err))
                   .append(")")
                   .append(" : ")
                   .append(info)),
      _error(err) {}

Exception::Exception(const Exception&) = default;

Exception::Exception(Exception&&) noexcept = default;

Exception& Exception::operator=(const Exception&) = default;

Exception& Exception::operator=(Exception&&) noexcept = default;

const char* Exception::what() const noexcept { return _message.c_str(); }

std::string Exception::message() const { return _message; }

int Exception::error() const { return _error; }

Exception::~Exception() noexcept = default;
}  // namespace broker
}  // namespace upmq
