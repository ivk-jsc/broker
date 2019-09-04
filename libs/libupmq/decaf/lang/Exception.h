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

#ifndef _DECAF_LANG_EXCEPTION_EXCEPTION_H_
#define _DECAF_LANG_EXCEPTION_EXCEPTION_H_

#ifdef _WIN32
#pragma warning(disable : 4251)
#endif

#include <decaf/lang/Throwable.h>
#include <decaf/lang/exceptions/ExceptionDefines.h>
#include <decaf/util/Config.h>
#include <sstream>
#include <cstdarg>

namespace decaf {
namespace lang {

class ExceptionData;

/*
 * Base class for all exceptions.
 */
class DECAF_API Exception : public Throwable {  //-V690
 protected:
  ExceptionData *data;

 public:
  /**
   * Default Constructor
   */
  Exception();

  /**
   * Copy Constructor
   *
   * @param ex
   *      The <code>Exception</code> instance to copy.
   */
  Exception(const Exception &ex) noexcept;

  /**
   * Move Constructor
   *
   * @param ex
   *      The <code>Exception</code> instance to move.
   */
  Exception(Exception &&ex) noexcept;

  /**
   * Constructor
   *
   * @param cause
   *      Pointer to the exception that caused this one to
   *      be thrown, the caller must ensure that it passes a
   *      valid pointer as this object takes ownership of the
   *      exception.
   */
  Exception(const std::exception *cause);

  /**
   * Constructor - Initializes the file name and line number where
   * this message occurred.  Sets the message to report, using an
   * optional list of arguments to parse into the message.
   *
   * @param file
   *      The file name where exception occurs
   * @param lineNumber
   *      The line number where the exception occurred.
   * @param msg
   *      The message to report
   * @param ...
   *      list of primitives that are formatted into the message
   */
  Exception(const char *file, const int lineNumber, const char *msg, ...);

  /**
   * Constructor - Initializes the file name and line number where
   * this message occurred.  Sets the message to report, using an
   * optional list of arguments to parse into the message.
   *
   * @param file
   *      The file name where exception occurs
   * @param lineNumber
   *      The line number where the exception occurred.
   * @param cause
   *      The exception that was the cause for this one to be thrown.
   * @param msg
   *      The message to report
   * @param ...
   *      list of primitives that are formatted into the message
   */
  Exception(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...);

  ~Exception() noexcept override;

  /**
   * Gets the message for this exception.
   * @return Text formatted error message
   */
  std::string getMessage() const override;

  /**
   * Gets the exception that caused this one to be thrown, this allows
   * for chaining of exceptions in the case of a method that throws only
   * a particular exception but wishes to allow for the real causal
   * exception to be passed only in case the caller knows about that
   * type of exception and wishes to respond to it.
   *
   * @return a const pointer reference to the causal exception, if there
   *          was no cause associated with this exception then NULL is returned.
   */
  const std::exception *getCause() const override;

  /**
   * Initializes the contained cause exception with the one given.  The caller should
   * ensure that a valid copy of the causal exception is passed as this Exception object
   * will take ownership of the passed pointer.  Do not pass a pointer to the address of
   * an exception allocated on the stack or from an exception in a catch block.
   *
   * @param cause
   *      The exception that was the cause of this one.
   */
  void initCause(const std::exception *cause) override;

  /**
   * Implement method from std::exception.
   *
   * @return the const char* of <code>getMessage()</code>.
   */
  const char *what() const noexcept override;

  /**
   * Sets the cause for this exception.
   *
   * @param msg
   *      The format string for the msg.
   * @param ...
   *      The params to format into the string.
   */
  virtual void setMessage(const char *msg, ...);

  /**
   * Adds a file/line number to the stack trace.
   *
   * @param file
   *      The name of the file calling this method (use __FILE__).
   * @param lineNumber
   *      The line number in the calling file (use __LINE__).
   */
  void setMark(const char *file, const int lineNumber) override;

  /**
   * Clones this exception.  This is useful for cases where you need
   * to preserve the type of the original exception as well as the message.
   * All subclasses should override.
   *
   * @return Copy of this Exception object
   */
  Exception *clone() const override;

