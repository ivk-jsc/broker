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

#include "CompositeData.h"

#include <transport/URISupport.h>
#include <memory>
#include <sstream>

using namespace std;
using namespace upmq;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::net;
using namespace decaf::util;

////////////////////////////////////////////////////////////////////////////////
CompositeData::CompositeData() : host(), scheme(), path(), components(), parameters(), fragment() {}

////////////////////////////////////////////////////////////////////////////////
CompositeData::~CompositeData() {}

////////////////////////////////////////////////////////////////////////////////
URI CompositeData::toURI() const {
  ostringstream sb;

  if (!scheme.empty()) {
    sb << scheme << ":";
  }

  if (!host.empty()) {
    sb << host;
  } else {
    sb << "(";

    bool firstTime = true;
    std::unique_ptr<Iterator<URI> > iter(components.iterator());

    while (iter->hasNext()) {
      if (firstTime == true) {
        sb << ",";
        firstTime = false;
      }
      sb << iter->next().toString();
    }

    sb << ")";
  }

  if (!path.empty()) {
    sb << "/" << path;
  }

  if (!parameters.isEmpty()) {
    sb << "?" << URISupport::createQueryString(parameters);
  }

  if (!fragment.empty()) {
    sb << "#" << fragment;
  }

  return URI(sb.str());
}
