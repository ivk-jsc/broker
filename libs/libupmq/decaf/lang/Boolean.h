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

#ifndef _DECAF_LANG_BOOLEAN_H_
#define _DECAF_LANG_BOOLEAN_H_

#include <decaf/lang/Comparable.h>
#include <decaf/lang/String.h>
#include <decaf/util/Config.h>
#include <string>

namespace decaf {
namespace lang {

class DECAF_API Boolean : public Comparable<Boolean>, public Comparable<bool> {
 private:
  // This objects boolean value
  bool value;

 public:
  /**
   * The Class object representing the primitive false boolean.
   */
  static const Boolean _FALSE;

  /**
   * The Class object representing the primitive type boolean.
   */
  static const Boolean _TRUE;

 public:
  /**
   * @param value - primitive boolean to wrap.
   */
  explicit Boolean(bool value_);

  /**
   * @param value - String value to convert to a boolean.
   */
  explicit Boolean(const String &value);

  ~Boolean() override = default;

  /**
   * @return the primitive boolean value of this object
   */
  bool booleanValue() const { return value; }

  /**
   * @return the string representation of this Booleans value.
   */
  std::string toString() const;

  /**
   * Compares this Boolean instance with another.
   * @param b - the Boolean instance to be compared
   * @return zero if this object represents the same boolean value as the
   * argument; a positive value if this object represents true and the
   * argument represents false; and a negative value if this object
   * represents false and the argument represents true
   */
  int compareTo(const Boolean &b) const override;

  /**
   * Compares equality between this object and the one passed.
   * @param value - the value to be compared to this one.
   * @return true if this object is equal to the one passed.
   */
  bool operator==(const Boolean &value) const override;

  /**
   * Compares this object to another and returns true if this object
   * is considered to be less than the one passed.  This
   * @param value - the value to be compared to this one.
   * @return true if this object is equal to the one passed.
   */
  bool operator<(const Boolean &value) const override;

  /**
   * @return true if the two Boolean Objects have the same value.
   */
  bool equals(const Boolean &b) const override { return this->value == b.value; }

  /**
   * Compares this Boolean instance with another.
   * @param b - the Boolean instance to be compared
   * @return zero if this object represents the same boolean value as the
   * argument; a positive value if this object represents true and the
   * argument represents false; and a negative value if this object
   * represents false and the argument represents true
   */
  int compareTo(const bool &b) const override;

  /**
   * Compares equality between this object and the one passed.
   * @param value - the value to be compared to this one.
   * @return true if this object is equal to the one passed.
   */
  bool operator==(const bool &value) const override;

  /**
   * Compares this object to another and returns true if this object
   * is considered to be less than the one passed.  This
   * @param value - the value to be compared to this one.
   * @return true if this object is equal to the one passed.
   */
  bool operator<(const bool &value) const override;

  /**
   * @return true if the two Boolean Objects have the same value.
   */
  bool equals(const bool &b) const override { return this->value == b; }

 public:
  /**
   * @param value
   *      The bool value to convert to a <code>Boolean</code> instance.
   *
   * @return a Boolean instance of the primitive boolean value
   */
  static Boolean valueOf(bool value);

  /**
   * @param value
   *      The std::string value to convert to a <code>Boolean</code> instance.
   *
   * @return a Boolean instance of the string value
   */
  static Boolean valueOf(const String &value);

  /**
   * Parses the String passed and extracts an bool.
   *
   * @param value
   *      The std::string value to parse
   * @return bool value
   */
  static bool parseBoolean(const String &value);

  /**
   * Converts the bool to a String representation.
   *
   * @param value The bool value to convert.
   *
   * @return std::string representation of the bool value passed.
   */
  static std::string toString(bool value);
};
}  // namespace lang
}  // namespace decaf

#endif /*_DECAF_LANG_BOOLEAN_H_*/
