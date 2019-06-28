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

#ifndef _DECAF_IO_CLOSEABLE_H_
#define _DECAF_IO_CLOSEABLE_H_

#include <decaf/io/IOException.h>
#include <decaf/util/Config.h>

namespace decaf {
namespace io {

/**
 * Interface for a class that implements the close method.
 */
class DECAF_API Closeable {
 public:
  virtual ~Closeable();

  /**
   * Closes this object and deallocates the appropriate resources.
   * The object is generally no longer usable after calling close.
   *
   * @throws IOException if an error occurs while closing.
   */
  virtual void close() = 0;
};
}  // namespace io
}  // namespace decaf

#endif /*_DECAF_IO_CLOSEABLE_H_*/
