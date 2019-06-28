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

#ifndef _CMS_CMSEXCEPTION_H_
#define _CMS_CMSEXCEPTION_H_

// Includes
#include <cstddef>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <cms/Config.h>

template <typename T1, typename T2, typename T3>
struct triplet {
  triplet() {}
  triplet(T1 &&first, T2 &&second, T3 third) : first(std::forward<T1>(first)), second(std::forward<T2>(second)), third(third) {}
  triplet(const triplet &) = default;
  triplet(triplet &&) = default;
  triplet &operator=(const triplet &) = default;
  triplet &operator=(triplet &&) = default;
  ~triplet() = default;
  T1 first;
  T2 second;
  T3 third;
};
namespace tr_helper {
template <class _Ty>
struct _Unrefwrap_helper {  // leave unchanged if not a reference_wrapper
  using type = _Ty;
};

template <class _Ty>
struct _Unrefwrap_helper<std::reference_wrapper<_Ty>> {  // make a reference from a reference_wrapper
  using type = _Ty &;
};

// decay, then unwrap a reference_wrapper
template <class _Ty>
using _Unrefwrap_t = typename _Unrefwrap_helper<typename std::decay<_Ty>::type>::type;

}  // namespace tr_helper
template <class T1, class T2, class T3>
constexpr triplet<tr_helper::_Unrefwrap_t<T1>, tr_helper::_Unrefwrap_t<T2>, tr_helper::_Unrefwrap_t<T3>> make_triplet(T1 &&m1, T2 &&m2, T3 m3) {
  using _Mytriplet = triplet<tr_helper::_Unrefwrap_t<T1>, tr_helper::_Unrefwrap_t<T2>, tr_helper::_Unrefwrap_t<T3>>;
  return _Mytriplet(std::forward<T1>(m1), std::forward<T2>(m2), m3);
}

namespace cms {

class CMSExceptionData;

/**
 * CMS API Exception that is the base for all exceptions thrown from CMS
 * classes.
 * <p>
 * This class represents an error that has occurred in CMS, providers
 * can wrap provider specific exceptions in this class by setting the
 * cause to an instance of a provider specific exception provided it
 * can be cast to an std::exception.
 * <p>
 * Since the contained cause exception is of type std::exception and the
 * C++ exception class has no clone or copy method defined the contained
 * exception can only be owned by one instance of an CMSException.  To that
 * end the class hands off the exception to each successive copy or clone
 * so care must be taken when handling CMSException instances.
 *
 * @since 1.0
 */
class CMS_API CMSException : public std::exception {
 private:
  // The actual data that defines this exception.
  CMSExceptionData *data;

 public:
  CMSException();

  CMSException(const CMSException &ex) noexcept;

  CMSException(CMSException &&ex) noexcept;

  CMSException(const std::string &message);

  CMSException(const std::string &message, const std::exception *cause);

  CMSException(const std::string &message, const std::exception *cause, const std::vector<triplet<std::string, std::string, int>> &stackTrace);

  CMSException(const std::string &message, const std::exception *cause, const std::vector<std::pair<std::string, int>> &stackTrace);

  ~CMSException() noexcept override;

  /**
   * Overridden assignment operator.  We don't allow CMSExceptions to be assigned to one
   * another so this method is deleted.
   *
   * @param
   *      The CMSException to assign to this instance.
   */
  CMSException &operator=(const cms::CMSException &) = delete;

  CMSException &operator=(cms::CMSException &&) = delete;

  /**
   * Gets the cause of the error.
   *
   * @return string errors message
   */
  virtual std::string getMessage() const;

  /**
   * Gets the exception that caused this one to be thrown, this allows
   * for chaining of exceptions in the case of a method that throws only
   * a particular exception but wishes to allow for the real causal
   * exception to be passed only in case the caller knows about that
   * type of exception and wishes to respond to it.
   * @return a const pointer reference to the causal exception, if there
   * was no cause associated with this exception then NULL is returned.
   */
  virtual const std::exception *getCause() const;

  /**
   * Provides the stack trace for every point where
   * this exception was caught, marked, and rethrown.
   *
   * @return vector containing stack trace strings
   */
  virtual std::vector<triplet<std::string, std::string, int>> getStackTrace() const;

  /**
   * Adds a func/file/line number to the stack trace.
   * @param func The name of the calling method (use __FUNCTION__).
   * @param file The name of the file calling this method (use __FILE__).
   * @param lineNumber The line number in the calling file (use __LINE__).
   */
  virtual void setMark(const char *func, const char *file, const int lineNumber);

  /**
   * Prints the stack trace to std::err
   */
  virtual void printStackTrace() const;

  /**
   * Prints the stack trace to the given output stream.
   *
   * @param stream the target output stream.
   */
  virtual void printStackTrace(std::ostream &stream) const;

  /**
   * Gets the stack trace as one contiguous string.
   *
   * @return string with formatted stack trace data
   */
  virtual std::string getStackTraceString() const;

  /**
   * Overloads the std::exception what() function to return the cause of the exception
   *
   * @return const char pointer to error message
   */
  const char *what() const noexcept override;

  /**
   * Creates a cloned version of this CMSException instance.
   *
   * This method passes on ownership of the contained cause exception pointer to
   * the clone.  This method is mainly useful to the CMS provider.
   *
   * @return new pointer that is a clone of this Exception, caller owns.
   */
  virtual CMSException *clone();
};
}  // namespace cms

#endif /*_CMS_CMSEXCEPTION_H_*/
