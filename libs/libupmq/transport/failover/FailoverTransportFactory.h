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

#ifndef _UPMQ_FAILOVER_TRANSPORT_FACTORY_H_
#define _UPMQ_FAILOVER_TRANSPORT_FACTORY_H_

#include <decaf/net/URI.h>
#include <decaf/util/Properties.h>
#include <transport/AbstractTransportFactory.h>
#include <transport/Config.h>
#include <transport/Transport.h>
#include <transport/WireFormat.h>

namespace upmq {
namespace transport {
namespace failover {

using decaf::lang::Pointer;

/**
 * Creates an instance of a FailoverTransport.
 *
 * @since 3.0
 */
class UPMQCPP_API FailoverTransportFactory : public AbstractTransportFactory {
 public:
  virtual ~FailoverTransportFactory() {}

  virtual Pointer<Transport> create(const decaf::net::URI &location);

  virtual Pointer<Transport> createComposite(const decaf::net::URI &location);

 protected:
  virtual Pointer<Transport> doCreateComposite(const decaf::net::URI &location, const decaf::util::Properties &properties);
};
}  // namespace failover
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_FAILOVER_TRANSPORT_FACTORY_H_ */
