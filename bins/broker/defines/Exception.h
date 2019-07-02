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

#ifndef __UPMQ_EXCEPTION_H__
#define __UPMQ_EXCEPTION_H__

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include "Defines.h"

#include <unordered_map>

namespace upmq {
namespace broker {

class Exception : public std::runtime_error {
 public:
  Exception(const std::string& info,
            // set exception information
            const std::string& errDescription,
            // set error description, for example, strerror(errno)
            int err,
            // set error, for example, errno
            const std::string& file,
            // set file name
            int line)  // set line number
      ;

  Exception(const Exception&);

  Exception(Exception&&) noexcept;

  Exception& operator=(const Exception&);

  Exception& operator=(Exception&&) noexcept;

  const char* what() const noexcept override;

  std::string message() const;

  int error() const;

  ~Exception() noexcept override;

 private:
  int _error{0};
};
}  // namespace broker
}  // namespace upmq

#define EXCEPTION(_info, _errDescription, _err) upmq::broker::Exception(_info, _errDescription, _err, __FILE__, __LINE__)

#endif  // __UPMQ_EXCEPTION_H__
