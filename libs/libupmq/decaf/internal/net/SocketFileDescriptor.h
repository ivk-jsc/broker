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

#ifndef _DECAF_INTERNAL_NET_SOCKETFILEDESCRIPTOR_H_
#define _DECAF_INTERNAL_NET_SOCKETFILEDESCRIPTOR_H_

#include <decaf/util/Config.h>

#include <decaf/io/FileDescriptor.h>

namespace decaf {
namespace internal {
namespace net {

/**
 * File Descriptor type used internally by Decaf Socket objects.
 *
 * @since 1.0
 */
class DECAF_API SocketFileDescriptor : public decaf::io::FileDescriptor {
 public:
  SocketFileDescriptor(long long value);

  ~SocketFileDescriptor() override;

  /**
   * Gets the OS Level FileDescriptor
   *
   * @return a FileDescriptor value.
   */
  long long getValue() const;
};
}  // namespace net
}  // namespace internal
}  // namespace decaf

#endif /* _DECAF_INTERNAL_NET_SOCKETFILEDESCRIPTOR_H_ */
