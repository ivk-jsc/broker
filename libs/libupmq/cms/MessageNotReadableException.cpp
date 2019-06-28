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

#include "MessageNotReadableException.h"

using namespace cms;

////////////////////////////////////////////////////////////////////////////////
MessageNotReadableException::MessageNotReadableException() = default;

////////////////////////////////////////////////////////////////////////////////
MessageNotReadableException::MessageNotReadableException(const MessageNotReadableException &ex) noexcept = default;

MessageNotReadableException::MessageNotReadableException(MessageNotReadableException &&ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
MessageNotReadableException::MessageNotReadableException(const std::string &message) : CMSException(message, nullptr) {}

////////////////////////////////////////////////////////////////////////////////
MessageNotReadableException::MessageNotReadableException(const std::string &message, const std::exception *cause) : CMSException(message, cause) {}

////////////////////////////////////////////////////////////////////////////////
MessageNotReadableException::MessageNotReadableException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int> > &stackTrace)
    : CMSException(message, cause, stackTrace) {}

////////////////////////////////////////////////////////////////////////////////
MessageNotReadableException::~MessageNotReadableException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
MessageNotReadableException *MessageNotReadableException::clone() { return new MessageNotReadableException(*this); }
