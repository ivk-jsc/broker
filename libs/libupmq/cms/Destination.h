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

#ifndef _CMS_DESTINATION_H_
#define _CMS_DESTINATION_H_

#include <cms/CMSProperties.h>
#include <cms/Config.h>

#include <string>

namespace cms {

/**
 * A Destination object encapsulates a provider-specific address.
 * <p>
 * There is no standard definition of a Destination address, each provider can
 * provide its own definition and there can be configuration data attached to the
 * Destination address.
 * <p>
 * All CMS Destination objects support concurrent use.
 *
 * @since 1.0
 */
class CMS_API Destination {
 public:
  enum DestinationType { TOPIC, QUEUE, TEMPORARY_TOPIC, TEMPORARY_QUEUE };

 public:
  virtual ~Destination(){};

  /**
   * Retrieve the Destination Type for this Destination
   *
   * @return The Destination Type
   */
  virtual DestinationType getType() const = 0;

  /**
   * Retrieve the Destination Name for this Destination
   *
   * @return The Destination Name
   */
  virtual const std::string &getName() const = 0;

  /**
   * Retrieve the Destination Type Name for this Destination
   *
   * @param destType
   *      The Destination Type
   * @return The Destination Type Name
   */
  static std::string getDestinationTypeName(DestinationType destType);

  /**
   * Creates a new instance of this destination type that is a
   * copy of this one, and returns it.
   *
   * @returns cloned copy of this object
   */
  virtual cms::Destination *clone() const = 0;

  /**
   * Copies the contents of the given Destination object to this one.
   *
   * @param source
   *      The source Destination object.
   */
  virtual void copy(const cms::Destination &source) = 0;

  /**
   * Compares two Destination instances to determine if they represent the same
   * logic Destination.
   *
   * @param other
   *      The other destination to compare this one to.
   *
   * @return true if the two destinations are the same.
   */
  virtual bool equals(const cms::Destination &other) const = 0;
};
}  // namespace cms

#endif /*_CMS_DESTINATION_H_*/
