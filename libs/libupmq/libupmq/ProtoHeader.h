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

#ifndef __ProtoHeader_H__
#define __ProtoHeader_H__

#include <string>

#ifndef EMPTY_STRING
#define EMPTY_STRING ""
static const std::string emptyString = EMPTY_STRING;
#endif  // EMPTY_STRING

#define stringify(name) #name

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4127 4800 4100 4267 4244)
#endif
#include <protocol.pb.h>
#include <google/protobuf/util/json_util.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  //__ProtoHeader_H__
