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

#ifndef __ConnectionFactoryImpl_CPP__
#define __ConnectionFactoryImpl_CPP__

#include "ConnectionFactoryImpl.h"

ConnectionFactoryImpl::ConnectionFactoryImpl() : _uri(cms::ConnectionFactory::DEFAULT_URI()) {}

ConnectionFactoryImpl::ConnectionFactoryImpl(const string &brokerURI) : _uri(brokerURI) {}

ConnectionFactoryImpl::~ConnectionFactoryImpl() {}

std::string cms::ConnectionFactory::DEFAULT_URI() { return "tcp://localhost:12345"; }

cms::ConnectionFactory *cms::ConnectionFactory::createCMSConnectionFactory(const string &brokerURI) { return new ConnectionFactoryImpl(brokerURI); }

cms::Connection *ConnectionFactoryImpl::createConnection(const string &username, const string &password, const string &clientId) {
  defaultUsername = username;
  defaultPassword = password;

  try {
    return new ConnectionImpl(_uri, defaultUsername, defaultPassword, clientId);
    // CMSException
    // CMSSecurityException
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Connection *ConnectionFactoryImpl::createConnection(const string &username, const string &password) {
  defaultUsername = username;
  defaultPassword = password;

  try {
    return new ConnectionImpl(_uri, defaultUsername, defaultPassword, EMPTY_STRING);
    // CMSException
    // CMSSecurityException
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

cms::Connection *ConnectionFactoryImpl::createConnection() {
  try {
    return new ConnectionImpl(_uri, defaultUsername, defaultPassword, EMPTY_STRING);
    // CMSException
    // CMSSecurityException
  }
  CATCH_ALL_THROW_CMSEXCEPTION
}

#endif  //__ConnectionFactoryImpl_CPP__
