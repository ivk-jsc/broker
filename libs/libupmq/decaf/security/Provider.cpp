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

#include "Provider.h"

#include <decaf/lang/Pointer.h>
#include <decaf/util/StlSet.h>

#include <decaf/security/ProviderService.h>

using namespace decaf;
using namespace decaf::security;
using namespace decaf::util;
using namespace decaf::lang;

////////////////////////////////////////////////////////////////////////////////
namespace decaf {
namespace security {

class ProviderImpl {
 public:
  StlSet<ProviderService *> services;

  ~ProviderImpl() {
    try {
      Pointer<Iterator<ProviderService *> > iter(services.iterator());
      while (iter->hasNext()) {
        delete iter->next();
      }
      services.clear();
    }
    DECAF_CATCHALL_NOTHROW()
  }
};
}  // namespace security
}  // namespace decaf

////////////////////////////////////////////////////////////////////////////////
Provider::Provider(const std::string &name_, double version_, const std::string &info_)
    : name(name_), version(version_), info(info_), impl(new ProviderImpl) {}

////////////////////////////////////////////////////////////////////////////////
Provider::~Provider() {
  try {
    delete this->impl;
  }
  DECAF_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
const Set<ProviderService *> &Provider::getServices() const { return this->impl->services; }

////////////////////////////////////////////////////////////////////////////////
void Provider::addService(ProviderService *service) {
  if (service != nullptr) {
    this->impl->services.add(service);
  }
}
