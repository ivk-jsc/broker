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

#include "About.h"

#include "ProtoBuf.h"
#include "Version.hpp"

namespace upmq {
namespace broker {

std::string About::version() {
  std::stringstream ss;
  Proto::ServerVersion version;
  ss << version.server_major_version() << "." << version.server_minor_version() << "." << MQ_VERSION_REVISION;
  return ss.str();
}

std::string About::author() {
  std::stringstream ss;
  ss << MQ_COMMITTER_NAME << " (" << MQ_COMMITTER_EMAIL << ")";
  return ss.str();
}

std::string About::commit(const std::string &delimeter) {
  std::stringstream ss;
  ss << MQ_COMMITTER_DATE << delimeter << "full-sha [" << MQ_COMMITTER_FULLSHA << "] " << delimeter << "short-sha [" << MQ_COMMITTER_SHORTSHA << "] "
     << delimeter << "commit [" << MQ_COMMITTER_NOTE << "]";
  return ss.str();
}

std::string About::vendor() {
  Proto::ServerVersion version;
  return version.server_vendor_id();
}

static const char *broker_version =
    "SWF : BROKER VER : " VER_FILEVERSION_STR " " MQ_COMMITTER_DATE " [" MQ_COMMITTER_FULLSHA "] (" MQ_COMMITTER_SHORTSHA ") " MQ_COMMITTER_NOTE;

void unused() {(void)broker_version;}
}  // namespace broker
}  // namespace upmq
