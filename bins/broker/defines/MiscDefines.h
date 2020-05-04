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

#ifndef BROKER_MISCDEFINES_H
#define BROKER_MISCDEFINES_H

#include <iostream>
#include "Exception.h"
#if POCO_VERSION_MAJOR > 1
#include <Poco/SQL/SQLite/SQLiteException.h>
namespace PDSQLITE = Poco::SQL::SQLite;
#else
#include <Poco/Data/SQLite/SQLiteException.h>
namespace PDSQLITE = Poco::Data::SQLite;
#endif

#ifdef _DEBUG
#define do_cerr(__info__, __sql__, __message__, __error__) \
  std::cerr << std::string(__info__) << " : " << (__sql__) << " : " << (__message__) << " : " << (__error__)
#else
#define do_cerr(__info__, __sql__, __message__, __error__) \
  UNUSED_VAR(__info__);                                    \
  UNUSED_VAR(__sql__);                                     \
  UNUSED_VAR(__message__);                                 \
  UNUSED_VAR(__error__)
#endif

#define TRY_POCO_DATA_EXCEPTION \
  {                             \
    bool locked;                \
    do {                        \
      locked = false;           \
      try

#define CATCH_POCO_DATA_EXCEPTION(__info__, __sql__, __expr__, __error__)                           \
  catch (PDSQLITE::DBLockedException & dblex) {                                                     \
    UNUSED_VAR(dblex);                                                                              \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::TableLockedException & tblex) {                                                  \
    UNUSED_VAR(tblex);                                                                              \
    Poco::Thread::yield();                                                                          \
    locked = true;                                                                                  \
  }                                                                                                 \
  catch (PDSQLITE::InvalidSQLStatementException & issex) {                                          \
    UNUSED_VAR(issex);                                                                              \
    Poco::Thread::yield();                                                                          \
    locked = true;                                                                                  \
  }                                                                                                 \
  catch (PDSQLITE::SQLiteException & sqlex) {                                                       \
    if (sqlex.message().find("locked") == std::string::npos) {                                      \
      __expr__;                                                                                     \
      throw EXCEPTION(std::string(__info__) + " : " + (__sql__), sqlex.message(), (__error__));     \
    }                                                                                               \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (Poco::LogicException & plex) {                                                             \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), plex.message(), (__error__));        \
  }                                                                                                 \
  catch (Poco::Data::ExecutionException & pdex) {                                                   \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pdex.message(), (__error__));        \
  }                                                                                                 \
  catch (Poco::Exception & pex) {                                                                   \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pex.message(), (__error__));         \
  }                                                                                                 \
  catch (const std::exception &e) {                                                                 \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), std::string(e.what()), (__error__)); \
  }                                                                                                 \
  }                                                                                                 \
  while (locked)                                                                                    \
    ;                                                                                               \
  }

#define CATCH_POCO_DATA_EXCEPTION_NO_INVALID_SQL(__info__, __sql__, __expr__, __error__)            \
  catch (PDSQLITE::DBLockedException & dblex) {                                                     \
    UNUSED_VAR(dblex);                                                                              \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::TableLockedException & tblex) {                                                  \
    UNUSED_VAR(tblex);                                                                              \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::SQLiteException & sqlex) {                                                       \
    if (sqlex.message().find("locked") == std::string::npos) {                                      \
      __expr__;                                                                                     \
      throw EXCEPTION(std::string(__info__) + " : " + (__sql__), sqlex.message(), (__error__));     \
    }                                                                                               \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (Poco::LogicException & plex) {                                                             \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), plex.message(), (__error__));        \
  }                                                                                                 \
  catch (Poco::Data::ExecutionException & pdex) {                                                   \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pdex.message(), (__error__));        \
  }                                                                                                 \
  catch (Poco::Exception & pex) {                                                                   \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pex.message(), (__error__));         \
  }                                                                                                 \
  catch (const std::exception &e) {                                                                 \
    __expr__;                                                                                       \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), std::string(e.what()), (__error__)); \
  }                                                                                                 \
  }                                                                                                 \
  while (locked)                                                                                    \
    ;                                                                                               \
  }

#define CATCH_POCO_DATA_EXCEPTION_PURE(__info__, __sql__, __error__)                                \
  catch (PDSQLITE::DBLockedException & dblex) {                                                     \
    UNUSED_VAR(dblex);                                                                              \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::TableLockedException & tblex) {                                                  \
    UNUSED_VAR(tblex);                                                                              \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::InvalidSQLStatementException & issex) {                                          \
    if (issex.message().find("no such table") != std::string::npos) {                               \
      throw EXCEPTION(std::string(__info__) + " : " + (__sql__), issex.message(), (__error__));     \
    }                                                                                               \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::SQLiteException & sqlex) {                                                       \
    if (sqlex.message().find("locked") == std::string::npos) {                                      \
      throw EXCEPTION(std::string(__info__) + " : " + (__sql__), sqlex.message(), (__error__));     \
    }                                                                                               \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (Poco::Data::ExecutionException & pdex) {                                                   \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pdex.message(), (__error__));        \
  }                                                                                                 \
  catch (Poco::Exception & pex) {                                                                   \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pex.message(), (__error__));         \
  }                                                                                                 \
  catch (const std::exception &e) {                                                                 \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), std::string(e.what()), (__error__)); \
  }                                                                                                 \
  }                                                                                                 \
  while (locked)                                                                                    \
    ;                                                                                               \
  }

