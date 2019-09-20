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

#include "WebAdminRequestHandler.h"
#include "Configuration.h"
#include "Destination.h"
#include "ClientsPageReplacer.h"
#include "MessagesPageReplacer.h"
#include <Poco/Net/HTMLForm.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <sstream>

std::vector<std::string> &WebAdminRequestHandler::split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> WebAdminRequestHandler::split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

void WebAdminRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &resp) {
  _req = &req;
  _resp = &resp;

  std::string method = _req->getMethod();
  if ((method == "post") || (method == "POST")) {
    onPost();
  } else {
    switch (static_cast<int>(onGet())) {
      case Poco::Net::HTTPResponse::HTTP_OK:
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        break;
      default:
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_GATEWAY);
    }
    std::ostream &out = _resp->send();
    out << _htmlResult;
    out.flush();
  }
}

Poco::Net::HTTPResponse::HTTPStatus WebAdminRequestHandler::onGet() {
  std::string lFile = _req->getURI();
  if ((lFile == "/") || (lFile == "/index.html")) {
    lFile = "/main.html";
  }

  Poco::Path f = Poco::Path::current();
  f.append(lFile).makeFile();

  _htmlResult.clear();

  std::string extension = f.getExtension();
  if ((extension == "html") || extension == "json") {
    _resp->setContentType("text/" + extension);
    std::string tmpWebStr;
    tmpWebStr.assign(lFile.begin() + 1, lFile.end() - 5);
    std::vector<std::string> tmpWebVec = split(tmpWebStr, '/');

    std::string indexPage = "/index." + extension;
    upmq::broker::Configuration::ReplacerMapType &replacerMap = CONFIGURATION::Instance().replacerMap;
    auto indexlIt = replacerMap.find(indexPage);
    if (indexlIt == replacerMap.end()) {
      replacerMap.emplace(indexPage, std::unique_ptr<TemplateParamReplacer>(new IndexPageReplacer(indexPage, nullptr)));
      indexlIt = replacerMap.find(indexPage);
    }

    auto replIt = replacerMap.find(lFile);
    const std::string clientsPage = "/clients." + extension;
    const std::string messagesPage = "/messages." + extension;
    if (replIt == replacerMap.end()) {
      if (lFile == "/main." + extension) {
        std::string sep = (extension == "json") ? " " : "<br>";
        replacerMap.emplace(lFile, std::unique_ptr<TemplateParamReplacer>(new MainPageReplacer(lFile, std::move(sep))));
        replIt = replacerMap.find(lFile);
      }
      if (((tmpWebVec[0] == "queues") || (tmpWebVec[0] == "topics")) && tmpWebVec.size() == 1) {
        if (lFile == "/queues." + extension) {
          replacerMap.emplace(lFile, std::unique_ptr<TemplateParamReplacer>(new QueuesPageReplacer(lFile)));
          replIt = replacerMap.find(lFile);
        }
        if (lFile == "/topics." + extension) {
          replacerMap.emplace(lFile, std::unique_ptr<TemplateParamReplacer>(new TopicsPageReplacer(lFile)));
          replIt = replacerMap.find(lFile);
        }
      } else if (((tmpWebVec[0] == "destinations") || (tmpWebVec[0] == "messages")) && tmpWebVec.size() > 1) {
        if ((tmpWebVec[0] == "destinations") || (tmpWebVec[0] == "topics")) {
          replacerMap.emplace(clientsPage,
                              std::unique_ptr<TemplateParamReplacer>(new ClientsPageReplacer(
                                  clientsPage, tmpWebVec[1], static_cast<int>(upmq::broker::Destination::type(tmpWebVec[2])))));
          replIt = replacerMap.find(clientsPage);
        } else if (tmpWebVec[0] == "messages") {
          replacerMap.emplace(messagesPage,
                              std::unique_ptr<TemplateParamReplacer>(new MessagesPageReplacer(
                                  messagesPage, tmpWebVec[1], static_cast<int>(upmq::broker::Destination::type(tmpWebVec[2])))));
          replIt = replacerMap.find(messagesPage);
        }
      }
    }

    if (replIt != replacerMap.end()) {
      if (extension != "json" && indexlIt != replIt) {
        dynamic_cast<IndexPageReplacer *>(indexlIt->second.get())->setChildReplacer(replIt->second.get());
        _htmlResult = dynamic_cast<IndexPageReplacer *>(indexlIt->second.get())->replace();
      } else if (indexlIt == replIt) {
        _htmlResult = "";
      } else {
        _htmlResult = replIt->second->replace();
      }

      if ((replIt->first == clientsPage) || (replIt->first == (messagesPage))) {
        replacerMap.erase(replIt);
      }
    }
  } else {
    if (extension == "css") {
      _resp->setContentType("text/css");
    } else {
      _resp->setContentType("*/*");
    }
    _htmlResult = TemplateParamReplacer::loadTemplate(lFile);
  }

  if (!_htmlResult.empty()) {
    return Poco::Net::HTTPResponse::HTTP_OK;
  }
  return Poco::Net::HTTPResponse::HTTP_BAD_GATEWAY;
}

