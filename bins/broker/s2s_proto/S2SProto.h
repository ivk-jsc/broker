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

#ifndef BROKER_S2SPROTO_H
#define BROKER_S2SPROTO_H

#include <string>

#ifndef make_string
#define make_string(__x__) #__x__
#endif

#ifndef declare_static_string
#define declare_static_string(__s__) constexpr char(__s__)[] = make_string(__s__)
#endif

namespace upmq {
namespace broker {
namespace s2s {
namespace proto {

declare_static_string(upmq_fwd_header);
declare_static_string(upmq_data_link);
declare_static_string(upmq_data_parts_number);
declare_static_string(upmq_data_parts_count);
declare_static_string(upmq_data_part_size);
declare_static_string(upmq_data_size);

declare_static_string(upmq_s2s_source_destination_name);
declare_static_string(upmq_s2s_source_destination_type);
declare_static_string(upmq_s2s_destination_broker_name);
declare_static_string(upmq_s2s_source_broker_name);
}  // namespace proto
}  // namespace s2s
}  // namespace broker
}  // namespace upmq

#endif  // BROKER_S2SPROTO_H