#define CATCH_POCO_DATA_EXCEPTION_PURE_TROW_INVALID_SQL(__info__, __sql__, __error__)               \
  catch (PDSQLITE::DBLockedException & dblex) {                                                     \
    UNUSED_VAR(dblex);                                                                              \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::TableLockedException & tblex) {                                                  \
    UNUSED_VAR(tblex);                                                                              \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (PDSQLITE::InvalidSQLStatementException & issex) {                                          \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), issex.message(), (__error__));       \
  }                                                                                                 \
  catch (PDSQLITE::SQLiteException & sqlex) {                                                       \
    if (sqlex.message().find("locked") == std::string::npos) {                                      \
      throw EXCEPTION(std::string(__info__) + " : " + (__sql__), sqlex.message(), (__error__));     \
    }                                                                                               \
    locked = true;                                                                                  \
    Poco::Thread::yield();                                                                          \
  }                                                                                                 \
  catch (Poco::Data::ExecutionException & pdex) {                                                   \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pdex.message(), (__error__));        \
  }                                                                                                 \
  catch (Poco::Exception & pex) {                                                                   \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), pex.message(), (__error__));         \
  }                                                                                                 \
  catch (const std::exception &e) {                                                                 \
    throw EXCEPTION(std::string(__info__) + " : " + (__sql__), std::string(e.what()), (__error__)); \
  }                                                                                                 \
  }                                                                                                 \
  while (locked)                                                                                    \
    ;                                                                                               \
  }

#define CATCH_POCO_DATA_EXCEPTION_PURE_NO_EXCEPT(__info__, __sql__, __error__)                  \
  catch (PDSQLITE::DBLockedException & dblex) {                                                 \
    UNUSED_VAR(dblex);                                                                          \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (PDSQLITE::TableLockedException & tblex) {                                              \
    UNUSED_VAR(tblex);                                                                          \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (PDSQLITE::InvalidSQLStatementException & issex) {                                      \
    UNUSED_VAR(issex);                                                                          \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (PDSQLITE::SQLiteException & sqlex) {                                                   \
    if (sqlex.message().find("locked") == std::string::npos) {                                  \
      throw EXCEPTION(std::string(__info__) + " : " + (__sql__), sqlex.message(), (__error__)); \
    }                                                                                           \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (Poco::InvalidAccessException & invaccex) {                                             \
    UNUSED_VAR(invaccex);                                                                       \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (Poco::Data::ExecutionException & pdex) {                                               \
    do_cerr(__info__, __sql__, pdex.message(), __error__);                                      \
  }                                                                                             \
  catch (Poco::Exception & pex) {                                                               \
    do_cerr(__info__, __sql__, pex.message(), __error__);                                       \
  }                                                                                             \
  catch (const std::exception &e) {                                                             \
    do_cerr(__info__, __sql__, e.what(), __error__);                                            \
  }                                                                                             \
  }                                                                                             \
  while (locked)                                                                                \
    ;                                                                                           \
  }

#define CATCH_POCO_DATA_EXCEPTION_PURE_NO_INVALIDEXCEPT_NO_EXCEPT(__info__, __sql__, __error__) \
  catch (PDSQLITE::DBLockedException & dblex) {                                                 \
    UNUSED_VAR(dblex);                                                                          \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (PDSQLITE::TableLockedException & tblex) {                                              \
    UNUSED_VAR(tblex);                                                                          \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (PDSQLITE::InvalidSQLStatementException & issex) {                                      \
    do_cerr(__info__, __sql__, issex.message(), __error__);                                     \
  }                                                                                             \
  catch (PDSQLITE::SQLiteException & sqlex) {                                                   \
    if (sqlex.message().find("locked") == std::string::npos) {                                  \
      throw EXCEPTION(std::string(__info__) + " : " + (__sql__), sqlex.message(), (__error__)); \
    }                                                                                           \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (Poco::InvalidAccessException & invaccex) {                                             \
    UNUSED_VAR(invaccex);                                                                       \
    locked = true;                                                                              \
    Poco::Thread::yield();                                                                      \
  }                                                                                             \
  catch (Poco::Data::ExecutionException & pdex) {                                               \
    do_cerr(__info__, __sql__, pdex.message(), __error__);                                      \
  }                                                                                             \
  catch (Poco::Exception & pex) {                                                               \
    do_cerr(__info__, __sql__, pex.message(), __error__);                                       \
  }                                                                                             \
  catch (const std::exception &e) {                                                             \
    do_cerr(__info__, __sql__, e.what(), __error__);                                            \
  }                                                                                             \
  }                                                                                             \
  while (locked)                                                                                \
    ;                                                                                           \
  }
#endif  // BROKER_MISCDEFINES_H
