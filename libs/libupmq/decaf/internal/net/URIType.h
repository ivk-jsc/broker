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

#ifndef _DECAF_INTERNAL_NET_URITYPE_H_
#define _DECAF_INTERNAL_NET_URITYPE_H_

#include <decaf/util/Config.h>
#include <string>

namespace decaf {
namespace internal {
namespace net {

/**
 * Basic type object that holds data that composes a given URI
 */
class DECAF_API URIType {
 private:
  std::string source;
  std::string scheme;
  std::string schemeSpecificPart;
  std::string authority;
  std::string userinfo;
  std::string host;
  int port;
  std::string path;
  std::string query;
  std::string fragment;
  bool opaque;
  bool absolute;
  bool serverAuthority;
  bool valid;
  int hashCode;

 public:
  URIType();
  URIType(const std::string &source);

  virtual ~URIType();

  /**
   * Gets the source URI string that was parsed to obtain this URIType
   * instance and the resulting data,
   * @return the source URI string
   */
  std::string getSource() const { return this->source; }

  /**
   * Sets the source URI string that was parsed to obtain this URIType
   * instance and the resulting data,
   * @param newSource - the source URI string
   */
  void setSource(const std::string &newSource) { this->source = newSource; }

  /**
   * Gets the Scheme of the URI, e.g. scheme ("http"/"ftp"/...).
   * @return scheme part string.
   */
  std::string getScheme() const { return scheme; }

  /**
   * Sets the Scheme of the URI, e.g. scheme ("http"/"ftp"/...).
   * @param newScheme - scheme part string.
   */
  void setScheme(const std::string &newScheme) { this->scheme = newScheme; }

  /**
   * Gets the Scheme Specific Part of the URI.
   * @return scheme specific part string.
   */
  std::string getSchemeSpecificPart() const { return schemeSpecificPart; }

  /**
   * Sets the Scheme Specific Part of the URI.
   * @param newSchemeSpecificPart - scheme specific part string.
   */
  void setSchemeSpecificPart(const std::string &newSchemeSpecificPart) { this->schemeSpecificPart = newSchemeSpecificPart; }

  /**
   * Gets the Authority of the URI.
   * @return Authority part string.
   */
  std::string getAuthority() const { return authority; }

  /**
   * Sets the Authority of the URI.
   * @param newAuthority Authority part string.
   */
  void setAuthority(const std::string &newAuthority) { this->authority = newAuthority; }

  /**
   * Gets the user info part of the URI, e.g. user name, as in
   * http://user:passwd@host:port/
   * @return user info part string.
   */
  std::string getUserInfo() const { return userinfo; }

  /**
   * Sets the user info part of the URI, e.g. user name, as in
   * http://user:passwd@host:port/
   * @param newUserinfo - user info part string.
   */
  void setUserInfo(const std::string &newUserinfo) { this->userinfo = newUserinfo; }

  /**
   * Gets the Host name part of the URI.
   * @return Host name part string.
   */
  std::string getHost() const { return host; }

  /**
   * Sets the Host name part of the URI.
   * @param newHost - Host name part string.
   */
  void setHost(const std::string &newHost) { this->host = newHost; }

  /**
   * Gets the port part of the URI.
   * @return port part string, -1 if not set.
   */
  int getPort() const { return port; }

  /**
   * Sets the port part of the URI.
   * @param newPort - port part string, -1 if not set.
   */
  void setPort(int newPort) { this->port = newPort; }

  /**
   * Gets the Path part of the URI.
   * @return Path part string.
   */
  std::string getPath() const { return path; }

  /**
   * Sets the Path part of the URI.
   * @param newPath - Path part string.
   */
  void setPath(const std::string &newPath) { this->path = newPath; }

  /**
   * Gets the Query part of the URI.
   * @return Query part string.
   */
  std::string getQuery() const { return query; }

  /**
   * Sets the Query part of the URI.
   * @param newQuery - Query part string.
   */
  void setQuery(const std::string &newQuery) { this->query = newQuery; }

  /**
   * Gets the Fragment part of the URI.
   * @return Fragment part string.
   */
  std::string getFragment() const { return fragment; }

  /**
   * Sets the Fragment part of the URI.
   * @param newFragment - Fragment part string.
   */
  void setFragment(const std::string &newFragment) { this->fragment = newFragment; }

  /**
   * Gets if the URI is Opaque.
   * @return true if opaque.
   */
  bool isOpaque() const { return opaque; }

  /**
   * Sets if the URI is Opaque.
   * @param newOpaque true if opaque.
   */
  void setOpaque(bool newOpaque) { this->opaque = newOpaque; }

  /**
   * Gets if the URI is Absolute.
   * @return true if Absolute.
   */
  bool isAbsolute() const { return absolute; }

  /**
   * Sets if the URI is Absolute.
   * @param newAbsolute - true if Absolute.
   */
  void setAbsolute(bool newAbsolute) { this->absolute = newAbsolute; }

  /**
   * Gets if the URI is a Server Authority.
   * @return true if Server Authority.
   */
  bool isServerAuthority() const { return serverAuthority; }

  /**
   * Sets if the URI is a Server Authority.
   * @param newServerAuthority - true if Server Authority.
   */
  void setServerAuthority(bool newServerAuthority) { this->serverAuthority = newServerAuthority; }

  /**
   * Gets if the URI is valid, meaning that the source has been set and
   * parsed and all relevant data fields have been set.
   * @return true if the URIType contains valid data.
   */
  bool isValid() const { return valid; }

  /**
   * Sets if the URI is valid, meaning that the source has been set and
   * parsed and all relevant data fields have been set.
   * @param newValid - true if the URIType contains valid data.
   */
  void setValid(bool newValid) { this->valid = newValid; }

  /**
   * Gets the computed hashCode for this URIType or return -1 if non is set
   *
   * @return the hash code for this URIType instance or -1 if not set.
   */
  int getHashCode() const { return this->hashCode; }

  /**
   * Sets the hash code for this URIType instance.
   *
   * @param newHashCode
   *      The new hash code that's been computed for this URIType instance.
   */
  void setHashCode(int newHashCode) { this->hashCode = newHashCode; }
};
}  // namespace net
}  // namespace internal
}  // namespace decaf

#endif /* _DECAF_INTERNAL_NET_URITYPE_H_ */
