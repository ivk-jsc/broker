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

#ifndef _DECAF_IO_EOFEXCEPTION_H_
#define _DECAF_IO_EOFEXCEPTION_H_

#include <decaf/io/IOException.h>

namespace decaf {
namespace io {

/*
 * Signals that an End of File exception has occurred.
 */
class DECAF_API EOFException : public io::IOException {
 public:
  /**
   * Default Constructor
   */
  EOFException();

  /**
   * Copy Constructor
   *
   * @param ex the exception to copy
   */
  EOFException(const lang::Exception &ex) noexcept;

  /**
   * Copy Constructor
   *
   * @param ex the exception to copy, which is an instance of this type
   */
  EOFException(const EOFException &ex) noexcept;

  /**
   * Move Constructor
   *
   * @param ex the exception to move, which is an instance of this type
   */
  EOFException(EOFException &&ex) noexcept;
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
  EOFException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...);

  /**
   * Constructor
   * @param cause Pointer to the exception that caused this one to
   * be thrown, the object is cloned caller retains ownership.
   */
  EOFException(const std::exception *cause);

  /**
   * Constructor
   *
   * @param file The file name where exception occurs
   * @param lineNumber The line number where the exception occurred.
   * @param msg The message to report
   * @param ... list of primitives that are formatted into the message
   */
  EOFException(const char *file, const int lineNumber, const char *msg, ...);

  /**
   * Clones this exception.  This is useful for cases where you need
   * to preserve the type of the original exception as well as the message.
   * All subclasses should override.
   *
   * @return a new instance of an Exception that is a copy of this one.
   */
  EOFException *clone() const override;

  ~EOFException() noexcept override;
};
}  // namespace io
}  // namespace decaf

#endif /*_DECAF_IO_EOFEXCEPTION_H_*/
