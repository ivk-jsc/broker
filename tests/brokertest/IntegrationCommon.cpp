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

#include "IntegrationCommon.h"

////////////////////////////////////////////////////////////////////////////////
const int IntegrationCommon::defaultDelay = 1000;
const unsigned int IntegrationCommon::defaultMsgCount = 5;
// 200;
bool IntegrationCommon::debug = false;

////////////////////////////////////////////////////////////////////////////////
IntegrationCommon::IntegrationCommon()
    : urlCommon("tcp://127.0.0.1:"), stompURL(urlCommon + "12345?transport.trace=false"), openwireURL(urlCommon + "12345?transport.trace=false") {}

////////////////////////////////////////////////////////////////////////////////
IntegrationCommon &IntegrationCommon::getInstance() {
  static IntegrationCommon instance;

  return instance;
}