  /**
   * Provides the stack trace for every point where this exception was caught,
   * marked, and rethrown.  The first item in the returned vector is the first
   * point where the mark was set (e.g. where the exception was created).
   *
   * @return the stack trace.
   */
  std::vector<std::pair<std::string, int> > getStackTrace() const override;

  /**
   * Prints the stack trace to std::err
   */
  void printStackTrace() const override;

  /**
   * Prints the stack trace to the given output stream.
   *
   * @param stream
   *      the target output stream.
   */
  void printStackTrace(std::ostream &stream) const override;

  /**
   * Gets the stack trace as one contiguous string.
   *
   * @return string with formatted stack trace data.
   */
  std::string getStackTraceString() const override;

  /**
   * Assignment operator, copies one Exception to another.
   *
   * @param ex
   *      const reference to another Exception
   */
  Exception &operator=(const Exception &ex);

  /**
   * Assignment operator, move one Exception to another.
   *
   * @param ex
   *      rvalue reference to another Exception
   */
  Exception &operator=(Exception &&ex) noexcept;

 protected:
  virtual void setStackTrace(const std::vector<std::pair<std::string, int> > &trace);

  virtual void buildMessage(const char *format, va_list &vargs);

 private:
  ExceptionData *releaseData();
};
}  // namespace lang
}  // namespace decaf

#define DECAF_DECLARE_EXCEPTION(API, CLS, BASE)                                               \
  class API CLS : public BASE {                                                               \
   public:                                                                                    \
    CLS();                                                                                    \
    CLS(const BASE &exc) noexcept;                                                            \
    CLS(const CLS &exc) noexcept;                                                             \
    CLS(CLS &&exc) noexcept;                                                                  \
    CLS(const std::exception *cause);                                                         \
    CLS(const char *file, int lineNumber, const char *msg, ...);                              \
    CLS(const char *file, int lineNumber, const std::exception *cause, const char *msg, ...); \
    ~CLS() noexcept override;                                                                 \
    CLS &operator=(const CLS &exc);                                                           \
    CLS &operator=(CLS &&exc) noexcept;                                                       \
    CLS *clone() const override;                                                              \
  };

#define DECAF_IMPLEMENT_EXCEPTION(CLS, BASE)                                                                    \
  CLS::CLS() = default;                                                                                         \
  CLS::CLS(const BASE &exc) noexcept : BASE(exc) {}                                                             \
  CLS::CLS(const CLS &exc) noexcept = default;                                                                  \
  CLS::CLS(CLS &&exc) noexcept = default;                                                                       \
  CLS::CLS(const std::exception *cause) : BASE(cause) {}                                                        \
  CLS::CLS(const char *file, int lineNumber, const char *msg, ...) {                                            \
    va_list vargs;                                                                                              \
    va_start(vargs, msg);                                                                                       \
    decaf::lang::Exception::buildMessage(msg, vargs);                                                           \
    va_end(vargs);                                                                                              \
    decaf::lang::Exception::setMark(file, lineNumber);                                                          \
  }                                                                                                             \
  CLS::CLS(const char *file, int lineNumber, const std::exception *cause, const char *msg, ...) : BASE(cause) { \
    va_list vargs;                                                                                              \
    va_start(vargs, msg);                                                                                       \
    decaf::lang::Exception::buildMessage(msg, vargs);                                                           \
    va_end(vargs);                                                                                              \
    decaf::lang::Exception::setMark(file, lineNumber);                                                          \
  }                                                                                                             \
  CLS::~CLS() noexcept = default;                                                                               \
  CLS &CLS::operator=(const CLS &exc) {                                                                         \
    BASE::operator=(exc);                                                                                       \
    return *this;                                                                                               \
  }                                                                                                             \
  CLS &CLS::operator=(CLS &&exc) noexcept {                                                                     \
    BASE::operator=(std::forward<BASE>(exc));                                                                   \
    return *this;                                                                                               \
  }                                                                                                             \
  CLS *CLS::clone() const { return new CLS(*this); }

#endif /*_DECAF_LANG_EXCEPTION_EXCEPTION_H_*/
