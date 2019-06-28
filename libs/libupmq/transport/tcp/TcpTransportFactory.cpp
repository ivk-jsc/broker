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

#include "TcpTransportFactory.h"

#include <transport/IOTransport.h>
#include <transport/URISupport.h>
#include <transport/WireFormat.h>
#include <transport/correlator/ResponseCorrelator.h>
#include <transport/inactivity/InactivityMonitor.h>
#include <transport/logging/LoggingTransport.h>
#include <transport/tcp/TcpTransport.h>

#include <decaf/lang/Boolean.h>
#include <decaf/lang/Integer.h>
#include <decaf/util/Properties.h>

using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::tcp;
using namespace upmq::transport::inactivity;
using namespace upmq::transport::logging;
using namespace upmq::transport::correlator;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> TcpTransportFactory::create(const decaf::net::URI &location) {
  try {
    Properties properties = upmq::transport::URISupport::parseQuery(location.getQuery());

    Pointer<WireFormat> wireFormat = this->createWireFormat(properties);

    // Create the initial Composite Transport, then wrap it in the normal Filters
    // for a non-composite Transport which right now is just a ResponseCorrelator
    Pointer<Transport> transport(doCreateComposite(location, wireFormat, properties));

    transport.reset(new ResponseCorrelator(transport));

    return transport;
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> TcpTransportFactory::createComposite(const decaf::net::URI &location) {
  try {
    Properties properties = upmq::transport::URISupport::parseQuery(location.getQuery());

    Pointer<WireFormat> wireFormat = this->createWireFormat(properties);

    // Create the initial Transport, then wrap it in the normal Filters
    return doCreateComposite(location, wireFormat, properties);
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
Pointer<Transport> TcpTransportFactory::doCreateComposite(const decaf::net::URI &location, const Pointer<transport::WireFormat> wireFormat, const decaf::util::Properties &properties) {
  try {
    Pointer<Transport> transport(new IOTransport(wireFormat));

    transport.reset(new TcpTransport(transport, location));

    // Give this class and any derived classes a chance to apply value that
    // are set in the properties object.
    doConfigureTransport(transport, properties);

    if (int period = Integer::parseInt(properties.getProperty("transport.ping", "300000"))) {
      transport.reset(new InactivityMonitor(transport, properties, wireFormat, 60000, period));
    }

    // If command tracing was enabled, wrap the transport with a logging transport.
    // We support the old CMS value, the UPMQ trace value and the NMS useLogging
    // value in order to be more friendly.
    if (properties.getProperty("transport.trace", "false") == "true") {
      transport.reset(new LoggingTransport(transport));
    }

    return transport;
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}

////////////////////////////////////////////////////////////////////////////////
void TcpTransportFactory::doConfigureTransport(Pointer<Transport> transport, const decaf::util::Properties &properties) {
  try {
    Pointer<TcpTransport> tcp = transport.dynamicCast<TcpTransport>();

    tcp->setTrace(Boolean::parseBoolean(properties.getProperty("transport.tcpTracingEnabled", "false")));
    tcp->setInputBufferSize(Integer::parseInt(properties.getProperty("inputBufferSize", "8192")));
    tcp->setOutputBufferSize(Integer::parseInt(properties.getProperty("outputBufferSize", "8192")));
    tcp->setLinger(Integer::parseInt(properties.getProperty("soLinger", "-1")));
    tcp->setKeepAlive(Boolean::parseBoolean(properties.getProperty("soKeepAlive", "false")));
    tcp->setReceiveBufferSize(Integer::parseInt(properties.getProperty("soReceiveBufferSize", "-1")));
    tcp->setSendBufferSize(Integer::parseInt(properties.getProperty("soSendBufferSize", "-1")));
    tcp->setTcpNoDelay(Boolean::parseBoolean(properties.getProperty("tcpNoDelay", "true")));
    tcp->setConnectTimeout(Integer::parseInt(properties.getProperty("soConnectTimeout", "0")));
  }
  AMQ_CATCH_RETHROW(UPMQException)
  AMQ_CATCH_EXCEPTION_CONVERT(Exception, UPMQException)
  AMQ_CATCHALL_THROW(UPMQException)
}
