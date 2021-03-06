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

#ifndef _DECAF_IO_UTFDATAFORMATEXCEPTION_H_
#define _DECAF_IO_UTFDATAFORMATEXCEPTION_H_

#include <decaf/io/IOException.h>

namespace decaf {
namespace io {

/**
 * Thrown from classes that attempt to read or write a UTF-8 encoded string
 * and an encoding error is encountered.
 *
 * @since 1.0
 */
class DECAF_API UTFDataFormatException : public decaf::io::IOException {
 public:
  /**
   * Default Constructor
   */
  UTFDataFormatException();

  /**
   * Copy Constructor
   *
   * @param ex the exception to copy
   */
  UTFDataFormatException(const lang::Exception &ex) noexcept;

  /**
   * Copy Constructor
   *
   * @param ex the exception to copy, which is an instance of this type
   */
  UTFDataFormatException(const UTFDataFormatException &ex) noexcept;

  /**
   * Move Constructor
   *
   * @param ex the exception to move, which is an instance of this type
   */
  UTFDataFormatException(UTFDataFormatException &&ex) noexcept;

  /**
   * Constructor - Initializes the file name and line number where
   * this message occurred.  Sets the message to report, using an
   * optional list of arguments to parse into the message
   *
   * @param file The file name where exception occurs
   * @param lineNumber The line number where the exception occurred.
   * @param cause The exception that was the cause for this one to be thrown.
   * @param msg The message to report
   * @param ... list of primitives that are formatted into the message
   */
  UTFDataFormatException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...);

  /**
   * Constructor
   *
   * @param cause Pointer to the exception that caused this one to
   * be thrown, the object is cloned caller retains ownership.
   */
  UTFDataFormatException(const std::exception *cause);

  /**
   * Constructor
   *
   * @param file The file name where exception occurs
   * @param lineNumber The line number where the exception occurred.
   * @param msg The message to report
   * @param ... list of primitives that are formatted into the message
   */
  UTFDataFormatException(const char *file, const int lineNumber, const char *msg, ...);

  /**
   * Clones this exception.  This is useful for cases where you need
   * to preserve the type of the original exception as well as the message.
   * All subclasses should override.
   *
   * @return A new instance of an Exception object that is a copy of this instance.
   */
  UTFDataFormatException *clone() const override;

  ~UTFDataFormatException() noexcept override;
};
}  // namespace io
}  // namespace decaf

#endif /* _DECAF_IO_UTFDATAFORMATEXCEPTION_H_ */
