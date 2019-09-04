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

#include "FailoverTransportFactory.h"

#include <transport/CompositeData.h>
#include <transport/URISupport.h>
#include <transport/correlator/ResponseCorrelator.h>
#include <transport/failover/FailoverTransport.h>

#include <decaf/lang/Boolean.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>

using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::failover;
using namespace upmq::transport::correlator;
using namespace decaf;
using namespace decaf::util;
using namespace decaf::lang;

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> FailoverTransportFactory::create(const decaf::net::URI &location) {
  try {
    Properties properties;  // unused but necessary for now.

    // Create the initial Transport, then wrap it in the normal Filters
    Pointer<Transport> transport(doCreateComposite(location, properties));

    // Create the Transport for response correlator
    transport.reset(new ResponseCorrelator(transport));

    return transport;
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> FailoverTransportFactory::createComposite(const decaf::net::URI &location) {
  try {
    Properties properties;  // unused but necessary for now.

    // Create the initial Transport, then wrap it in the normal Filters
    return doCreateComposite(location, properties);
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> FailoverTransportFactory::doCreateComposite(const decaf::net::URI &location,
                                                               const decaf::util::Properties &properties UPMQCPP_UNUSED) {
  DECAF_UNUSED_VAR(properties);
  try {
    CompositeData data = URISupport::parseComposite(location);
    Pointer<FailoverTransport> transport(new FailoverTransport());

    Properties topLvlProperties = data.getParameters();

    transport->setInitialReconnectDelay(Long::parseLong(topLvlProperties.getProperty("initialReconnectDelay", "10")));
    transport->setMaxReconnectDelay(Long::parseLong(topLvlProperties.getProperty("maxReconnectDelay", "30000")));
    transport->setUseExponentialBackOff(Boolean::parseBoolean(topLvlProperties.getProperty("useExponentialBackOff", "true")));
    transport->setMaxReconnectAttempts(Integer::parseInt(topLvlProperties.getProperty("maxReconnectAttempts", "-1")));
    transport->setStartupMaxReconnectAttempts(Integer::parseInt(topLvlProperties.getProperty("startupMaxReconnectAttempts", "-1")));
    transport->setRandomize(Boolean::parseBoolean(topLvlProperties.getProperty("randomize", "false")));
    transport->setBackup(Boolean::parseBoolean(topLvlProperties.getProperty("backup", "false")));
    transport->setBackupPoolSize(Integer::parseInt(topLvlProperties.getProperty("backupPoolSize", "1")));
    transport->setTimeout(Long::parseLong(topLvlProperties.getProperty("timeout", "-1")));
    transport->setTrackMessages(Boolean::parseBoolean(topLvlProperties.getProperty("trackMessages", "false")));
    transport->setMaxCacheSize(Integer::parseInt(topLvlProperties.getProperty("maxCacheSize", "131072")));
    transport->setMaxPullCacheSize(Integer::parseInt(topLvlProperties.getProperty("maxPullCacheSize", "10")));
    transport->setUpdateURIsSupported(Boolean::parseBoolean(topLvlProperties.getProperty("updateURIsSupported", "true")));
    transport->setPriorityBackup(Boolean::parseBoolean(topLvlProperties.getProperty("priorityBackup", "false")));
    transport->setPriorityURIs(topLvlProperties.getProperty("priorityURIs", ""));

    transport->addURI(false, data.getComponents());

    return transport.dynamicCast<Transport>();
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}
