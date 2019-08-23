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

#ifndef IVK_UPMQ_MAINPAGEREPLACER_H
#define IVK_UPMQ_MAINPAGEREPLACER_H

#include "TemplateParamReplacer.h"

class MainPageReplacer : public TemplateParamReplacer {
 public:
  enum MainParam {
    brokerName = 0,
    brokerVersion,
    brokerPort,
    brokerLogP,
    brokerLogInteractive,
    brokerLogPath,
    brokerSDBMS,
    brokerSDBMSPool,
    brokerSConnection,
    brokerSData,
    brokerSPath,
    brokerDestinations,
    brokerDestinationAutocreate,
    brokerJournal,
    brokerReaders,
    brokerWriters,
    brokerAcceptors,
    brokerSubscriptionWorkers,
    brokerNetClients,
    brokerSessions,
    brokerSubscriptions
  };

  explicit MainPageReplacer(std::string pageName);

  std::string brokerNameReplacer();

  std::string brokerVersionReplacer();

  std::string brokerPortReplacer();

  std::string brokerLogPriorityReplacer();

  std::string brokerLogInteractiveReplacer();

  std::string brokerLogPathReplacer();

  std::string brokerSDBMSPoolReplacer();

  std::string brokerDestinationReplacer();
  std::string brokerDestinationAutocreateReplacer();

  std::string brokerJournalReplacer();

  std::string brokerReadersReplacer();
  std::string brokerWritersReplacer();
  std::string brokerAcceptorsReplacer();
  std::string brokerSubscriptionWorkersReplacer();

  std::string brokerStorageDBMSReplacer();

  std::string brokerStorageConnectionReplacer();

  std::string brokerStorageDataReplacer();

  std::string brokerStorageDBPathReplacer();

  std::string brokerNetClientsReplacer();

  std::string brokerSessionsReplacer();

  std::string brokerSubscriptionsReplacer();

  std::string getH1() const override;
};

#endif  // IVK_UPMQ_MAINPAGEREPLACER_H
