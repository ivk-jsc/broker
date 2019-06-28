cmake_minimum_required(VERSION 3.12...3.13)
project(libupmq
        VERSION 1.2
        DESCRIPTION "mq client library")


add_library(libupmq_objlib OBJECT)

add_library(libupmq_static STATIC $<TARGET_OBJECTS:libupmq_objlib>)
add_library(libupmq SHARED $<TARGET_OBJECTS:libupmq_objlib>)

set_target_properties(libupmq_static PROPERTIES DEBUG_OUTPUT_NAME "upmq_staticd" RELEASE_OUTPUT_NAME "upmq_static" RELWITHDEBINFO_OUTPUT_NAME "upmq_static")
set_target_properties(libupmq_static PROPERTIES PREFIX "lib")

add_dependencies(libupmq_static libupmq_objlib)

set_target_properties(libupmq PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(libupmq PROPERTIES DEBUG_OUTPUT_NAME "upmqd" RELEASE_OUTPUT_NAME "upmq" RELWITHDEBINFO_OUTPUT_NAME "upmq")
set_target_properties(libupmq PROPERTIES PREFIX "lib")
set_target_properties(libupmq PROPERTIES IMPORT_PREFIX "lib")

add_dependencies(libupmq libupmq_objlib)

target_include_directories(libupmq PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include/upmq>  # <prefix>/include/upmq/cms
                           )

add_library(upmq::client_static ALIAS libupmq_static)
add_library(upmq::client ALIAS libupmq)

if (WIN32)
    add_definitions(/DNOMINMAX)
    add_definitions(/DCMS_DLL)
    add_definitions(/DCMS_EXPORTS)
    add_compile_options(/FS)
endif (WIN32)

include_directories(${SHARE_DIR})

set(LIBUPMQ_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${LIBUPMQ_DIR})

set(CMS_DIR ${LIBUPMQ_DIR}/cms)
set(DECAF_DIR ${LIBUPMQ_DIR}/decaf)
set(INTERNAL_LIBUPMQ_DIR ${LIBUPMQ_DIR}/libupmq)
set(STATE_DIR ${LIBUPMQ_DIR}/state)
set(THREADS_DIR ${LIBUPMQ_DIR}/threads)
set(TRANSPORT_DIR ${LIBUPMQ_DIR}/transport)

# --- cms sources ---
file(
        GLOB_RECURSE CMS_HEADER_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${CMS_DIR}/*.h*"
)

file(
        GLOB_RECURSE CMS_SOURCE_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${CMS_DIR}/*.cpp*"
)

# --- decaf sources ---
file(
        GLOB_RECURSE DECAF_HEADER_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${DECAF_DIR}/*.h*"
)

file(
        GLOB_RECURSE DECAF_SOURCE_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${DECAF_DIR}/*.cpp*"
)

if (WIN32)
    list(REMOVE_ITEM DECAF_HEADER_FILES "decaf/internal/util/concurrent/unix/PlatformDefs.h")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/util/concurrent/unix/Atomics.cpp")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/util/concurrent/unix/PlatformThread.cpp")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/security/unix/SecureRandomImpl.cpp")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/security/unix/SecureRandomImpl.h")
else (WIN32)
    list(REMOVE_ITEM DECAF_HEADER_FILES "decaf/internal/util/concurrent/windows/PlatformDefs.h")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/util/concurrent/windows/Atomics.cpp")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/util/concurrent/windows/PlatformThread.cpp")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/security/windows/SecureRandomImpl.cpp")
    list(REMOVE_ITEM DECAF_SOURCE_FILES "decaf/internal/security/windows/SecureRandomImpl.h")
endif (WIN32)

# --- internal libupmq sources ---
file(
        GLOB_RECURSE INTERNAL_LIBUPMQ_HEADER_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${INTERNAL_LIBUPMQ_DIR}/*.h*"
)

file(
        GLOB_RECURSE INTERNAL_LIBUPMQ_SOURCE_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${INTERNAL_LIBUPMQ_DIR}/*.cpp*"
)

# --- state sources ---
file(
        GLOB_RECURSE STATE_HEADER_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${STATE_DIR}/*.h*"
)

file(
        GLOB_RECURSE STATE_SOURCE_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${STATE_DIR}/*.cpp*"
)

# --- state sources ---
file(
        GLOB_RECURSE THREADS_HEADER_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${THREADS_DIR}/*.h*"
)

file(
        GLOB_RECURSE THREADS_SOURCE_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${THREADS_DIR}/*.cpp*"
)

# --- transport sources ---
file(
        GLOB_RECURSE TRANSPORT_HEADER_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${TRANSPORT_DIR}/*.h*"
)

file(
        GLOB_RECURSE TRANSPORT_SOURCE_FILES
        LIST_DIRECTORIES false
        RELATIVE "${LIBUPMQ_DIR}"
        "${TRANSPORT_DIR}/*.cpp*"
)

if (WIN32)
    set(SOURCE_FILES ${SOURCE_FILES} libupmq.rc)
endif ()

# --- repack cms ---
foreach (SOURCE IN ITEMS ${CMS_SOURCE_FILES})
    set(CMS_SOURCE_FILES_NEW ${CMS_SOURCE_FILES_NEW} "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${CMS_HEADER_FILES})
    set(CMS_HEADER_FILES_NEW ${CMS_HEADER_FILES_NEW} "${HEADER}")
    set_target_properties(libupmq_objlib PROPERTIES PUBLIC_HEADER "${HEADER}")
endforeach ()

foreach (SOURCE IN ITEMS ${CMS_SOURCE_FILES_NEW})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${CMS_HEADER_FILES_NEW})
    get_filename_component(HEADER_PATH "${HEADER}" PATH)
    string(REPLACE "/" "\\" HEADER_PATH_MSVC "${HEADER_PATH}")
    source_group("${HEADER_PATH_MSVC}" FILES "${HEADER}")
endforeach ()


# --- repack cms ---
foreach (SOURCE IN ITEMS ${DECAF_SOURCE_FILES})
    set(DECAF_SOURCE_FILES_NEW ${DECAF_SOURCE_FILES_NEW} "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${DECAF_HEADER_FILES})
    set(DECAF_HEADER_FILES_NEW ${DECAF_HEADER_FILES_NEW} "${HEADER}")
endforeach ()

foreach (SOURCE IN ITEMS ${DECAF_SOURCE_FILES_NEW})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${DECAF_HEADER_FILES_NEW})
    get_filename_component(HEADER_PATH "${HEADER}" PATH)
    string(REPLACE "/" "\\" HEADER_PATH_MSVC "${HEADER_PATH}")
    source_group("${HEADER_PATH_MSVC}" FILES "${HEADER}")
endforeach ()

# --- repack internal libupmq ---
foreach (SOURCE IN ITEMS ${INTERNAL_LIBUPMQ_SOURCE_FILES})
    set(INTERNAL_LIBUPMQ_SOURCE_FILES_NEW ${INTERNAL_LIBUPMQ_SOURCE_FILES_NEW} "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${INTERNAL_LIBUPMQ_HEADER_FILES})
    set(INTERNAL_LIBUPMQ_HEADER_FILES_NEW ${INTERNAL_LIBUPMQ_HEADER_FILES_NEW} "${HEADER}")
endforeach ()

foreach (SOURCE IN ITEMS ${INTERNAL_LIBUPMQ_SOURCE_FILES_NEW})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${INTERNAL_LIBUPMQ_HEADER_FILES_NEW})
    get_filename_component(HEADER_PATH "${HEADER}" PATH)
    string(REPLACE "/" "\\" HEADER_PATH_MSVC "${HEADER_PATH}")
    source_group("${HEADER_PATH_MSVC}" FILES "${HEADER}")
endforeach ()

# --- repack state ---
foreach (SOURCE IN ITEMS ${STATE_SOURCE_FILES})
    set(STATE_SOURCE_FILES_NEW ${STATE_SOURCE_FILES_NEW} "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${STATE_HEADER_FILES})
    set(STATE_HEADER_FILES_NEW ${STATE_HEADER_FILES_NEW} "${HEADER}")
endforeach ()

foreach (SOURCE IN ITEMS ${STATE_SOURCE_FILES_NEW})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${STATE_HEADER_FILES_NEW})
    get_filename_component(HEADER_PATH "${HEADER}" PATH)
    string(REPLACE "/" "\\" HEADER_PATH_MSVC "${HEADER_PATH}")
    source_group("${HEADER_PATH_MSVC}" FILES "${HEADER}")
endforeach ()

# --- repack threads ---
foreach (SOURCE IN ITEMS ${THREADS_SOURCE_FILES})
    set(THREADS_SOURCE_FILES_NEW ${THREADS_SOURCE_FILES_NEW} "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${THREADS_HEADER_FILES})
    set(THREADS_HEADER_FILES_NEW ${THREADS_HEADER_FILES_NEW} "${HEADER}")
endforeach ()

foreach (SOURCE IN ITEMS ${THREADS_SOURCE_FILES_NEW})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${THREADS_HEADER_FILES_NEW})
    get_filename_component(HEADER_PATH "${HEADER}" PATH)
    string(REPLACE "/" "\\" HEADER_PATH_MSVC "${HEADER_PATH}")
    source_group("${HEADER_PATH_MSVC}" FILES "${HEADER}")
endforeach ()

# --- repack transport ---
foreach (SOURCE IN ITEMS ${TRANSPORT_SOURCE_FILES})
    set(TRANSPORT_SOURCE_FILES_NEW ${TRANSPORT_SOURCE_FILES_NEW} "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${TRANSPORT_HEADER_FILES})
    set(TRANSPORT_HEADER_FILES_NEW ${TRANSPORT_HEADER_FILES_NEW} "${HEADER}")
endforeach ()

foreach (SOURCE IN ITEMS ${TRANSPORT_SOURCE_FILES_NEW})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
    source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE}")
endforeach ()

foreach (HEADER IN ITEMS ${TRANSPORT_HEADER_FILES_NEW})
    get_filename_component(HEADER_PATH "${HEADER}" PATH)
    string(REPLACE "/" "\\" HEADER_PATH_MSVC "${HEADER_PATH}")
    source_group("${HEADER_PATH_MSVC}" FILES "${HEADER}")
endforeach ()

if (NOT WIN32)
    set(SYS_LINK_LIBS ${CMAKE_DL_LIBS})
    if (NOT APPLE)
        set(SYS_LINK_LIBS ${SYS_LINK_LIBS} -lrt -lcrypt Threads::Threads)
    endif (NOT APPLE)
else (NOT WIN32)
    set(SYS_LINK_LIBS ${SYS_LINK_LIBS}
        ws2_32
        mswsock
        rpcrt4
        iphlpapi
        )
endif (NOT WIN32)

target_sources(libupmq_objlib INTERFACE ${CMS_HEADER_FILES_NEW} PRIVATE ${CMS_SOURCE_FILES_NEW}
               ${DECAF_HEADER_FILES_NEW} ${DECAF_SOURCE_FILES_NEW}
               ${INTERNAL_LIBUPMQ_HEADER_FILES_NEW} ${INTERNAL_LIBUPMQ_SOURCE_FILES_NEW}
               ${STATE_HEADER_FILES_NEW} ${STATE_SOURCE_FILES_NEW}
               ${THREADS_HEADER_FILES_NEW} ${THREADS_SOURCE_FILES_NEW}
               ${TRANSPORT_HEADER_FILES_NEW} ${TRANSPORT_SOURCE_FILES_NEW})


target_link_libraries(libupmq_objlib PRIVATE upmq::protocol
                      protobuf::libprotobuf
                      Poco::Foundation
                      Poco::Net
                      ${SYS_LINK_LIBS})

target_link_libraries(libupmq_static PRIVATE upmq::protocol
                      protobuf::libprotobuf
                      Poco::Foundation
                      Poco::Net
                      ${SYS_LINK_LIBS})

target_link_libraries(libupmq PRIVATE upmq::protocol
                      protobuf::libprotobuf
                      Poco::Foundation
                      Poco::Net
                      ${SYS_LINK_LIBS})

install(TARGETS libupmq
        EXPORT client
        RUNTIME DESTINATION lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION "include/upmq/cms"
        COMPONENT client
        NAMELINK_COMPONENT development
        PUBLIC_HEADER COMPONENT development
        )

install(EXPORT client NAMESPACE upmq::
        DESTINATION lib/cmake/libupmq
        )

install(FILES ${CMS_HEADER_FILES_NEW}
        DESTINATION "include/upmq/cms"
        )