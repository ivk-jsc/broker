cmake_minimum_required(VERSION 3.12...3.13)
project(libupmqprotocol
        VERSION 1.2
        DESCRIPTION "mq client-server protocol (protobuf)")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "protocol.proto")

set_directory_properties(ADDITIONAL_MAKE_CLEAN_FILES ${PROTO_SRCS} ${PROTO_HDRS})
get_filename_component(PROTO_HDR_DIRECTORY ${PROTO_HDRS} DIRECTORY)

include_directories(${PROTO_HDR_DIRECTORY})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(libupmqprotocol STATIC)

if (MSVC)
  target_compile_options(libupmqprotocol PUBLIC /wd4125 /wd4100)
else ()
  target_compile_options(libupmqprotocol PUBLIC -Wno-unused-parameter)
endif ()

target_sources(libupmqprotocol PRIVATE ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(libupmqprotocol PRIVATE protobuf::libprotobuf)

set_target_properties(libupmqprotocol PROPERTIES DEBUG_OUTPUT_NAME "upmqprotocold" RELEASE_OUTPUT_NAME "upmqprotocol" RELWITHDEBINFO_OUTPUT_NAME "upmqprotocol")
set_target_properties(libupmqprotocol PROPERTIES PREFIX "lib")
target_include_directories(libupmqprotocol PUBLIC SYSTEM "${PROTO_HDR_DIRECTORY}")

add_library(upmq::protocol ALIAS libupmqprotocol)