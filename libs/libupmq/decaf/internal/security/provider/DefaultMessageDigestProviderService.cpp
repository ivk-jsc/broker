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

#include "DefaultMessageDigestProviderService.h"

#include <decaf/internal/security/provider/crypto/MD4MessageDigestSpi.h>
#include <decaf/internal/security/provider/crypto/MD5MessageDigestSpi.h>
#include <decaf/internal/security/provider/crypto/SHA1MessageDigestSpi.h>

using namespace decaf;
using namespace decaf::security;
using namespace decaf::internal;
using namespace decaf::internal::security;
using namespace decaf::internal::security::provider;
using namespace decaf::internal::security::provider::crypto;

////////////////////////////////////////////////////////////////////////////////
DefaultMessageDigestProviderService::DefaultMessageDigestProviderService(const Provider *provider, const std::string &algorithmName)
    : ProviderService(provider, "MessageDigest", algorithmName) {}

////////////////////////////////////////////////////////////////////////////////
DefaultMessageDigestProviderService::~DefaultMessageDigestProviderService() {}

////////////////////////////////////////////////////////////////////////////////
SecuritySpi *DefaultMessageDigestProviderService::newInstance() {
  if (getAlgorithm() == "MD4") {
    return new MD4MessageDigestSpi();
  }
  if (getAlgorithm() == "MD5") {
    return new MD5MessageDigestSpi();
  }
  return new SHA1MessageDigestSpi();
}
