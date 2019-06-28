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

#ifndef UPMQ_SYS_REGEX_H
#define UPMQ_SYS_REGEX_H

#include <regex>
#include <stdexcept>
#include <string>

#if defined(_POSIX_SOURCE) || defined(__unix__) || defined(__MACH__)
#include <regex.h>
#elif defined(_MSC_VER)
#else
#error "No known regex implementation"
#endif

// This is a very simple wrapper for Basic Regular Expression facilities either
// provided by POSIX or C++ tr1
//
// Matching expressions are not supported and so the only useful operation is
// a simple boolean match indicator.

namespace upmq {
namespace broker {

#if defined(_POSIX_SOURCE) || defined(__unix__) || defined(__MACH__)

class SelectorRegex {
  ::regex_t re{};

 public:
  explicit SelectorRegex(const std::string &s) {
    int rc = ::regcomp(&re, s.c_str(), REG_NOSUB);
    if (rc != 0) {
      throw std::logic_error("Regular expression compilation error");
    }
  }

  ~SelectorRegex() { ::regfree(&re); }

  friend bool regex_match(const std::string &s, const SelectorRegex &re);
};

inline bool regex_match(const std::string &s, const SelectorRegex &re) { return ::regexec(&(re.re), s.c_str(), 0, nullptr, 0) == 0; }

#elif defined(_MSC_VER)
#if _MSC_VER >= 1914
#define std_tr std
#else
#define std_tr std::tr1
#endif
class SelectorRegex : public std_tr::regex {
 public:
  explicit SelectorRegex(const std::string &s) : std_tr::regex(s, std_tr::regex::basic) {}
};

using std_tr::regex_match;

#else
#error "No known regex implementation"
#endif
}  // namespace broker
}  // namespace upmq

#endif
