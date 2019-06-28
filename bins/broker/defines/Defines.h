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

#ifndef __BROKER_DEFINES_H__
#define __BROKER_DEFINES_H__

#include "Errors.h"
#include "MessageDefines.h"
#include "StorageDefines.h"

#include <cstdint>
#include <iostream>
#include <string>

#if !defined(_WIN32) & !defined(_WIN64)
#include <cinttypes>
#endif

#define UNUSED_VAR(x) (void)x

#define MakeStringify(name) #name

#define DT_FORMAT "%Y-%m-%d %H:%M:%s"
#define DT_FORMAT_SIMPLE "%Y-%m-%d %H:%M:%S"
#ifdef _WIN32
#define atoll _atoi64
#endif

static const std::string emptyString;

#define TOPIC_PREFIX "topic"
#define TEMP_TOPIC_PREFIX "temp-topic"
#define QUEUE_PREFIX "queue"
#define TEMP_QUEUE_PREFIX "temp-queue"
#define ANY_PREFIX "*"

#define non_std_endl '\n'

#endif  // __BROKER_DEFINES_H__
