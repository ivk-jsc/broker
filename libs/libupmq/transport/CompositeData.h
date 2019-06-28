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

#ifndef _UPMQ_COMPOSITE_DATA_H_
#define _UPMQ_COMPOSITE_DATA_H_

#include <decaf/net/URI.h>
#include <decaf/util/LinkedList.h>
#include <decaf/util/Properties.h>
#include <transport/Config.h>

namespace upmq {
namespace transport {

using decaf::net::URI;
using decaf::util::LinkedList;
using decaf::util::Properties;

/**
 * Represents a Composite URI
 *
 * @since 3.0
 */
class UPMQCPP_API CompositeData {
 private:
  std::string host;
  std::string scheme;
  std::string path;
  LinkedList<URI> components;
  Properties parameters;
  std::string fragment;

 public:
  CompositeData();
  virtual ~CompositeData();

  LinkedList<URI> &getComponents() { return components; }
  const LinkedList<URI> &getComponents() const { return components; }

  void setComponents(const LinkedList<URI> &newComponents) { this->components = newComponents; }

  std::string getFragment() const { return fragment; }

  void setFragment(const std::string &newFragment) { this->fragment = newFragment; }

  const Properties &getParameters() const { return parameters; }

  void setParameters(const Properties &newParameters) { this->parameters = newParameters; }

  std::string getScheme() const { return scheme; }

  void setScheme(const std::string &newScheme) { this->scheme = newScheme; }

  std::string getPath() const { return path; }

  void setPath(const std::string &newPath) { this->path = newPath; }

  std::string getHost() const { return host; }

  void setHost(const std::string &newHost) { this->host = newHost; }

  /**
   * @throws decaf::net::URISyntaxException
   */
  URI toURI() const;
};
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_COMPOSITE_DATA_H_ */
