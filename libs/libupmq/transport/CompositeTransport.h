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

#ifndef _UPMQ_COMPOSITE_TRANSPORT_H_
#define _UPMQ_COMPOSITE_TRANSPORT_H_

#include <decaf/net/URI.h>
#include <decaf/util/List.h>
#include <transport/Config.h>
#include <transport/Transport.h>

namespace upmq {
namespace transport {

using decaf::net::URI;
using decaf::util::List;

/**
 * A Composite Transport is a Transport implementation that is composed of several
 * Transports.  The composition could be such that only one Transport exists for
 * each URI that is composed or there could be many active Transports working at
 * once.
 *
 * @since 3.0
 */
class UPMQCPP_API CompositeTransport : public upmq::transport::Transport {
 public:
  virtual ~CompositeTransport();

  /**
   * Add a URI to the list of URI's that will represent the set of Transports
   * that this Transport is a composite of.
   *
   * @param rebalance
   *      Indicates if the addition should cause a forced reconnect or not.
   * @param uris
   *      The new URI set to add to the set this composite maintains.
   */
  virtual void addURI(bool rebalance, const List<URI> &uris) = 0;

  /**
   * Remove a URI from the set of URI's that represents the set of Transports
   * that this Transport is composed of, removing a URI for which the composite
   * has created a connected Transport should result in that Transport being
   * disposed of.
   *
   * @param rebalance
   *      Indicates if the removal should cause a forced reconnect or not.
   * @param uris
   *        The new URI set to remove to the set this composite maintains.
   */
  virtual void removeURI(bool rebalance, const List<URI> &uris) = 0;
};
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_COMPOSITE_TRANSPORT_H_ */
