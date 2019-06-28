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

#ifndef _UPMQ_ABSTRACT_TRANSPORT_FACTORY_H_
#define _UPMQ_ABSTRACT_TRANSPORT_FACTORY_H_

#include <decaf/lang/Pointer.h>
#include <decaf/util/NoSuchElementException.h>
#include <decaf/util/Properties.h>
#include <transport/Config.h>
#include <transport/TransportFactory.h>
#include <transport/WireFormat.h>

namespace upmq {
namespace transport {

using decaf::lang::Pointer;

/**
 * Abstract implementation of the TransportFactory interface, providing
 * the base functionality that's common to most of the TransportFactory
 * instances.
 *
 * @since 3.0
 */
class UPMQCPP_API AbstractTransportFactory : public TransportFactory {
 public:
  virtual ~AbstractTransportFactory();

 protected:
  /**
   * Creates the WireFormat that is configured for this Transport and returns it.
   * The default WireFormat is Openwire.
   *
   * @param properties
   *        The properties that were configured on the URI.
   *
   * @return a pointer to a WireFormat instance that the caller then owns.
   *
   * @throws NoSuchElementException if the configured WireFormat is not found.
   */
  virtual Pointer<transport::WireFormat> createWireFormat(const decaf::util::Properties &properties);
};
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_ABSTRACT_TRANSPORT_FACTORY_H_ */
