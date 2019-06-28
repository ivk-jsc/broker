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

#include "SecureRandom.h"

#include <decaf/lang/exceptions/IllegalArgumentException.h>
#include <decaf/lang/exceptions/NullPointerException.h>

#ifdef HAVE_PTHREAD_H
#include <decaf/internal/security/unix/SecureRandomImpl.h>
#else
#include <decaf/internal/security/windows/SecureRandomImpl.h>
#endif

using namespace decaf;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::security;
using namespace decaf::internal;
using namespace decaf::internal::security;

////////////////////////////////////////////////////////////////////////////////
SecureRandom::SecureRandom() : secureRandom(new SecureRandomImpl()) {}

////////////////////////////////////////////////////////////////////////////////
SecureRandom::SecureRandom(const std::vector<unsigned char> &seed) : secureRandom(new SecureRandomImpl()) {
  if (!seed.empty()) {
    this->secureRandom->providerSetSeed(&seed[0], (int)seed.size());
  }
}

////////////////////////////////////////////////////////////////////////////////
SecureRandom::SecureRandom(const unsigned char *seed, int size) : secureRandom(new SecureRandomImpl()) {
  if (seed == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Seed buffer pointer passed was NULL");
  }

  if (size < 0) {
    throw IllegalArgumentException(__FILE__, __LINE__, "Passed buffer size was negative.");
  }

  if (size > 0) {
    this->secureRandom->providerSetSeed(seed, size);
  }
}

////////////////////////////////////////////////////////////////////////////////
SecureRandom::~SecureRandom() {}

////////////////////////////////////////////////////////////////////////////////
void SecureRandom::nextBytes(std::vector<unsigned char> &buf) {
  if (!buf.empty()) {
    this->secureRandom->providerNextBytes(&buf[0], (int)buf.size());
  }
}

////////////////////////////////////////////////////////////////////////////////
void SecureRandom::nextBytes(unsigned char *buf, int size) {
  if (buf == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Buffer pointer passed was NULL");
  }

  if (size < 0) {
    throw IllegalArgumentException(__FILE__, __LINE__, "Passed buffer size was negative.");
  }

  if (size > 0) {
    this->secureRandom->providerNextBytes(buf, size);
  }
}

////////////////////////////////////////////////////////////////////////////////
void SecureRandom::setSeed(unsigned long long newSeed) {
  if (newSeed == 0) {
    return;
  }

  unsigned char byteSeed[] = {static_cast<unsigned char>((newSeed >> 56) & 0xFF),
                              static_cast<unsigned char>((newSeed >> 48) & 0xFF),
                              static_cast<unsigned char>((newSeed >> 40) & 0xFF),
                              static_cast<unsigned char>((newSeed >> 32) & 0xFF),
                              static_cast<unsigned char>((newSeed >> 24) & 0xFF),
                              static_cast<unsigned char>((newSeed >> 16) & 0xFF),
                              static_cast<unsigned char>((newSeed >> 8) & 0xFF),
                              static_cast<unsigned char>((newSeed)&0xFF)};

  this->secureRandom->providerSetSeed(byteSeed, 8);
}

////////////////////////////////////////////////////////////////////////////////
void SecureRandom::setSeed(const std::vector<unsigned char> &newSeed) {
  if (!newSeed.empty()) {
    this->secureRandom->providerSetSeed(&newSeed[0], static_cast<int>(newSeed.size()));
  }
}

////////////////////////////////////////////////////////////////////////////////
void SecureRandom::setSeed(const unsigned char *newSeed, int size) {
  if (newSeed == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Buffer pointer passed was NULL");
  }

  if (size < 0) {
    throw IllegalArgumentException(__FILE__, __LINE__, "Passed buffer size was negative.");
  }

  if (size > 0) {
    this->secureRandom->providerSetSeed(newSeed, size);
  }
}

////////////////////////////////////////////////////////////////////////////////
int SecureRandom::next(int numBits) {
  if (numBits < 0) {
    numBits = 0;
  } else if (numBits > 32) {
    numBits = 32;
  }

  int bytes = (numBits + 7) / 8;
  unsigned char *next = new unsigned char[bytes];
  unsigned int ret = 0;

  this->nextBytes(next, bytes);
  for (int i = 0; i < bytes; i++) {
    ret = (next[i] & 0xFF) | (ret << 8);
  }

  ret = ret >> ((bytes * 8) - numBits);  //-V610

  delete[] next;

  return ret;
}
