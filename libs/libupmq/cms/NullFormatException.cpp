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

#include "NullFormatException.h"

using namespace cms;

////////////////////////////////////////////////////////////////////////////////
NullFormatException::NullFormatException() = default;

////////////////////////////////////////////////////////////////////////////////
NullFormatException::NullFormatException(const NullFormatException &ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
NullFormatException::NullFormatException(NullFormatException &&ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
NullFormatException::NullFormatException(const std::string &message) : CMSException(message, nullptr) {}

////////////////////////////////////////////////////////////////////////////////
NullFormatException::NullFormatException(const std::string &message, const std::exception *cause) : CMSException(message, cause) {}

////////////////////////////////////////////////////////////////////////////////
NullFormatException::NullFormatException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace)
    : CMSException(message, cause, stackTrace) {}

////////////////////////////////////////////////////////////////////////////////
NullFormatException::~NullFormatException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
NullFormatException *NullFormatException::clone() { return new NullFormatException(*this); }
