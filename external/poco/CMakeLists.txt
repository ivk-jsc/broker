set(Poco_VERSION_MAJOR 0)
set(Poco_VERSION_MINOR 0)
set(Poco_VERSION_PATCH 0)

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

macro(hexchar2dec VAR VAL)
  if (${VAL} MATCHES "[0-9]")
    set(${VAR} ${VAL})
  elseif (${VAL} MATCHES "[aA]")
    set(${VAR} 10)
  elseif (${VAL} MATCHES "[bB]")
    set(${VAR} 11)
  elseif (${VAL} MATCHES "[cC]")
    set(${VAR} 12)
  elseif (${VAL} MATCHES "[dD]")
    set(${VAR} 13)
  elseif (${VAL} MATCHES "[eE]")
    set(${VAR} 14)
  elseif (${VAL} MATCHES "[fF]")
    set(${VAR} 15)
  else ()
    message(FATAL_ERROR "Invalid format for hexidecimal character")
  endif ()
endmacro(hexchar2dec)

get_target_property(POCO_INCLUDE_DIRS Poco::Foundation INTERFACE_INCLUDE_DIRECTORIES)

list(GET POCO_INCLUDE_DIRS 0 POCO_INCLUDE_DIR)

if (EXISTS ${POCO_INCLUDE_DIR})
  if (EXISTS ${POCO_INCLUDE_DIR}/Poco/Version.h)
    set(VERSION_FILE ${POCO_INCLUDE_DIR}/Poco/Version.h)
  else ()
    set(VERSION_FILE ${POCO_INCLUDE_DIR}/Poco/Foundation.h)
  endif ()
  # regex quantifiers like {8} don't seem to work so we'll stick with + even though
  # it's not strictly true
  set(VERS_REGEX "^#define[ \t]+POCO_VERSION[ \t]+0x([0-9A-F]+)$")
  file(STRINGS ${VERSION_FILE} POCO_VERSION REGEX ${VERS_REGEX})

  # pull out just the part after the 0x
  string(REGEX REPLACE ${VERS_REGEX} "\\1" POCO_VERSION ${POCO_VERSION})

  # Pretty format
  string(SUBSTRING ${POCO_VERSION} 0 2 Poco_VERSION_MAJOR)
  string(REGEX REPLACE "^0\(.\)" "\\1" Poco_VERSION_MAJOR ${Poco_VERSION_MAJOR})
  string(SUBSTRING ${POCO_VERSION} 2 2 Poco_VERSION_MINOR)
  string(REGEX REPLACE "^0\(.\)" "\\1" Poco_VERSION_MINOR ${Poco_VERSION_MINOR})
  string(SUBSTRING ${POCO_VERSION} 4 2 Poco_VERSION_PATCH)
  string(REGEX REPLACE "^0\(.\)" "\\1" Poco_VERSION_PATCH ${Poco_VERSION_PATCH})
  hexchar2dec(Poco_VERSION_MINOR ${Poco_VERSION_MINOR})
  set(POCO_VERSION "${Poco_VERSION_MAJOR}.${Poco_VERSION_MINOR}.${Poco_VERSION_PATCH}")
endif ()

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
                        Poco_VERSION_MAJOR ${Poco_VERSION_MAJOR}
                        Poco_VERSION_MINOR ${Poco_VERSION_MINOR}
                        Poco_VERSION_PATCH ${Poco_VERSION_PATCH}
                        )
endif ()