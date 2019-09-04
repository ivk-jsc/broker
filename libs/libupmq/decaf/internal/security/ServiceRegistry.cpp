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

#include "ServiceRegistry.h"

#include <decaf/lang/Pointer.h>
#include <decaf/security/Provider.h>
#include <decaf/security/ProviderService.h>
#include <decaf/util/ArrayList.h>
#include <decaf/util/HashMap.h>

using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::security;
using namespace decaf::internal;
using namespace decaf::internal::security;

////////////////////////////////////////////////////////////////////////////////
namespace decaf {
namespace internal {
namespace security {

class ServiceRegistryImpl {
 private:
  ServiceRegistryImpl(const ServiceRegistryImpl &);
  ServiceRegistryImpl &operator=(const ServiceRegistryImpl &);

 public:
  ArrayList<const Provider *> providers;
  HashMap<std::string, ProviderService *> services;

 public:
  ServiceRegistryImpl() : providers(), services() {}
};
}  // namespace security
}  // namespace internal
}  // namespace decaf

////////////////////////////////////////////////////////////////////////////////
ServiceRegistry::ServiceRegistry() : impl(new ServiceRegistryImpl) {}

////////////////////////////////////////////////////////////////////////////////
ServiceRegistry::~ServiceRegistry() { delete this->impl; }

////////////////////////////////////////////////////////////////////////////////
void ServiceRegistry::addProvider(const Provider *provider) {
  if (provider == nullptr) {
    return;
  }

  this->impl->providers.add(provider);

  Pointer<Iterator<ProviderService *> > iter(provider->getServices().iterator());
  while (iter->hasNext()) {
    ProviderService *service = iter->next();

    std::string type = service->getType();
    std::string algorithm = service->getAlgorithm();
    std::string name = type.append(".").append(algorithm);

    if (!this->impl->services.containsKey(name)) {
      this->impl->services.put(name, service);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
ProviderService *ServiceRegistry::getService(const std::string &name) {
  if (this->impl->services.containsKey(name)) {
    return this->impl->services.get(name);
  }

  return nullptr;
}
