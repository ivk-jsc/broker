cmake_minimum_required(VERSION 3.12...3.13)

project(simpleproducer
        VERSION 1.0
        DESCRIPTION "C++ MQ-Producer")

add_executable(simpleproducer)

include_directories(${SHARE_DIR})
include_directories(../optparse)

set(SOURCE_FILES main.cpp QSender.cpp TPublisher.cpp)

target_sources(simpleproducer PRIVATE ${SOURCE_FILES})
target_link_libraries(simpleproducer PRIVATE upmq::client Threads::Threads)