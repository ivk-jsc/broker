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

#ifndef _UPMQ_CPP_H_
#define _UPMQ_CPP_H_

#include <transport/Config.h>

namespace upmq {
namespace transport {

class UPMQCPP_API UPMQCPP {
 protected:
  UPMQCPP();

 public:
  UPMQCPP(const UPMQCPP &) = delete;
  UPMQCPP &operator=(const UPMQCPP &) = delete;

  virtual ~UPMQCPP();

  /**
   * Initialize the UPMQ-CPP Library constructs, this method will
   * init all the internal Registry objects and initialize the Decaf
   * library.
   *
   * @throws runtime_error if an error occurs while initializing this library.
   */
  static void initializeLibrary();

  /**
   * Initialize the UPMQ-CPP Library constructs, this method will
   * initialize all the internal Registry objects and initialize the Decaf
   * library.  This method takes the args passed to the main method of
   * process for use is setting system properties and configuring the
   * UPMQ-CPP Library.
   *
   * @param argc - the count of arguments passed to this Process.
   * @param argv - the array of string arguments passed to this process.
   *
   * @throws runtime_error if an error occurs while initializing this library.
   */
  static void initializeLibrary(int argc, char **argv);

  /**
   * Shutdown the UPMQ-CPP Library, freeing any resources
   * that could not be freed up to this point.  All the user created
   * UPMQ-CPP objects and Decaf Library objects should have been
   * destroyed by the time this method is called.
   */
  static void shutdownLibrary();

 private:
  static void registerTransports();
};
}  // namespace transport
}  // namespace upmq

#endif /* _UPMQ_CPP_H_ */
