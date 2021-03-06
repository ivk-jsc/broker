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

#include "FilterOutputStream.h"

#include <decaf/io/OutputStream.h>
#include <decaf/lang/exceptions/NullPointerException.h>

#include <typeinfo>

using namespace decaf;
using namespace decaf::io;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
FilterOutputStream::FilterOutputStream(OutputStream *outputStream_, bool own_)
    : OutputStream(), outputStream(outputStream_), own(own_), closed(outputStream == nullptr) {}

////////////////////////////////////////////////////////////////////////////////
FilterOutputStream::~FilterOutputStream() {
  try {
    this->close();
  }
  DECAF_CATCHALL_NOTHROW()

  try {
    if (own) {
      delete outputStream;
    }
    outputStream = nullptr;
  }
  DECAF_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void FilterOutputStream::doWriteByte(unsigned char c) {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "FilterOutputStream::write - Stream is closed");
    }

    this->outputStream->write(c);
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FilterOutputStream::doWriteArray(const unsigned char *pbuffer, int size) {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "FilterOutputStream::write - Stream is closed");
    }

    this->doWriteArrayBounded(pbuffer, size, 0, size);
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCH_RETHROW(NullPointerException)
  DECAF_CATCH_RETHROW(IndexOutOfBoundsException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FilterOutputStream::doWriteArrayBounded(const unsigned char *pbuffer, int size, int offset, int length) {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "FilterOutputStream::write - Stream is closed");
    }

    if (pbuffer == nullptr) {
      throw decaf::lang::exceptions::NullPointerException(__FILE__, __LINE__, "FilterOutputStream::write - Buffer passed is Null.");
    }

    if (size < 0) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "size parameter out of Bounds: %d.", size);
    }

    if (offset > size || offset < 0) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "offset parameter out of Bounds: %d.", offset);
    }

    if (length < 0 || length > size - offset) {
      throw IndexOutOfBoundsException(__FILE__, __LINE__, "length parameter out of Bounds: %d.", length);
    }

    // Calls the doWriteByte method since subclasses may over override that method.
    for (int ix = offset; ix < offset + length; ++ix) {
      this->doWriteByte(pbuffer[ix]);
    }
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCH_RETHROW(NullPointerException)
  DECAF_CATCH_RETHROW(IndexOutOfBoundsException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FilterOutputStream::flush() {
  try {
    if (isClosed()) {
      throw IOException(__FILE__, __LINE__, "FilterOutputStream::flush - Stream is closed");
    }

    this->outputStream->flush();
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
void FilterOutputStream::close() {
  try {
    if (!this->closed && this->outputStream != nullptr) {
      this->outputStream->flush();
      this->outputStream->close();
    }
    this->closed = true;
  }
  DECAF_CATCH_RETHROW(IOException)
  DECAF_CATCHALL_THROW(IOException)
}

////////////////////////////////////////////////////////////////////////////////
std::string FilterOutputStream::toString() const {
  if (this->outputStream != nullptr) {
    return this->outputStream->toString();
  }

  return typeid(this).name();
}

////////////////////////////////////////////////////////////////////////////////
bool FilterOutputStream::isClosed() const { return this->closed || this->outputStream == nullptr; }
