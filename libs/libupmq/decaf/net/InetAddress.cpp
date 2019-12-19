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

#include "InetAddress.h"

#include <decaf/lang/Byte.h>
#include <decaf/lang/System.h>
#include <decaf/lang/exceptions/RuntimeException.h>
#include <decaf/net/Inet4Address.h>
#include <decaf/net/Inet6Address.h>
#include <decaf/net/UnknownHostException.h>
#include <Poco/Net/DNS.h>
using namespace decaf;
using namespace decaf::net;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::internal;

////////////////////////////////////////////////////////////////////////////////
const unsigned char InetAddress::loopbackBytes[4] = {127, 0, 0, 1};
const unsigned char InetAddress::anyBytes[4] = {0, 0, 0, 0};

////////////////////////////////////////////////////////////////////////////////
InetAddress::InetAddress() : hostname(), reached(false), addressBytes() {}

////////////////////////////////////////////////////////////////////////////////
InetAddress::InetAddress(const unsigned char *ipAddress, int numBytes) : hostname(), reached(false), addressBytes() {
  if (ipAddress == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "InetAddress constructor called with null address array.");
  }

  if (numBytes < 0) {
    throw IllegalArgumentException(__FILE__, __LINE__, "Number of bytes value is invalid: %d", numBytes);
  }

  unsigned char *copy = new unsigned char[numBytes];
  System::arraycopy(ipAddress, 0, copy, 0, numBytes);
  this->addressBytes.reset(copy, numBytes);
}

////////////////////////////////////////////////////////////////////////////////
InetAddress::InetAddress(const std::string &hostname_, const unsigned char *ipAddress, int numBytes)
    : hostname(hostname_), reached(false), addressBytes() {
  if (ipAddress == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "InetAddress constructor called with null address array.");
  }

  if (numBytes < 0) {
    throw IllegalArgumentException(__FILE__, __LINE__, "Number of bytes value is invalid: %d", numBytes);
  }

  unsigned char *copy = new unsigned char[numBytes];
  System::arraycopy(ipAddress, 0, copy, 0, numBytes);
  this->addressBytes.reset(copy, numBytes);
}

////////////////////////////////////////////////////////////////////////////////
InetAddress::~InetAddress() {}

////////////////////////////////////////////////////////////////////////////////
InetAddress *InetAddress::clone() const { return new InetAddress(*this); }

////////////////////////////////////////////////////////////////////////////////
ArrayPointer<unsigned char> InetAddress::getAddress() const { return this->addressBytes.clone(); }

////////////////////////////////////////////////////////////////////////////////
std::string InetAddress::getHostAddress() const {
  std::string address;

  for (int ix = 0; ix < this->addressBytes.length(); ix++) {
    address.append(Byte::toString(addressBytes[ix]));

    if (ix < this->addressBytes.length() - 1) {
      address.append(".");
    }
  }
  return address;
}

////////////////////////////////////////////////////////////////////////////////
std::string InetAddress::getHostName() const {
  if (!this->hostname.empty()) {
    return this->hostname;
  }

  return this->getHostAddress();
}

////////////////////////////////////////////////////////////////////////////////
std::string InetAddress::toString() const { return getHostName() + " / " + getHostAddress(); }

////////////////////////////////////////////////////////////////////////////////
InetAddress *InetAddress::getByAddress(const std::string &hostname, const unsigned char *bytes, int numBytes) {
  if (numBytes == 4) {
    return new Inet4Address(hostname, bytes, numBytes);
  } else if (numBytes == 16) {
    return new Inet6Address(hostname, bytes, numBytes);
  } else {
    throw UnknownHostException(__FILE__, __LINE__, "Number of Bytes passed was invalid: %d", numBytes);
  }
}

////////////////////////////////////////////////////////////////////////////////
InetAddress *InetAddress::getByAddress(const unsigned char *bytes, int numBytes) {
  if (numBytes == 4) {
    return new Inet4Address(bytes, numBytes);
  } else if (numBytes == 16) {
    return new Inet6Address(bytes, numBytes);
  } else {
    throw UnknownHostException(__FILE__, __LINE__, "Number of Bytes passed was invalid: %d", numBytes);
  }
}

////////////////////////////////////////////////////////////////////////////////
InetAddress *InetAddress::getLocalHost() {
  std::string host;
  using Poco::Net::DNS;
  using Poco::Net::HostEntry;
  try {
    const HostEntry &entry = DNS::thisHost();
    host = entry.name();

    const HostEntry::AddressList &addrs = entry.addresses();
    auto addr_it = addrs.begin();
    // for (; addr_it != addrs.end(); ++addr_it) std::cout << "Address: " << addr_it->toString() << std::endl;
    std::string ip = addr_it->toString();
    if (addr_it->family() == Poco::Net::AddressFamily::IPv4) {
      return new Inet4Address(host, (const unsigned char *)ip.c_str(), static_cast<int>(ip.size()));
    }
    return new Inet6Address(host, (const unsigned char *)ip.c_str(), static_cast<int>(ip.size()));
  }
  DECAF_CATCH_RETHROW(UnknownHostException)
  DECAF_CATCH_EXCEPTION_CONVERT(Exception, UnknownHostException)
  DECAF_CATCHALL_THROW(UnknownHostException)
}

////////////////////////////////////////////////////////////////////////////////
unsigned int InetAddress::bytesToInt(const unsigned char *bytes, int start) {
  // First mask the byte with 255, as when a negative
  // signed byte converts to an integer, it has bits
  // on in the first 3 bytes, we are only concerned
  // about the right-most 8 bits.
  // Then shift the rightmost byte to align with its
  // position in the integer.
  int value = ((bytes[start + 3] & 255)) | ((bytes[start + 2] & 255) << 8) | ((bytes[start + 1] & 255) << 16) | ((bytes[start] & 255) << 24);

  return value;
}

////////////////////////////////////////////////////////////////////////////////
InetAddress InetAddress::getAnyAddress() { return Inet4Address("localhost", InetAddress::loopbackBytes, 4); }

////////////////////////////////////////////////////////////////////////////////
InetAddress InetAddress::getLoopbackAddress() { return Inet4Address(InetAddress::anyBytes, 4); }
