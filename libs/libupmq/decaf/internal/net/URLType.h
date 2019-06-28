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

#ifndef _DECAF_INTERNAL_NET_URLTYPE_H_
#define _DECAF_INTERNAL_NET_URLTYPE_H_

#include <decaf/lang/String.h>
#include <decaf/util/Config.h>

namespace decaf {
namespace internal {
namespace net {

/**
 * Basic type object that holds data that composes a given URL
 */
class DECAF_API URLType {
 private:
  decaf::lang::String file;
  decaf::lang::String protocol;
  decaf::lang::String host;
  int port;
  decaf::lang::String authority;
  decaf::lang::String userInfo;
  decaf::lang::String path;
  decaf::lang::String query;
  decaf::lang::String ref;
  int hashCode;

 public:
  URLType();

  virtual ~URLType();

  /**
   * Gets the File of the URL.
   * @return File part string.
   */
  decaf::lang::String getFile() const { return file; }

  /**
   * Sets the File of the URL.
   * @param newFile Authority part string.
   */
  void setFile(const decaf::lang::String &newFile) { this->file = newFile; }

  /**
   * Gets the protocol of the URL, e.g. protocol ("http"/"ftp"/...).
   * @return protocol part string.
   */
  decaf::lang::String getProtocol() const { return protocol; }

  /**
   * Sets the protocol of the URL, e.g. protocol ("http"/"ftp"/...).
   * @param newProtocol - protocol part string.
   */
  void setProtocol(const decaf::lang::String &newProtocol) { this->protocol = newProtocol; }

  /**
   * Gets the Authority of the URL.
   * @return Authority part string.
   */
  decaf::lang::String getAuthority() const { return authority; }

  /**
   * Sets the Authority of the URL.
   * @param newAuthority Authority part string.
   */
  void setAuthority(const decaf::lang::String &newAuthority) { this->authority = newAuthority; }

  /**
   * Gets the user info part of the URL, e.g. user name, as in
   * http://user:passwd@host:port/
   * @return user info part string.
   */
  decaf::lang::String getUserInfo() const { return userInfo; }

  /**
   * Sets the user info part of the URL, e.g. user name, as in
   * http://user:passwd@host:port/
   *
   * @param newUserInfo - user info part string.
   */
  void setUserInfo(const decaf::lang::String &newUserInfo) { this->userInfo = newUserInfo; }

  /**
   * Gets the Host name part of the URL.
   * @return Host name part string.
   */
  decaf::lang::String getHost() const { return host; }

  /**
   * Sets the Host name part of the URL.
   * @param newHost - Host name part string.
   */
  void setHost(const decaf::lang::String &newHost) { this->host = newHost; }

  /**
   * Gets the port part of the URL.
   * @return port part string, -1 if not set.
   */
  int getPort() const { return port; }

  /**
   * Sets the port part of the URL.
   * @param newPort - port part string, -1 if not set.
   */
  void setPort(int newPort) { this->port = newPort; }

  /**
   * Gets the Path part of the URL.
   * @return Path part string.
   */
  decaf::lang::String getPath() const { return path; }

  /**
   * Sets the Path part of the URL.
   * @param newPath - Path part string.
   */
  void setPath(const decaf::lang::String &newPath) { this->path = newPath; }

  /**
   * Gets the Query part of the URL.
   * @return Query part string.
   */
  decaf::lang::String getQuery() const { return query; }

  /**
   * Sets the Query part of the URL.
   * @param newQuery - Query part string.
   */
  void setQuery(const decaf::lang::String &newQuery) { this->query = newQuery; }

  /**
   * Gets the Ref part of the URL.
   * @return Ref part string.
   */
  decaf::lang::String getRef() const { return ref; }

  /**
   * Sets the Ref part of the URL.
   * @param newRef - Ref part string.
   */
  void setRef(const decaf::lang::String &newRef) { this->ref = newRef; }

  /**
   * Gets the computed hashCode for this URLType or return -1 if non is set
   *
   * @return the hash code for this URLType instance or -1 if not set.
   */
  int getHashCode() const { return this->hashCode; }

  /**
   * Sets the hash code for this URLType instance.
   *
   * @param newHashCode
   *      The new hash code that's been computed for this URLType instance.
   */
  void setHashCode(int newHashCode) { this->hashCode = newHashCode; }
};
}  // namespace net
}  // namespace internal
}  // namespace decaf

#endif /* _DECAF_INTERNAL_NET_URLTYPE_H_ */
