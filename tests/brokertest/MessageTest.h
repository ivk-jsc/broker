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

#ifndef _MESSAGETEST_H_
#define _MESSAGETEST_H_

#include <gtest/gtest.h>
#include "CMSProvider.h"
#include "IntegrationCommon.h"

using namespace cms;

class MessageTest : public ::testing::Test {
 protected:
  bool readOnlyMessage = false;
  std::string cmsMessageId;
  std::string cmsCorrelationID;
  std::unique_ptr<Destination> cmsDestination;
  std::unique_ptr<Destination> cmsReplyTo;
  int cmsDeliveryMode = 0;
  bool cmsRedelivered = false;
  std::string cmsType;
  long long cmsExpiration = 0;
  int cmsPriority = 4;
  long long cmsTimestamp = 0;

  std::vector<long long> consumerIDs{};

  MessageTest() = default;

  ~MessageTest() override = default;

  void SetUp() override;

  void TearDown() override;

  std::unique_ptr<CMSProvider> cmsProvider;

  std::string getBrokerURL() const { return IntegrationCommon::getInstance().getOpenwireURL(); }
};

#endif /*_MESSAGETEST_H_*/