void WebAdminRequestHandler::onPost() {
  Poco::Net::HTMLForm form(*_req, _req->stream());
  auto it = form.find("add");
  if (it != form.end()) {
    onAddDestination(form);
    return;
  }
  it = form.find("del");
  if (it != form.end()) {
    onDelDestination(form);
    return;
  }
  it = form.find("add-message-button");
  if (it != form.end()) {
    onSendMessage(form);
    return;
  }
  it = form.find("del-message-button");
  if (it != form.end()) {
    onDelMessage(form);
    return;
  }
  it = form.find("del-all-message-button");
  if (it != form.end()) {
    onDelMessage(form);
    return;
  }
}

void WebAdminRequestHandler::onAddDestination(Poco::Net::HTMLForm &form) {
  (void)form;
  std::string respRedirect;
  //  Storage::_StorageInfo storageInfo = prepareDestination(form);
  //  _myServerApp->_storageRegestry->addStorage(storageInfo);
  //  if ((storageInfo.type == Storage::QUEUE) || (storageInfo.type == Storage::TEMPORARY_QUEUE)) {
  respRedirect = "/queues.html";
  //  }
  //  else {
  //    respRedirect = "/topics.html";
  //  }
  _resp->redirect(respRedirect, Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY);
}

void WebAdminRequestHandler::onDelDestination(Poco::Net::HTMLForm &form) {
  (void)form;
  std::string respRedirect;
  //  Storage::_StorageInfo storageInfo = prepareDestination(form);
  //  _myServerApp->_storageRegestry->delStorage(storageInfo);
  //  if ((storageInfo.type == Storage::QUEUE) || (storageInfo.type == Storage::TEMPORARY_QUEUE)) {
  respRedirect = "/queues.html";
  //  }
  //  else {
  //    respRedirect = "/topics.html";
  //  }
  _resp->redirect(respRedirect, Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY);
}

void WebAdminRequestHandler::onSendMessage(Poco::Net::HTMLForm &form) {
  std::string lFile = _req->getURI();
  std::string tmpWebStr;
  tmpWebStr.assign(lFile.begin() + 1, lFile.end() - 5);
  std::vector<std::string> tmpWebVec = split(tmpWebStr, '/');

  if (tmpWebVec.size() < 3) {
    _resp->redirect("/", Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY);
    return;
  }
  Poco::Net::NameValueCollection::ConstIterator it = form.find("message");
  if (it == form.end()) {
    _resp->redirect(lFile, Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY);
    return;
  }

  // std::string destination = tmpWebVec[1];
  //  cms::Destination::DestinationType destinationType =
  //  static_cast<cms::Destination::DestinationType>(Storage::_StorageInfo::nameType(tmpWebVec[2])); try {
  //    unique_ptr<cms::ConnectionFactory> connectionFactory(cms::ConnectionFactory::createCMSConnectionFactory(std::string("tcp://127.0.0.1:") +
  //    std::to_string((long long) brokerCFG.port))); unique_ptr<cms::Connection> connection(connectionFactory->createConnection());
  //    connection->start();
  //    unique_ptr<cms::Session> session(connection->createSession(cms::Session::AUTO_ACKNOWLEDGE));
  //    unique_ptr<cms::Destination> destinationTo(getDestination(session.get(), destination, destinationType));
  //    unique_ptr<cms::MessageProducer> producer(session->createProducer(destinationTo.get()));
  //    unique_ptr<cms::TextMessage> textMessage(session->createTextMessage(it->second));
  //    producer->send(textMessage.get());
  //    _resp->redirect(lFile, HTTPResponse::HTTP_MOVED_PERMANENTLY);
  //  }
  //  catch (cms::CMSException &cmex) {
  //    cmex.printStackTrace();
  //  }
  //  catch (std::runtime_error &rex) {
  //    cout << rex.what() << endl;
  //  }
  //  catch (...) {
  //    cout << "unknown exception : onSendMessage " << endl;
  //  }
}

