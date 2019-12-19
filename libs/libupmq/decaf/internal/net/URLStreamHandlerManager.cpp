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

#include <decaf/internal/net/URLStreamHandlerManager.h>

#include <decaf/internal/net/Network.h>
#include <decaf/lang/Exception.h>
#include <decaf/lang/Runnable.h>
#include <decaf/lang/exceptions/RuntimeException.h>
#include <decaf/net/URLStreamHandler.h>
#include <decaf/net/URLStreamHandlerFactory.h>

#include <decaf/internal/net/file/FileHandler.h>
#include <decaf/internal/net/http/HttpHandler.h>
#include <decaf/internal/net/https/HttpsHandler.h>

using namespace decaf;
using namespace decaf::internal;
using namespace decaf::internal::net;
using namespace decaf::internal::net::http;
using namespace decaf::internal::net::https;
using namespace decaf::internal::net::file;
using namespace decaf::net;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
URLStreamHandlerManager *URLStreamHandlerManager::instance;

////////////////////////////////////////////////////////////////////////////////
namespace {

class ShutdownTask : public decaf::lang::Runnable {
 private:
  URLStreamHandlerManager **defaultRef;

 private:
  ShutdownTask(const ShutdownTask &);
  ShutdownTask &operator=(const ShutdownTask &);

 public:
  ShutdownTask(URLStreamHandlerManager **defaultRef_) : defaultRef(defaultRef_) {}
  virtual ~ShutdownTask() {}

  virtual void run() override { *defaultRef = nullptr; }
};
}  // namespace

////////////////////////////////////////////////////////////////////////////////
namespace decaf {
namespace internal {
namespace net {

class URLStreamHandlerManagerImpl {
 public:
  URLStreamHandlerFactory *factory;

 public:
  URLStreamHandlerManagerImpl() : factory(nullptr) {}

  virtual ~URLStreamHandlerManagerImpl() {
    try {
      delete factory;
    } catch (...) {
    }
  }
};
}  // namespace net
}  // namespace internal
}  // namespace decaf

////////////////////////////////////////////////////////////////////////////////
URLStreamHandlerManager::URLStreamHandlerManager() : impl(new URLStreamHandlerManagerImpl) {}

////////////////////////////////////////////////////////////////////////////////
URLStreamHandlerManager::~URLStreamHandlerManager() {
  try {
    delete impl;
  }
  DECAF_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
URLStreamHandlerManager *URLStreamHandlerManager::getInstance() {
  if (instance == nullptr) {
    synchronized(Network::getNetworkRuntime()->getRuntimeLock()) {
      if (instance != nullptr) {
        return instance;
      }

      instance = new URLStreamHandlerManager;

      // Store the default in the Network Runtime, it will be destroyed when the
      // Application calls the Decaf shutdownLibrary method.
      Network::getNetworkRuntime()->addAsResource(instance);
      Network::getNetworkRuntime()->addShutdownTask(new ShutdownTask(&instance));
    }
  }

  return instance;
}

////////////////////////////////////////////////////////////////////////////////
void URLStreamHandlerManager::setURLStreamHandlerFactory(URLStreamHandlerFactory *factory) {
  synchronized(Network::getNetworkRuntime()->getRuntimeLock()) {
    if (impl->factory != nullptr) {
      throw RuntimeException(__FILE__, __LINE__, "Application already set a URLStreamHandlerFactory");
    }

    impl->factory = factory;
  }
}

////////////////////////////////////////////////////////////////////////////////
URLStreamHandler *URLStreamHandlerManager::getURLStreamHandler(const decaf::lang::String &protocol) {
  URLStreamHandler *streamHandler = nullptr;

  synchronized(Network::getNetworkRuntime()->getRuntimeLock()) {
    // If there is a stream handler factory, then attempt to
    // use it to create the handler.
    if (impl->factory != nullptr) {
      streamHandler = impl->factory->createURLStreamHandler(protocol.toString());
      if (streamHandler != nullptr) {
        return streamHandler;
      }
    }

    // No one else has provided a handler, so try our internal one.
    if (protocol.equalsIgnoreCase("http")) {
      return new HttpHandler;
    } else if (protocol.equalsIgnoreCase("https")) {
      return new HttpsHandler;
    } else if (protocol.equalsIgnoreCase("file")) {
      return new FileHandler;
    }

    // TODO we should cache the stream handlers and return the cached version
    //      we just need to ensure we manage the lifetime from within this object.
  }

  return streamHandler;
}
