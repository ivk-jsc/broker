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

#include "MainPageReplacer.h"
#include "Defines.h"
#include "Connection.h"
#include "About.h"

MainPageReplacer::MainPageReplacer(std::string pageName) : TemplateParamReplacer(std::move(pageName)) {
  addReplacer(MakeStringify(brokerName), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerNameReplacer);
  addReplacer(MakeStringify(brokerVersion), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerVersionReplacer);
  addReplacer(MakeStringify(brokerPort), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerPortReplacer);
  addReplacer(MakeStringify(brokerLogP), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerLogPriorityReplacer);
  addReplacer(MakeStringify(brokerSDBMS), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerStorageDBMSReplacer);
  addReplacer(MakeStringify(brokerSConnection), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerStorageConnectionReplacer);
  addReplacer(MakeStringify(brokerSData), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerStorageDataReplacer);
  addReplacer(MakeStringify(brokerDestinationAutocreate), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerDestinationAutocreateReplacer);
  addReplacer(MakeStringify(brokerSDBMSPool), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerSDBMSPoolReplacer);
  addReplacer(MakeStringify(brokerJournal), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerJournalReplacer);
  addReplacer(MakeStringify(brokerReaders), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerReadersReplacer);
  addReplacer(MakeStringify(brokerWriters), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerWritersReplacer);
  addReplacer(MakeStringify(brokerAcceptors), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerAcceptorsReplacer);
  addReplacer(MakeStringify(brokerSubscriptions), (TemplateParamReplacer::Callback)&MainPageReplacer::brokerSubscriptionsReplacer);
}

std::string MainPageReplacer::brokerNameReplacer() { return CONFIGURATION::Instance().name(); }

std::string MainPageReplacer::brokerPortReplacer() { return std::to_string(CONFIGURATION::Instance().port()); }

std::string MainPageReplacer::brokerVersionReplacer() { return upmq::broker::About::version() + " <br> " + upmq::broker::About::commit(); }

std::string MainPageReplacer::brokerLogPriorityReplacer() {
  std::string log;
  switch (ASYNCLOGGER::Instance().logPriority) {
    case 1:
      log = "FATAL ERROR";
      break;
    case 2:
      log = "CRITICAL ERROR";
      break;
    case 3:
      log = "ERROR";
      break;
    case 4:
      log = "WARNING";
      break;
    case 5:
      log = "NOTICE";
      break;
    case 6:
      log = "INFORMATION";
      break;
    case 7:
      log = "DEBUG";
      break;
    case 8:
      log = "TRACE";
      break;
    default:
    case 0:
      log = "NO LOG";
      break;
  }
  return log;
}

std::string MainPageReplacer::brokerStorageDBMSReplacer() { return upmq::broker::Configuration::Storage::typeName(STORAGE_CONFIG.connection.props.dbmsType); }

std::string MainPageReplacer::brokerStorageConnectionReplacer() { return STORAGE_CONFIG.connection.value.get(); }
std::string MainPageReplacer::getH1() const { return "Broker Configuration"; }
std::string MainPageReplacer::brokerStorageDataReplacer() { return STORAGE_CONFIG.data.get().toString(); }

std::string MainPageReplacer::brokerSDBMSPoolReplacer() { return std::to_string(STORAGE_CONFIG.connection.props.connectionPool); }
std::string MainPageReplacer::brokerDestinationAutocreateReplacer() { return DESTINATION_CONFIG.autocreate ? "TRUE" : "FALSE"; }
std::string MainPageReplacer::brokerJournalReplacer() { return STORAGE_CONFIG.messageJournal(); }
std::string MainPageReplacer::brokerReadersReplacer() { return std::to_string(THREADS_CONFIG.readers); }
std::string MainPageReplacer::brokerWritersReplacer() { return std::to_string(THREADS_CONFIG.writers); }
std::string MainPageReplacer::brokerAcceptorsReplacer() { return std::to_string(THREADS_CONFIG.accepters); }
std::string MainPageReplacer::brokerSubscriptionsReplacer() { return std::to_string(THREADS_CONFIG.subscribers); }
