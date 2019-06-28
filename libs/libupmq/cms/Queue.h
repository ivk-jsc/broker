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

#ifndef _CMS_QUEUE_H_
#define _CMS_QUEUE_H_

#include <cms/CMSException.h>
#include <cms/Config.h>
#include <cms/Destination.h>

namespace cms {

/**
 * An interface encapsulating a provider-specific queue name.
 *
 * Messages sent to a Queue are sent to a Single Subscriber on that Queue Destination.
 * This allows for Queues to be used as load balances implementing a SEDA based architecture.
 * The length of time that a Provider will store a Message in a Queue is not defined by
 * the CMS API, consult your Provider documentation for this information.
 *
 * @since 1.0
 */
class CMS_API Queue : public virtual Destination {
 public:
  virtual ~Queue(){};

  /**
   * Gets the name of this queue.
   *
   * @return The queue name.
   *
   * @throws CMSException - If an internal error occurs.
   */
  virtual std::string getQueueName() const = 0;
};
}  // namespace cms

#endif /*_CMS_QUEUE_H_*/