void WebAdminRequestHandler::onDelMessage(Poco::Net::HTMLForm &form) {
  (void)form;
  std::string lFile = _req->getURI();
  //  std::string tmpWebStr;
  //  tmpWebStr.assign(lFile.begin() + 1, lFile.end() - 5);
  //  std::vector<std::string> tmpWebVec = split(tmpWebStr, '/');
  //  bool delAll = false;
  //  if (tmpWebVec.size() < 3) {
  //    _resp->redirect("/", HTTPResponse::HTTP_MOVED_PERMANENTLY);
  //    return;
  //  }
  //  NameValueCollection::ConstIterator it = form.find("message-id");
  //  if (it == form.end()) {
  //    it = form.find("del-all-message-button");
  //    if (it == form.end()) {
  //      _resp->redirect(lFile, HTTPResponse::HTTP_MOVED_PERMANENTLY);
  //      return;
  //    }
  //    else {
  //      delAll = true;
  //    }
  //  }
  //
  //
  //  std::string destination = tmpWebVec[1];
  //  Storage::Type destinationType = Storage::_StorageInfo::nameType(tmpWebVec[2]);
  //  Storage *pStorage = _myServerApp->_storageFactory->addStorage(brokerCFG.storageConnection,
  //                                                                brokerCFG.storageData,
  //                                                                destination,
  //                                                                destinationType,
  //                                                                static_cast<Storage::DBMSType>(brokerCFG.storageDBMSType));
  //  if (pStorage) {
  //    IStorageSession *pSession = pStorage->createSession(Storage::AUTO_ACKNOWLEDGE);
  //    if (pSession) {
  //      IStorageClient *client = pSession->addClient(IStorageClient::CONSUMER, "", IStorageClient::ONLINE, "");
  //      if (client) {
  //        if (!delAll) {
  //          pSession->deleteMessage(client, it->second, 0);
  //        }
  //        else {
  //          pSession->deleteMessageAll(client);
  //        }
  //      }
  //      pStorage->closeSession(pSession->getId());
  //    }
  //    _myServerApp->_storageFactory->delStorage(pStorage->getInfo().name, pStorage->getInfo().type);
  //  }

  _resp->redirect(lFile, Poco::Net::HTTPResponse::HTTP_MOVED_PERMANENTLY);
}

// Storage::_StorageInfo WebAdminRequestHandler::prepareDestination(HTMLForm &form) {
//  std::string dataPath;
//  std::string connectionString;
//  std::string dbms;
//
//  Storage::DBMSType dbmsType = static_cast<Storage::DBMSType>(brokerCFG.storageDBMSType);
//  try {
//    dataPath = form.get("dataPath");
//  }
//  catch (Poco::NotFoundException &nef) { }
//  if ((dataPath == "DEFAULT") || dataPath.empty()) {
//    dataPath = brokerCFG.storageData;
//  }
//  try {
//    connectionString = form.get("connectionString");
//  }
//  catch (Poco::NotFoundException &nef) { }
//  if ((connectionString == "DEFAULT") || (connectionString.empty())) {
//    connectionString = brokerCFG.storageConnection;
//  }
//  dbms = form.get("dbms");
//  if (dbms != "DEFAULT") {
//    dbmsType = Storage::_StorageInfo::dbmsNameType(dbms);
//  }
//  Storage::_StorageInfo storageInfo(connectionString,
//                                    dataPath,
//                                    form.get("destination"),
//                                    Storage::_StorageInfo::nameType(form.get("type")),
//                                    dbmsType);
//  return storageInfo;
//}

// cms::Destination *WebAdminRequestHandler::getDestination(cms::Session *session,
//                                                         const std::string &dName,
//                                                         cms::Destination::DestinationType destType) const {
//  cms::Destination *dest = NULL;
//
//  switch (static_cast<int>(destType)) {
//    case cms::Destination::QUEUE:
//      dest = session->createQueue(dName);
//      break;
//    case cms::Destination::TOPIC:
//      dest = session->createTopic(dName);
//      break;
//    case cms::Destination::TEMP_QUEUE:
//      dest = session->createTemporaryQueue();
//      break;
//    case cms::Destination::TEMP_TOPIC:
//      dest = session->createTemporaryTopic();
//      break;
//    default:
//    case cms::Destination::DESTINATION_NULL:
//      break;
//  }
//  return dest;
//}
