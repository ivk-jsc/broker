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

#include "AbstractTransportFactory.h"

#include "transport/UPMQWireFormat.h"

using namespace upmq;
using namespace upmq::transport;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::net;
using namespace decaf::util;

////////////////////////////////////////////////////////////////////////////////
AbstractTransportFactory::~AbstractTransportFactory() {}

////////////////////////////////////////////////////////////////////////////////
Pointer<WireFormat> AbstractTransportFactory::createWireFormat(const decaf::util::Properties &properties) {
  DECAF_UNUSED_VAR(properties);
  try {
    return Pointer<WireFormat>(new UPMQWireFormat());
  }
  AMQ_CATCH_RETHROW(NoSuchElementException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, NoSuchElementException)
  AMQ_CATCHALL_THROW(NoSuchElementException)
}
