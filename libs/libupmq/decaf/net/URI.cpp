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

#include "URI.h"

#include <decaf/internal/net/URIEncoderDecoder.h>
#include <decaf/internal/net/URIHelper.h>
#include <decaf/internal/util/StringUtils.h>
#include <decaf/lang/Character.h>
#include <decaf/lang/Integer.h>
#include <decaf/net/URL.h>

using namespace std;
using namespace decaf;
using namespace decaf::net;
using namespace decaf::internal;
using namespace decaf::internal::net;
using namespace decaf::internal::util;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
const std::string URI::unreserved = "_-!.~\'()*";
const std::string URI::punct = ",;:$&+=";
const std::string URI::reserved = punct + "?/[]@";
const std::string URI::someLegal = unreserved + punct;
const std::string URI::allLegal = unreserved + reserved;

////////////////////////////////////////////////////////////////////////////////
URI::URI() {}

////////////////////////////////////////////////////////////////////////////////
URI::URI(const URI &uri) : uri(uri.uri), uriString(uri.uriString) {}

////////////////////////////////////////////////////////////////////////////////
URI::URI(const std::string &uri) : uriString(uri) { this->parseURI(uri, false); }

////////////////////////////////////////////////////////////////////////////////
URI::URI(const std::string &scheme, const std::string &ssp, const std::string &fragment) {
  std::string aUri;

  if (!scheme.empty()) {
    aUri.append(scheme);
    aUri.append(":");
  }

  if (!ssp.empty()) {
    // QUOTE ILLEGAL CHARACTERS
    aUri.append(quoteComponent(ssp, allLegal));
  }

  if (!fragment.empty()) {
    aUri.append("#");
    // QUOTE ILLEGAL CHARACTERS
    aUri.append(quoteComponent(fragment, allLegal));
  }

  // Now hand of to the main parse function.
  this->parseURI(aUri, false);
}

////////////////////////////////////////////////////////////////////////////////
URI::URI(const std::string &scheme, const std::string &userInfo, const std::string &host, int port, const std::string &path, const std::string &query, const std::string &fragment) {
  if (scheme.empty() && userInfo.empty() && host.empty() && path.empty() && query.empty() && fragment.empty()) {
    return;
  }

  if (!scheme.empty() && !path.empty() && path.at(0) != '/') {
    throw URISyntaxException(__FILE__, __LINE__, path, "URI::URI - Path string: %s starts with invalid char '/'");
  }

  std::string aUri;
  if (!scheme.empty()) {
    aUri.append(scheme);
    aUri.append(":");
  }

  aUri.append("//");

  if (!userInfo.empty()) {
    // QUOTE ILLEGAL CHARACTERS in userinfo
    aUri.append(quoteComponent(userInfo, someLegal));
    aUri.append("@");
  }

  if (!host.empty()) {
    std::string newHost = host;

    // check for ipv6 addresses that hasn't been enclosed
    // in square brackets
    if (host.find(':') != std::string::npos && host.find(']') == std::string::npos && host.find('[') == std::string::npos) {
      newHost = std::string("[") + host + "]";
    }

    aUri.append(newHost);
  }

  if (port != -1) {
    aUri.append(":");
    aUri.append(Integer::toString(port));
  }

  if (!path.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append(quoteComponent(path, "/@" + someLegal));
  }

  if (!query.empty()) {
    aUri.append("?");
    // QUOTE ILLEGAL CHARS
    aUri.append(quoteComponent(query, allLegal));
  }

  if (!fragment.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append("#");
    aUri.append(quoteComponent(fragment, allLegal));
  }

  this->parseURI(aUri, true);
}

////////////////////////////////////////////////////////////////////////////////
URI::URI(const std::string &scheme, const std::string &host, const std::string &path, const std::string &fragment) : uri(), uriString() {
  if (scheme.empty() && host.empty() && path.empty() && fragment.empty()) {
    return;
  }

  if (!scheme.empty() && !path.empty() && path.at(0) != '/') {
    throw URISyntaxException(__FILE__, __LINE__, path, "URI::URI - Path string: %s starts with invalid char '/'");
  }

  std::string aUri;
  if (!scheme.empty()) {
    aUri.append(scheme);
    aUri.append(":");
  }

  if (!host.empty()) {
    aUri.append("//");

    std::string newHost = host;

    // check for ipv6 addresses that hasn't been enclosed
    // in square brackets
    if (host.find(':') != std::string::npos && host.find(']') == std::string::npos && host.find('[') == std::string::npos) {
      newHost = std::string("[") + host + "]";
    }

    aUri.append(newHost);
  }

  if (!path.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append(quoteComponent(path, "/@" + someLegal));
  }

  if (!fragment.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append("#");
    aUri.append(quoteComponent(fragment, allLegal));
  }

  this->parseURI(aUri, true);
}

