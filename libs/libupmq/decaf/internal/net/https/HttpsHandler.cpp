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

#include <decaf/internal/net/https/HttpsHandler.h>

#include <decaf/lang/exceptions/IllegalArgumentException.h>

using namespace decaf;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::internal;
using namespace decaf::internal::net;
using namespace decaf::internal::net::https;

////////////////////////////////////////////////////////////////////////////////
HttpsHandler::~HttpsHandler() {}

////////////////////////////////////////////////////////////////////////////////
decaf::net::URLConnection *HttpsHandler::openConnection(const decaf::net::URL &url DECAF_UNUSED) {
  DECAF_UNUSED_VAR(url);
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
decaf::net::URLConnection *HttpsHandler::openConnection(const decaf::net::URL &url DECAF_UNUSED, const decaf::net::Proxy *proxy) {
  DECAF_UNUSED_VAR(url);
  if (proxy == nullptr) {
    throw IllegalArgumentException(__FILE__, __LINE__, "proxy object cannot be NULL");
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
int HttpsHandler::getDefaultPort() const { return 80; }
