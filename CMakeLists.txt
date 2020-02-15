cmake_minimum_required(VERSION 3.12...3.13)

project(upmq
        VERSION 3.5
        DESCRIPTION "C++ MQ-Broker")


option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_EXAMPLES "Enable examples" ON)
option(ENABLE_WEB_ADMIN "Enable web-admin gui for broker" OFF)
option(ENABLE_POSTGRESQL "Enable PostgreSQL support (usefull only for POCO greater or equal  1.10)" OFF)
option(ENABLE_ODBC "Enable ODBC support" OFF)
option(ENABLE_USING_SENDFILE "Enable using SendFile for file transfering (not applyable with cygwin)" ON)
option(MAKE_DISTR_BY_CPACK "Make distribuive with cpack" OFF)

if (POCO_ROOT_DIR)
  list(APPEND CMAKE_MODULE_PATH "${POCO_ROOT_DIR}/lib/cmake/Poco")
  list(APPEND CMAKE_PREFIX_PATH "${POCO_ROOT_DIR}")
endif ()

if (PROTOBUF_ROOT_DIR)
  list(APPEND CMAKE_MODULE_PATH "${PROTOBUF_ROOT_DIR}/cmake")
  list(APPEND CMAKE_PREFIX_PATH "${PROTOBUF_ROOT_DIR}")
endif ()

if (GTEST_ROOT_DIR)
  list(APPEND CMAKE_MODULE_PATH "${GTEST_ROOT_DIR}/lib/cmake/GTest")
  list(APPEND CMAKE_PREFIX_PATH "${GTEST_ROOT_DIR}")
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(MSVC_COMPILE_FLAGS "/MP")
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_COMPILE_FLAGS}")
  if (MSVC_TOOLSET_VERSION GREATER 140)
    #        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /W4")
  endif ()
endif ()
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wextra -Wshadow")
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT result)
  if (result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif ()
endif ()

if (WIN32)
  set(VER_SCRIPT "${CMAKE_SOURCE_DIR}/version.bat")
else (WIN32)
  set(VER_SCRIPT "${CMAKE_SOURCE_DIR}/version.sh")
endif (WIN32)

if (CYGWIN)
  set(ENABLE_USING_SENDFILE OFF)
endif ()

execute_process(COMMAND ${VER_SCRIPT}
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

file(STRINGS "${CMAKE_SOURCE_DIR}/VERSION.txt" VERSION)
message(STATUS "VERSION : ${VERSION}")

file(STRINGS "${CMAKE_SOURCE_DIR}/COMMIT.txt" COMMIT)
message(STATUS "COMMIT : ${COMMIT}")

string(REPLACE "." "," VER_FILE ${VERSION})
add_definitions(-DVER_FILEVERSION=${VER_FILE},0)
add_definitions(-DVER_FILEVERSION_STR=\"${VERSION}.0\")
add_definitions(-DVER_PRODUCTVERSION=${VER_FILE},0)
add_definitions(-DVER_PRODUCTVERSION_STR=\"${VERSION}.0\")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SOURCE_DIR}/share/Version.hpp")

set(SHARE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/share")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

add_subdirectory(external)

add_subdirectory(libs)

add_subdirectory(bins)

if (ENABLE_TESTS)
  add_subdirectory(tests)
endif ()

if (ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif ()