////////////////////////////////////////////////////////////////////////////////
URI::URI(const std::string &scheme, const std::string &authority, const std::string &path, const std::string &query, const std::string &fragment) : uri(), uriString() {
  if (!scheme.empty() && !path.empty() && path.at(0) != '/') {
    throw URISyntaxException(__FILE__, __LINE__, path, "URI::URI - Path String %s must start with a '/'");
  }

  std::string aUri;

  if (!scheme.empty()) {
    aUri.append(scheme);
    aUri.append(":");
  }

  aUri.append("//");

  if (!authority.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append(quoteComponent(authority, "@[]" + someLegal));
  }

  if (!path.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append(quoteComponent(path, "/@" + someLegal));
  }

  if (!query.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append("?");
    aUri.append(quoteComponent(query, allLegal));
  }

  if (!fragment.empty()) {
    // QUOTE ILLEGAL CHARS
    aUri.append("#");
    aUri.append(quoteComponent(fragment, allLegal));
  }

  this->parseURI(aUri, false);
}

////////////////////////////////////////////////////////////////////////////////
void URI::parseURI(const std::string &aUri, bool forceServer) {
  try {
    this->uri = URIHelper().parseURI(aUri, forceServer);
  }
  DECAF_CATCH_RETHROW(URISyntaxException)
  DECAF_CATCHALL_THROW(URISyntaxException)
}

