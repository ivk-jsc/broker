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

#include "FileDescriptor.h"

using namespace decaf;
using namespace decaf::io;

////////////////////////////////////////////////////////////////////////////////
FileDescriptor FileDescriptor::in(0, true);
FileDescriptor FileDescriptor::out(1, false);
FileDescriptor FileDescriptor::err(2, false);

////////////////////////////////////////////////////////////////////////////////
FileDescriptor::FileDescriptor() : descriptor(-1), readonly(false) {}

////////////////////////////////////////////////////////////////////////////////
FileDescriptor::FileDescriptor(long long value, bool readonly) : descriptor(value), readonly(readonly) {}

////////////////////////////////////////////////////////////////////////////////
FileDescriptor::~FileDescriptor() = default;

////////////////////////////////////////////////////////////////////////////////
void FileDescriptor::sync() {
  if (!this->readonly) {
    // TODO - Implement a Sync method for each OS.
  }
}

////////////////////////////////////////////////////////////////////////////////
bool FileDescriptor::valid() const { return this->descriptor != -1; }
