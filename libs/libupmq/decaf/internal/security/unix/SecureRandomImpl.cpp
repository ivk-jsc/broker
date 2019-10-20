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

#include "SecureRandomImpl.h"

#include <decaf/lang/Exception.h>
#include <decaf/lang/exceptions/IllegalArgumentException.h>
#include <decaf/lang/exceptions/NullPointerException.h>
#include <decaf/lang/exceptions/RuntimeException.h>
#include <decaf/util/Random.h>
#include <memory>
#include <cstdio>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

using namespace decaf;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;
using namespace decaf::util;
using namespace decaf::security;
using namespace decaf::internal;
using namespace decaf::internal::security;

////////////////////////////////////////////////////////////////////////////////
namespace decaf {
namespace internal {
namespace security {

class SRNGData {
 public:
  SRNGData(const SRNGData &) = delete;
  SRNGData operator=(const SRNGData &) = delete;
  int randFile;
  std::unique_ptr<Random> random;

  SRNGData() : randFile(-1), random() {}
};
}  // namespace security
}  // namespace internal
}  // namespace decaf

////////////////////////////////////////////////////////////////////////////////
SecureRandomImpl::SecureRandomImpl() : config(new SRNGData()) {
  try {
    const char *files[] = {"/dev/urandom", "/dev/random"};
    int index = 0;

    do {
      // Attempt to find an OS source for secure random bytes.
      config->randFile = open(files[index++], O_RDONLY);
    } while (index < 2 && config->randFile == -1);

    // Defaults to the Decaf version.
    if (config->randFile == -1) {
      this->config->random.reset(new Random());
    }
  }
  DECAF_CATCH_RETHROW(Exception)
  DECAF_CATCHALL_THROW(Exception)
}

////////////////////////////////////////////////////////////////////////////////
SecureRandomImpl::~SecureRandomImpl() {
  try {
    delete this->config;
  }
  DECAF_CATCH_NOTHROW(Exception)
  DECAF_CATCHALL_NOTHROW()
}

////////////////////////////////////////////////////////////////////////////////
void SecureRandomImpl::providerSetSeed(const unsigned char *seed, int size) {
  // Only seed the default random, the other sources don't need a seed.
  if (this->config->random != nullptr) {
    for (int i = 0; i < size; i++) {
      this->config->random->setSeed((long long)seed[i]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void SecureRandomImpl::providerNextBytes(unsigned char *bytes, int numBytes) {
  if (bytes == nullptr) {
    throw NullPointerException(__FILE__, __LINE__, "Byte Buffer passed cannot be NULL.");
  }

  if (numBytes < 0) {
    throw IllegalArgumentException(__FILE__, __LINE__, "Number of bytes to read was negative: %d", numBytes);
  }

  if (this->config->randFile != -1) {
    size_t bytesRead = 0;

    // read it all.
    do {
      errno = 0;
      bytesRead += read(this->config->randFile, (void *)&bytes[bytesRead], numBytes - bytesRead);
    } while (bytesRead < static_cast<size_t>(numBytes) || errno != 0);

    // Since the dev random files are special OS random sources we should never get
    // an EOF or other error, if so its bad.
    if (bytesRead != static_cast<size_t>(numBytes)) {
      throw RuntimeException(__FILE__, __LINE__, "Unexpected error while reading random bytes from system resources.");
    }

  } else {
    this->config->random->nextBytes(bytes, numBytes);
  }
}

////////////////////////////////////////////////////////////////////////////////
unsigned char *SecureRandomImpl::providerGenerateSeed(int numBytes) {
  if (numBytes == 0) {
    return nullptr;
  }

  unsigned char *buffer = new unsigned char[numBytes];
  providerNextBytes(buffer, numBytes);
  return buffer;
}