////////////////////////////////////////////////////////////////////////////////
int URI::compareTo(const URI &value) const {
  int ret = 0;

  const std::string thisScheme = this->uri.getScheme();
  const std::string valueScheme = value.getScheme();
  // compare schemes
  if (thisScheme.empty() && !valueScheme.empty()) {
    return -1;
  } else if (!thisScheme.empty() && valueScheme.empty()) {
    return 1;
  } else if (!thisScheme.empty() && !valueScheme.empty()) {
    ret = StringUtils::compareIgnoreCase(thisScheme.c_str(), valueScheme.c_str());
    if (ret != 0) {
      return ret > 0 ? 1 : -1;
    }
  }

  // compare opacities
  if (!this->uri.isOpaque() && value.isOpaque()) {
    return -1;
  } else if (this->uri.isOpaque() && !value.isOpaque()) {
    return 1;
  } else if (this->uri.isOpaque() && value.isOpaque()) {
    ret = StringUtils::compare(this->getSchemeSpecificPart().c_str(), value.getSchemeSpecificPart().c_str());
    if (ret != 0) {
      return ret > 0 ? 1 : -1;
    }
  } else {
    // otherwise both must be hierarchical

    const std::string thisAuthority = this->uri.getAuthority();
    const std::string valueAuthority = value.getAuthority();
    // compare authorities
    if (!thisAuthority.empty() && valueAuthority.empty()) {
      return 1;
    } else if (thisAuthority.empty() && !valueAuthority.empty()) {
      return -1;
    } else if (!thisAuthority.empty() && !valueAuthority.empty()) {
      if (!this->uri.getHost().empty() && !value.getHost().empty()) {
        const std::string thisUserInfo = this->getUserInfo();
        const std::string valueUserInfo = value.getUserInfo();
        // both are server based, so compare userinfo, host, port
        if (!thisUserInfo.empty() && valueUserInfo.empty()) {
          return 1;
        } else if (thisUserInfo.empty() && !valueUserInfo.empty()) {
          return -1;
        } else if (!thisUserInfo.empty() && !valueUserInfo.empty()) {
          ret = StringUtils::compare(thisUserInfo.c_str(), valueUserInfo.c_str());
          if (ret != 0) {
            return ret > 0 ? 1 : -1;
          }
        }

        // userinfo's are the same, compare hostname
        ret = StringUtils::compareIgnoreCase(this->uri.getHost().c_str(), value.getHost().c_str());
        if (ret != 0) {
          return ret > 0 ? 1 : -1;
        }

        // compare port
        if (this->getPort() != uri.getPort()) {
          return (getPort() - uri.getPort()) > 0 ? 1 : -1;
        }
      } else {
        // one or both are registry based, compare the whole authority
        ret = StringUtils::compare(thisAuthority.c_str(), valueAuthority.c_str());
        if (ret != 0) {
          return ret > 0 ? 1 : -1;
        }
      }
    }

    // authorities are the same, compare paths
    ret = StringUtils::compare(this->getPath().c_str(), value.getPath().c_str());
    if (ret != 0) {
      return ret > 0 ? 1 : -1;
    }

    const std::string thisQuery = this->getQuery();
    const std::string valueQuery = value.getQuery();
    // compare queries
    if (!thisQuery.empty() && valueQuery.empty()) {
      return 1;
    } else if (thisQuery.empty() && !valueQuery.empty()) {
      return -1;
    } else if (!thisQuery.empty() && !valueQuery.empty()) {
      ret = StringUtils::compare(thisQuery.c_str(), uri.getQuery().c_str());
      if (ret != 0) {
        return ret > 0 ? 1 : -1;
      }
    }
  }

  const std::string thisFragment = this->getFragment();
  const std::string valueFragment = value.getFragment();
  // everything else is identical, so compare fragments
  if (!thisFragment.empty() && valueFragment.empty()) {
    return 1;
  }
  if (thisFragment.empty() && !valueFragment.empty()) {
    return -1;
  }
  if (!thisFragment.empty() && !valueFragment.empty()) {
    ret = StringUtils::compare(thisFragment.c_str(), valueFragment.c_str());
    if (ret != 0) {
      return ret > 0 ? 1 : -1;
    }
  }

  // identical
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool URI::equals(const URI &value) const {
  if (value.uri.getFragment().empty() != this->uri.getFragment().empty()) {
    return false;
  }
  if (!value.uri.getFragment().empty() && !this->uri.getFragment().empty()) {
    if (!equalsHexCaseInsensitive(value.uri.getFragment(), this->uri.getFragment())) {
      return false;
    }
  }

  if (value.uri.getScheme().empty() != this->uri.getScheme().empty()) {
    return false;
  }
  if (!value.uri.getScheme().empty() && !this->uri.getScheme().empty()) {
    if (StringUtils::compareIgnoreCase(value.uri.getScheme().c_str(), this->uri.getScheme().c_str()) != 0) {
      return false;
    }
  }

  if (value.uri.isOpaque() && this->uri.isOpaque()) {
    return equalsHexCaseInsensitive(value.uri.getSchemeSpecificPart(), this->uri.getSchemeSpecificPart());
  }
  if (!value.uri.isOpaque() && !this->uri.isOpaque()) {
    if (!equalsHexCaseInsensitive(this->uri.getPath(), value.uri.getPath())) {
      return false;
    }

    if (value.uri.getQuery().empty() != this->uri.getQuery().empty()) {
      return false;
    }
    if (!value.uri.getQuery().empty() && !this->uri.getQuery().empty()) {
      if (!equalsHexCaseInsensitive(value.uri.getQuery(), this->uri.getQuery())) {
        return false;
      }
    }

    if (value.uri.getAuthority().empty() != this->uri.getAuthority().empty()) {
      return false;
    }
    if (!value.uri.getAuthority().empty() && !this->uri.getAuthority().empty()) {
      if (value.uri.getHost().empty() != this->uri.getHost().empty()) {
        return false;
      }
      if (value.uri.getHost().empty() && this->uri.getHost().empty()) {
        // both are registry based, so compare the whole authority
        return equalsHexCaseInsensitive(value.uri.getAuthority(), this->uri.getAuthority());
      }
      // uri.host != "" && host != "", so server-based

      if (StringUtils::compareIgnoreCase(value.uri.getHost().c_str(), this->uri.getHost().c_str()) != 0) {
        return false;
      }

      if (this->uri.getPort() != value.uri.getPort()) {
        return false;
      }

      if (value.uri.getUserInfo().empty() != this->uri.getUserInfo().empty()) {
        return false;
      }
      if (!value.uri.getUserInfo().empty() && !this->uri.getUserInfo().empty()) {
        return equalsHexCaseInsensitive(this->uri.getUserInfo(), value.uri.getUserInfo());
      }
      return true;
    }
    // no authority
    return true;
  }
  // one is opaque, the other hierarchical
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool URI::operator==(const URI &value) const { return compareTo(value) == 0; }

////////////////////////////////////////////////////////////////////////////////
bool URI::operator<(const URI &value) const { return compareTo(value) == -1; }

////////////////////////////////////////////////////////////////////////////////
URI URI::create(const string &uri) {
  try {
    return URI(uri);
  } catch (URISyntaxException &e) {
    throw IllegalArgumentException(e);
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string URI::quoteComponent(const std::string &component, const std::string &legalset) {
  try {
    /*
     * Use a different encoder than URLEncoder since: 1. chars like "/",
     * "#", "@" etc needs to be preserved instead of being encoded, 2.
     * UTF-8 char set needs to be used for encoding instead of default
     * platform one
     */
    return URIEncoderDecoder::quoteIllegal(component, legalset);
  }
  DECAF_CATCH_RETHROW(decaf::lang::Exception)
  DECAF_CATCHALL_THROW(decaf::lang::Exception)
}

////////////////////////////////////////////////////////////////////////////////
std::string URI::encodeOthers(const std::string &src) const {
  try {
    /*
     * Use a different encoder than URLEncoder since: 1. chars like "/",
     * "#", "@" etc needs to be preserved instead of being encoded, 2.
     * UTF-8 char set needs to be used for encoding instead of default
     * platform one 3. Only other chars need to be converted
     */
    return URIEncoderDecoder::encodeOthers(src);
  }
  DECAF_CATCH_RETHROW(decaf::lang::Exception)
  DECAF_CATCHALL_THROW(decaf::lang::Exception)
}

////////////////////////////////////////////////////////////////////////////////
std::string URI::decode(const std::string &src) const {
  if (src.empty()) {
    return src;
  }

  try {
    return URIEncoderDecoder::decode(src);
  }
  DECAF_CATCH_RETHROW(decaf::lang::Exception)
  DECAF_CATCHALL_THROW(decaf::lang::Exception)
}

////////////////////////////////////////////////////////////////////////////////
bool URI::equalsHexCaseInsensitive(const std::string &first, const std::string &second) const {
  if (first.find('%') != second.find('%')) {
    return StringUtils::compare(first.c_str(), second.c_str()) == 0;
  }

  std::size_t index = 0;
  std::size_t previndex = 0;

  while ((index = first.find('%', previndex)) != string::npos && second.find('%', previndex) == index) {
    bool match = first.substr(previndex, index - previndex) == second.substr(previndex, index - previndex);

    if (!match) {
      return false;
    }

    match = StringUtils::compareIgnoreCase(first.substr(index + 1, 3).c_str(), second.substr(index + 1, 3).c_str()) == 0;

    if (!match) {
      return false;
    }

    index += 3;
    previndex = index;
  }

  return first.substr(previndex) == second.substr(previndex);
}

////////////////////////////////////////////////////////////////////////////////
std::string URI::convertHexToLowerCase(const std::string &s) const {
  string result;

  if (s.find('%') == string::npos) {
    return s;
  }

  std::size_t index = 0;
  std::size_t previndex = 0;

  while ((index = s.find('%', previndex)) != string::npos) {
    result.append(s.substr(previndex, (index - previndex) + 1));

    string temp = s.substr(index + 1, 3);

    for (const char &i : temp) {
      result.append(1, Character::toLowerCase(i));
    }

    index += 3;
    previndex = index;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
std::string URI::normalize(const std::string &path) const {
  if (path.empty()) {
    return path;
  }

  // count the number of '/'s, to determine number of segments
  std::size_t index = static_cast<size_t>(-1);
  std::size_t pathlen = path.length();
  unsigned int size = 0;

  if (path.at(0) != '/') {
    size++;
  }

  while ((index = path.find('/', index + 1)) != string::npos) {
    if (index + 1 < pathlen && path.at(index + 1) != '/') {
      size++;
    }
  }

  std::vector<string> seglist(size);
  std::vector<bool> include(size);

  // break the path into segments and store in the list
  std::size_t current = 0;
  std::size_t index2 = 0;

  index = (path.at(0) == '/') ? 1 : 0;
  while ((index2 = path.find('/', index + 1)) != string::npos) {
    seglist[current++] = path.substr(index, index2 - index);
    index = index2 + 1;
  }

  // if current==size, then the last character was a slash
  // and there are no more segments
  if (current < size) {
    seglist[current] = path.substr(index);
  }

  // determine which segments get included in the normalized path
  for (unsigned int i = 0; i < size; i++) {
    include[i] = true;

    if (seglist[i] == "..") {
      int remove = i - 1;

      // search back to find a segment to remove, if possible
      while (remove > -1 && !include[remove]) {
        remove--;
      }

      // if we find a segment to remove, remove it and the ".."
      // segment
      if (remove > -1 && !(seglist[remove] == "..")) {
        include[remove] = false;
        include[i] = false;
      }

    } else if (seglist[i] == ".") {
      include[i] = false;
    }
  }

  // put the path back together
  string newpath;
  if (path.at(0) == '/') {
    newpath.append("/");
  }

  for (unsigned int i = 0; i < seglist.size(); i++) {
    if (include[i]) {
      newpath.append(seglist[i]);
      newpath.append("/");
    }
  }

  // if we used at least one segment and the path previously ended with
  // a slash and the last segment is still used, then delete the extra
  // trailing '/'
  if (path.at(path.length() - 1) != '/' && !seglist.empty() && include[seglist.size() - 1]) {
    newpath.erase(newpath.length() - 1, 1);
  }

  string result = newpath;

  // check for a ':' in the first segment if one exists,
  // prepend "./" to normalize
  index = result.find(':');
  index2 = result.find('/');

  if (index != string::npos && (index2 == string::npos || index < index2)) {
    newpath.insert(0, "./");
    result = std::move(newpath);
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
URI URI::normalize() const {
  if (isOpaque()) {
    return *this;
  }

  string normalizedPath = normalize(this->uri.getPath());

  // if the path is already normalized, return this
  if (this->uri.getPath() == normalizedPath) {
    return *this;
  }

  // get an exact copy of the URI re-calculate the scheme specific part
  // since the path of the normalized URI is different from this URI.
  URI result = *this;
  result.uri.setPath(normalizedPath);
  result.setSchemeSpecificPart();

  return result;
}

////////////////////////////////////////////////////////////////////////////////
void URI::setSchemeSpecificPart() {
  // ssp = [//authority][path][?query]
  string ssp;

  if (!this->uri.getAuthority().empty()) {
    ssp.append(string("//") + this->uri.getAuthority());
  }

  if (!this->uri.getPath().empty()) {
    ssp.append(this->uri.getPath());
  }

  if (!this->uri.getQuery().empty()) {
    ssp.append("?" + this->uri.getQuery());
  }

  this->uri.setSchemeSpecificPart(ssp);

  // reset string, so that it can be re-calculated correctly when asked.
  this->uriString = "";
}

////////////////////////////////////////////////////////////////////////////////
URI URI::parseServerAuthority() const {
  URI newURI = *this;

  if (!newURI.uri.isServerAuthority()) {
    newURI.uri = URIHelper().parseAuthority(true, this->uri.getAuthority());
  }

  return newURI;
}

////////////////////////////////////////////////////////////////////////////////
URI URI::relativize(const URI &relative) const {
  if (relative.isOpaque() || this->isOpaque()) {
    return relative;
  }

  if (this->uri.getScheme().empty() ? !relative.uri.getScheme().empty() : this->uri.getScheme() != relative.uri.getScheme()) {
    return relative;
  }

  if (this->uri.getAuthority().empty() ? !relative.uri.getAuthority().empty() : this->uri.getAuthority() != relative.uri.getAuthority()) {
    return relative;
  }

  // normalize both paths
  string thisPath = normalize(this->uri.getPath());
  string relativePath = normalize(relative.uri.getPath());

  /*
   * if the paths aren't equal, then we need to determine if this URI's
   * path is a parent path (begins with) the relative URI's path
   */
  if (thisPath != relativePath) {
    // if this URI's path doesn't end in a '/', add one
    if (thisPath.empty() || thisPath.at(thisPath.length() - 1) != '/') {
      thisPath = thisPath + '/';
    }

    /*
     * if the relative URI's path doesn't start with this URI's path,
     * then just return the relative URI; the URIs have nothing in
     * common
     */
    if (relativePath.find(thisPath) != 0) {
      return relative;
    }
  }

  URI result;
  result.uri.setFragment(relative.uri.getFragment());
  result.uri.setQuery(relative.uri.getQuery());
  // the result URI is the remainder of the relative URI's path
  result.uri.setPath(relativePath.substr(thisPath.length()));
  result.setSchemeSpecificPart();

  return result;
}

////////////////////////////////////////////////////////////////////////////////
URI URI::resolve(const URI &relative) const {
  if (relative.isAbsolute() || this->isOpaque()) {
    return relative;
  }

  URI result;
  if (relative.uri.getPath().empty() && relative.uri.getScheme().empty() && relative.uri.getAuthority().empty() && relative.uri.getQuery().empty() && !relative.uri.getFragment().empty()) {
    // if the relative URI only consists of fragment,
    // the resolved URI is very similar to this URI,
    // except that it has the fragment from the relative URI.
    result = *this;
    result.uri.setFragment(relative.uri.getFragment());
    result.uriString = "";

    // no need to re-calculate the scheme specific part,
    // since fragment is not part of scheme specific part.
    return result;
  }

  if (!relative.uri.getAuthority().empty()) {
    // if the relative URI has authority,
    // the resolved URI is almost the same as the relative URI,
    // except that it has the scheme of this URI.
    result = relative;
    result.uri.setScheme(this->uri.getScheme());
    result.uri.setAbsolute(this->uri.isAbsolute());
    result.uriString = "";

  } else {
    // since relative URI has no authority,
    // the resolved URI is very similar to this URI,
    // except that it has the query and fragment of the relative URI,
    // and the path is different.
    result = *this;
    result.uri.setFragment(relative.uri.getFragment());
    result.uri.setQuery(relative.uri.getQuery());
    result.uriString = "";

    if (relative.uri.getPath().at(0) == '/') {
      result.uri.setPath(relative.uri.getPath());
    } else {
      // resolve a relative reference
      std::size_t endindex = this->uri.getPath().find_last_of('/') + 1;
      result.uri.setPath(normalize(this->uri.getPath().substr(0, endindex) + relative.uri.getPath()));
    }

    // re-calculate the scheme specific part since
    // query and path of the resolved URI is different from this URI.
    result.setSchemeSpecificPart();
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
URI URI::resolve(const std::string &relative) const { return resolve(create(relative)); }

////////////////////////////////////////////////////////////////////////////////
string URI::toString() const {
  if (this->uriString.empty()) {
    string result;

    if (!this->uri.getScheme().empty()) {
      result.append(this->uri.getScheme());
      result.append(":");
    }

    if (this->isOpaque()) {
      result.append(this->uri.getSchemeSpecificPart());
    } else {
      if (!this->uri.getAuthority().empty()) {
        result.append("//");
        result.append(this->uri.getAuthority());
      }

      if (!this->uri.getPath().empty()) {
        result.append(this->uri.getPath());
      }

      if (!this->uri.getQuery().empty()) {
        result.append("?");
        result.append(this->uri.getQuery());
      }
    }

    if (!this->uri.getFragment().empty()) {
      result.append("#");
      result.append(this->uri.getFragment());
    }

    this->uriString = std::move(result);
  }

  return this->uriString;
}

////////////////////////////////////////////////////////////////////////////////
URL URI::toURL() const {
  if (!this->isAbsolute()) {
    throw IllegalArgumentException(__FILE__, __LINE__, "URI is not absolute, cannot convert to an URL.");
  }

  return URL(this->toString());
}

////////////////////////////////////////////////////////////////////////////////
std::string URI::getAuthority() const { return this->decode(this->uri.getAuthority()); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getFragment() const { return this->decode(this->uri.getFragment()); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getHost() const { return this->uri.getHost(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getPath() const { return this->decode(this->uri.getPath()); }

////////////////////////////////////////////////////////////////////////////////
int URI::getPort() const { return this->uri.getPort(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getQuery() const { return this->decode(this->uri.getQuery()); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getRawAuthority() const { return this->uri.getAuthority(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getRawFragment() const { return this->uri.getFragment(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getRawPath() const { return this->uri.getPath(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getRawQuery() const { return this->uri.getQuery(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getRawSchemeSpecificPart() const { return this->uri.getSchemeSpecificPart(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getRawUserInfo() const { return this->uri.getUserInfo(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getScheme() const { return this->uri.getScheme(); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getSchemeSpecificPart() const { return this->decode(this->uri.getSchemeSpecificPart()); }

////////////////////////////////////////////////////////////////////////////////
std::string URI::getUserInfo() const { return this->decode(this->uri.getUserInfo()); }

////////////////////////////////////////////////////////////////////////////////
bool URI::isAbsolute() const { return this->uri.isAbsolute(); }

////////////////////////////////////////////////////////////////////////////////
bool URI::isOpaque() const { return this->uri.isOpaque(); }
