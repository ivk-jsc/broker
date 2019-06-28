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

#ifndef _UPMQ_EXCEPTIONS_EXCEPTIONDEFINES_H_
#define _UPMQ_EXCEPTIONS_EXCEPTIONDEFINES_H_

/**
 * Macro for catching and re-throwing an exception of
 * a given type.
 * @param type The type of the exception to throw
 * (e.g. UPMQException ).
 */
#define AMQ_CATCH_RETHROW(type)     \
  catch (type & ex) {               \
    ex.setMark(__FILE__, __LINE__); \
    throw;                          \
  }

/**
 * Macro for catching an exception of one type and then re-throwing
 * as another type.
 * @param sourceType the type of the exception to be caught.
 * @param targetType the type of the exception to be thrown.
 */
#define AMQ_CATCH_EXCEPTION_CONVERT(sourceType, targetType) \
  catch (sourceType & ex) {                                 \
    targetType target(ex);                                  \
    target.setMark(__FILE__, __LINE__);                     \
    throw targetType(std::move(target));                    \
  }

/**
 * A catch-all that throws a known exception.
 * @param type the type of exception to be thrown.
 */
#define AMQ_CATCHALL_THROW(type)                                \
  catch (...) {                                                 \
    throw type(__FILE__, __LINE__, "caught unknown exception"); \
  }

/**
 * A catch-all that does not throw an exception, one use would
 * be to catch any exception in a destructor and mark it, but not
 * throw so that cleanup would continue as normal.
 */
#define AMQ_CATCHALL_NOTHROW()                                                                         \
  catch (...) {                                                                                        \
    upmq::transport::UPMQException ex(__FILE__, __LINE__, "caught unknown exception, not rethrowing"); \
  }

/**
 * Macro for catching and re-throwing an exception of
 * a given type.
 * @param type The type of the exception to throw
 * (e.g. UPMQException ).
 */
#define AMQ_CATCH_NOTHROW(type)     \
  catch (type & ex) {               \
    ex.setMark(__FILE__, __LINE__); \
  }

#endif /*_UPMQ_EXCEPTIONS_EXCEPTIONDEFINES_H_*/
