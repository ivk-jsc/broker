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

#include <decaf/lang/System.h>
#include <decaf/lang/exceptions/UnsupportedOperationException.h>
#include <decaf/util/Config.h>
#include <decaf/util/Date.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTime.h>

using namespace std;
using namespace decaf;
using namespace decaf::util;
using namespace decaf::lang;
using namespace decaf::lang::exceptions;

////////////////////////////////////////////////////////////////////////////////
Date::Date() : time(System::currentTimeMillis()) {}

////////////////////////////////////////////////////////////////////////////////
Date::Date(long long milliseconds) : time(milliseconds) {}

////////////////////////////////////////////////////////////////////////////////
Date::Date(const Date &source) : time(0) { (*this) = source; }

////////////////////////////////////////////////////////////////////////////////
Date::~Date() {}

////////////////////////////////////////////////////////////////////////////////
long long Date::getTime() const { return time; }

////////////////////////////////////////////////////////////////////////////////
void Date::setTime(long long milliseconds) { this->time = milliseconds; }

////////////////////////////////////////////////////////////////////////////////
bool Date::after(const Date &when) const { return time > when.time; }

////////////////////////////////////////////////////////////////////////////////
bool Date::before(const Date &when) const { return time < when.time; }

////////////////////////////////////////////////////////////////////////////////
Date &Date::operator=(const Date &source) {
  this->time = source.time;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::equals(const Date &when) const { return time == when.time; }

////////////////////////////////////////////////////////////////////////////////
int Date::compareTo(const Date &value) const {
  if (this->time < value.time) {
    return -1;
  } else if (this->time > value.time) {
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator==(const Date &value) const { return (this->time == value.time); }

////////////////////////////////////////////////////////////////////////////////
bool Date::operator<(const Date &value) const { return (this->time < value.time); }

////////////////////////////////////////////////////////////////////////////////
std::string Date::toString() const {
  Poco::Timestamp timestamp(time * 1000);

  // dow mon dd hh:mm:ss zzz yyyy
  static std::string format = "%a %b %d %T %Z %Y";

  return Poco::DateTimeFormatter::format(timestamp, format);
}

////////////////////////////////////////////////////////////////////////////////
std::string Date::toISOString() const {
  Poco::Timestamp timestamp(time * 1000);
  return Poco::DateTimeFormatter::format(timestamp, Poco::DateTimeFormat::ISO8601_FRAC_FORMAT);
}
