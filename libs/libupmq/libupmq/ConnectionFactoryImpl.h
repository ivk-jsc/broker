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

#ifndef __ConnectionFactoryImpl_H__
#define __ConnectionFactoryImpl_H__

#include <cms/ConnectionFactory.h>

#include "ConnectionImpl.h"
#include "ExceptionImpl.h"

using namespace std;

class CMS_API ConnectionFactoryImpl : public cms::ConnectionFactory {
 public:
  ConnectionFactoryImpl();
  ConnectionFactoryImpl(const string &brokerURI);
  ~ConnectionFactoryImpl();

  cms::Connection *createConnection() override;
  cms::Connection *createConnection(const string &username, const string &password) override;
  cms::Connection *createConnection(const string &username, const string &password, const string &clientId) override;

 private:
  string defaultUsername;
  string defaultPassword;

  string _objectId;
  string _uri;
};

#endif  //__ConnectionFactoryImpl_H__
