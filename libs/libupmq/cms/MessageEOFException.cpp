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

#include "MessageEOFException.h"

using namespace cms;

////////////////////////////////////////////////////////////////////////////////
MessageEOFException::MessageEOFException() = default;

////////////////////////////////////////////////////////////////////////////////
MessageEOFException::MessageEOFException(const MessageEOFException &ex) noexcept = default;

MessageEOFException::MessageEOFException(MessageEOFException &&ex) noexcept = default;

////////////////////////////////////////////////////////////////////////////////
MessageEOFException::MessageEOFException(const std::string &message) : CMSException(message, nullptr) {}

////////////////////////////////////////////////////////////////////////////////
MessageEOFException::MessageEOFException(const std::string &message, const std::exception *cause) : CMSException(message, cause) {}

////////////////////////////////////////////////////////////////////////////////
MessageEOFException::MessageEOFException(const std::string &message,
                                         const std::exception *cause,
                                         const std::vector<triplet<std::string, std::string, int> > &stackTrace)
    : CMSException(message, cause, stackTrace) {}

////////////////////////////////////////////////////////////////////////////////
MessageEOFException::~MessageEOFException() noexcept = default;

////////////////////////////////////////////////////////////////////////////////
MessageEOFException *MessageEOFException::clone() { return new MessageEOFException(*this); }
