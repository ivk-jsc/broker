find_package(Poco QUIET REQUIRED
             Foundation
             Util
             Net
             XML
             JSON)

if (NOT Poco_FOUND)
  message(STATUS "Try to set POCO_ROOT_DIR, for example cmake -DPOCO_ROOT_DIR=/usr/local")
endif ()

set(Poco_ALIASES
    Poco::Foundation
    Poco::Util
    Poco::Net
    Poco::XML
    Poco::JSON)

if (Poco_VERSION_MAJOR GREATER 1)
  find_package(Poco QUIET REQUIRED
               SQL
               SQLSQLite)
  list(APPEND Poco_ALIASES Poco::SQL Poco::SQLSQLite)
  if (ENABLE_POSTGRESQL)
    find_package(Poco QUIET REQUIRED SQLPostgreSQL)
    list(APPEND Poco_ALIASES Poco::SQLPostgreSQL)
  endif ()
  if (ENABLE_ODBC)
    find_package(Poco QUIET REQUIRED SQLODBC)
    list(APPEND Poco_ALIASES Poco::SQLODBC)
  endif ()
else ()
  find_package(Poco QUIET REQUIRED
               Data
               DataSQLite)
  list(APPEND Poco_ALIASES Poco::Data Poco::DataSQLite)
  if (ENABLE_POSTGRESQL)
    find_package(Poco QUIET REQUIRED DataPostgreSQL)
    list(APPEND Poco_ALIASES Poco::DataPostgreSQL)
  endif ()
  if (ENABLE_ODBC)
    find_package(Poco QUIET REQUIRED DataODBC)
    list(APPEND Poco_ALIASES Poco::DataODBC)
  endif ()
endif ()

if (Poco_FOUND)
  set_target_properties(${Poco_ALIASES}
                        PROPERTIES
                        IMPORTED_GLOBAL TRUE
                        Poco_VERSION_MAJOR ${Poco_VERSION_MAJOR})
endif ()