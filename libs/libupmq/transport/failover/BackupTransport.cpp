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

#include "BackupTransport.h"

#include <transport/failover/BackupTransportPool.h>

using namespace upmq;
using namespace upmq::transport;
using namespace upmq::transport::failover;

////////////////////////////////////////////////////////////////////////////////
BackupTransport::BackupTransport(BackupTransportPool *parent) : parent(parent), closed(true), priority(false) {}

////////////////////////////////////////////////////////////////////////////////
BackupTransport::~BackupTransport() {}

decaf::net::URI BackupTransport::getUri() const { return this->uri; }

void BackupTransport::setUri(const decaf::net::URI &newUri) { this->uri = newUri; }

const Pointer<Transport> &BackupTransport::getTransport() const { return transport; }

void BackupTransport::setTransport(Pointer<Transport> newTransport) {
  this->transport = std::move(newTransport);

  if (this->transport != nullptr) {
    this->transport->setTransportListener(this);
  }
}

////////////////////////////////////////////////////////////////////////////////
void BackupTransport::onException(const decaf::lang::Exception &) {
  this->closed = true;

  if (this->parent != nullptr) {
    this->parent->onBackupTransportFailure(this);
  }
}

bool BackupTransport::isClosed() const { return this->closed; }

void BackupTransport::setClosed(bool value) { this->closed = value; }

bool BackupTransport::isPriority() const { return this->priority; }

void BackupTransport::setPriority(bool value) { this->priority = value; }
