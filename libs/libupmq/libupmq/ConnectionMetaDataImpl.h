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

#ifndef __ConnectionMetaDataImpl_H__
#define __ConnectionMetaDataImpl_H__

#include <cms/ConnectionMetaData.h>
#include <sstream>

#include "ProtoHeader.h"

using namespace std;

class ConnectionMetaDataImpl : public cms::ConnectionMetaData {
 public:
  ConnectionMetaDataImpl();
  ~ConnectionMetaDataImpl();

  string getCMSVersion() const override;

  int getCMSMajorVersion() const override;

  int getCMSMinorVersion() const override;

  string getCMSProviderName() const override;

  string getProviderVersion() const override;

  int getProviderMajorVersion() const override;

  int getProviderMinorVersion() const override;

  int getProviderPatchVersion() const override;

  vector<string> getCMSXPropertyNames() const override;

  static const int CMS_MAJOR;
  static const int CMS_MINOR;
  static const string CMS_PROVIDER;

  Proto::ServerVersion _sv;
};

#endif  //__ConnectionMetaDataImpl_H__
