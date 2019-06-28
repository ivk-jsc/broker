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

#ifndef _DECAF_INTERNAL_UTIL_RESOURCELIFECYCLEMANAGER_H_
#define _DECAF_INTERNAL_UTIL_RESOURCELIFECYCLEMANAGER_H_

#include <decaf/util/Config.h>

#include <decaf/internal/util/Resource.h>
#include <decaf/util/StlSet.h>

namespace decaf {
namespace internal {
namespace util {

/**
 *
 * @since 1.0
 */
class DECAF_API ResourceLifecycleManager {
 private:
  decaf::util::StlSet<Resource *> resources;

 public:
  ResourceLifecycleManager();

  virtual ~ResourceLifecycleManager();

  virtual void addResource(Resource *value);

 protected:
  virtual void destroyResources();
};
}  // namespace util
}  // namespace internal
}  // namespace decaf

#endif /* _DECAF_INTERNAL_UTIL_RESOURCELIFECYCLEMANAGER_H_ */
