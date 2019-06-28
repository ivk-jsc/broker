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

#ifndef __ConnectionMetaDataImpl_CPP__
#define __ConnectionMetaDataImpl_CPP__

#include "ConnectionMetaDataImpl.h"

const int ConnectionMetaDataImpl::CMS_MAJOR = 1;
const int ConnectionMetaDataImpl::CMS_MINOR = 0;
const string ConnectionMetaDataImpl::CMS_PROVIDER = "upmq";

ConnectionMetaDataImpl::ConnectionMetaDataImpl() {}

ConnectionMetaDataImpl::~ConnectionMetaDataImpl() {}

string ConnectionMetaDataImpl::getCMSVersion() const {
  stringstream str;
  str << CMS_MAJOR << "." << CMS_MINOR;
  return str.str();
}

int ConnectionMetaDataImpl::getCMSMajorVersion() const { return CMS_MAJOR; }

int ConnectionMetaDataImpl::getCMSMinorVersion() const { return CMS_MINOR; }

string ConnectionMetaDataImpl::getCMSProviderName() const { return CMS_PROVIDER; }

string ConnectionMetaDataImpl::getProviderVersion() const {
  stringstream str;
  str << _sv.server_major_version() << "." << _sv.server_minor_version() << "." << _sv.server_revision_version();
  return str.str();
}

int ConnectionMetaDataImpl::getProviderMajorVersion() const { return _sv.server_major_version(); }

int ConnectionMetaDataImpl::getProviderMinorVersion() const { return _sv.server_minor_version(); }

int ConnectionMetaDataImpl::getProviderPatchVersion() const { return _sv.server_revision_version(); }

vector<string> ConnectionMetaDataImpl::getCMSXPropertyNames() const {
  std::vector<string> vec;
  vec.push_back("JMSXUserID");
  vec.push_back("JMSXAppID");
  vec.push_back("JMSXDeliveryCount");
  vec.push_back("JMSXProducerTXID");
  vec.push_back("JMSXConsumerTXID");
  vec.push_back("JMSXRcvTimestamp");
  vec.push_back("JMSXState");

  vec.push_back("JMSXGroupID");
  vec.push_back("JMSXGroupSeq");
  return vec;
}

#endif  //__ConnectionMetaDataImpl_CPP__
