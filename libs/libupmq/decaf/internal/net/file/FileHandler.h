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

#ifndef _DECAF_INTERNAL_NET_FILE_FILEHANDLER_H_
#define _DECAF_INTERNAL_NET_FILE_FILEHANDLER_H_

#include <decaf/util/Config.h>

#include <decaf/net/URLStreamHandler.h>

namespace decaf {
namespace internal {
namespace net {
namespace file {

class DECAF_API FileHandler : public decaf::net::URLStreamHandler {
 public:
  virtual ~FileHandler();

 public:
  virtual decaf::net::URLConnection *openConnection(const decaf::net::URL &url);

  virtual decaf::net::URLConnection *openConnection(const decaf::net::URL &url, const decaf::net::Proxy *proxy);

  virtual int getDefaultPort() const;
};
}  // namespace file
}  // namespace net
}  // namespace internal
}  // namespace decaf

#endif /* _DECAF_INTERNAL_NET_FILE_FILEHANDLER_H_ */