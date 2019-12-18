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

#ifndef _DECAF_NET_INET4ADDRESS_H_
#define _DECAF_NET_INET4ADDRESS_H_

#include <decaf/util/Config.h>

#include <decaf/net/InetAddress.h>

namespace decaf {
namespace net {

class DECAF_API Inet4Address : public InetAddress {
 private:
  friend class InetAddress;

 protected:
  Inet4Address();
  Inet4Address(const unsigned char *ipAddress, int numBytes);
  Inet4Address(const std::string &hostname_, const unsigned char *ipAddress, int numBytes);

 public:
  ~Inet4Address() override;

 public:
  InetAddress *clone() const override;

  /**
   * Check if this InetAddress is a valid wildcard address.
   *
   * @return true if the address is a wildcard address.
   */
  bool isAnyLocalAddress() const override;

  /**
   * Check if this InetAddress is a valid loopback address.
   *
   * @return true if the address is a loopback address.
   */
  bool isLoopbackAddress() const override;

  /**
   * Check if this InetAddress is a valid Multicast address.
   *
   * @return true if the address is a Multicast address.
   */
  bool isMulticastAddress() const override;

  /**
   * Check if this InetAddress is a valid link local address.
   *
   * @return true if the address is a link local address.
   */
  bool isLinkLocalAddress() const override;

  /**
   * Check if this InetAddress is a valid site local address.
   *
   * @return true if the address is a site local address.
   */
  bool isSiteLocalAddress() const override;

  /**
   * Check if this InetAddress is Multicast and has Global scope.
   *
   * @return true if the address is Multicast and has Global scope.
   */
  bool isMCGlobal() const override;

  /**
   * Check if this InetAddress is Multicast and has Node Local scope.
   *
   * @return true if the address is Multicast and has Node Local scope.
   */
  bool isMCNodeLocal() const override;

  /**
   * Check if this InetAddress is Multicast and has Link Local scope.
   *
   * @return true if the address is Multicast and has Link Local scope.
   */
  bool isMCLinkLocal() const override;

  /**
   * Check if this InetAddress is Multicast and has Site Local scope.
   *
   * @return true if the address is Multicast and has Site Local scope.
   */
  bool isMCSiteLocal() const override;

  /**
   * Check if this InetAddress is Multicast and has Organization scope.
   *
   * @return true if the address is Multicast and has Organization scope.
   */
  bool isMCOrgLocal() const override;
};
}  // namespace net
}  // namespace decaf

#endif /* _DECAF_NET_INET4ADDRESS_H_ */
