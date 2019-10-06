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

#ifndef IVK_BROKER_WEBADMINREQUESTHANDLER_H
#define IVK_BROKER_WEBADMINREQUESTHANDLER_H

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTMLForm.h>

#include "MainPageReplacer.h"
#include "IndexPageReplacer.h"
#include "QueuesPageReplacer.h"
#include "TopicsPageReplacer.h"

#include <memory>

class WebAdminApp;

class WebAdminRequestHandler : public Poco::Net::HTTPRequestHandler {
 public:
  void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp) override;

  Poco::Net::HTTPResponse::HTTPStatus onGet();
  void onPost();

 private:
  Poco::Net::HTTPServerRequest *_req;
  Poco::Net::HTTPServerResponse *_resp;
  std::string _htmlResult;
  std::vector<std::string> split(const std::string &s, char delim);
  std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
  void onAddDestination(Poco::Net::HTMLForm &form);
  void onDelDestination(Poco::Net::HTMLForm &form);
  //  Storage::_StorageInfo prepareDestination(Poco::Net::HTMLForm &form);
  void onSendMessage(Poco::Net::HTMLForm &form);
  void onDelMessage(Poco::Net::HTMLForm &form);
  //  cms::Destination *getDestination(cms::Session *session, const string &dName, cms::Destination::DestinationType destType) const;
};

#endif  // IVK_BROKER_WEBADMINREQUESTHANDLER_H
