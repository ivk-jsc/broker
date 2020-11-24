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

#ifndef BROKER_MESSAGEDEFINES_H
#define BROKER_MESSAGEDEFINES_H

namespace message {

enum _DeliveryStatus { NOT_SENT = 0, WAS_SENT = 1, DELIVERED = 2 };

static const int MAX_COUNT_PRIORITY = 10;

enum GroupStatus { NOT_IN_GROUP = 0, ONE_OF_GROUP = 1, LAST_IN_GROUP = 2 };
}  // namespace message
#endif  // BROKER_MESSAGEDEFINES_H
