cmake_minimum_required(VERSION 3.12...3.13)

project(simpleconsumer
        VERSION 1.0
        DESCRIPTION "C++ MQ-Consumer")

add_executable(simpleconsumer)

include_directories(${SHARE_DIR})
include_directories(../optparse)

set(SOURCE_FILES main.cpp QReceiver.cpp TSubscriber.cpp)

target_sources(simpleconsumer PRIVATE ${SOURCE_FILES})
target_link_libraries(simpleconsumer PRIVATE upmq::client Threads::Threads)