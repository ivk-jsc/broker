cmake_minimum_required(VERSION 3.1)
project(brokertest
        VERSION 3.4
        DESCRIPTION "C++ MQ-Tests")

cmake_policy(SET CMP0069 NEW)

enable_testing()
add_executable(brokertest)

if (WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
endif (WIN32)

include_directories(${SHARE_DIR})

if (WIN32)
  set(SYS_LINK_LIBS ${SYS_LINK_LIBS}
      ws2_32
      mswsock
      rpcrt4
      user32
      )
elseif (UNIX AND NOT APPLE)
  set(SYS_LINK_LIBS ${SYS_LINK_LIBS} UUID::UUID -pthread)
endif ()

set(SOURCE_FILES
    AsyncSenderTest.cpp
    BytesMessageTest.cpp
    ClientAckTest.cpp
    CMSListener.cpp
    CMSProvider.cpp
    ConnectionStartStopTest.cpp
    DurableTest.cpp
    IntegrationCommon.cpp
    JmsMessageGroupTest.cpp
    main.cpp
    MapMessageTest.cpp
    MessageTest.cpp
    QueueBrowserTest.cpp
    SelectorTest.cpp
    SimpleRollbackTest.cpp
    SimpleTest.cpp
    StreamMessageTest.cpp
    TextMessageTest.cpp
    TransactionTest.cpp
    )

set(HEADER_FILES
    AsyncSenderTest.h
    BytesMessageTest.h
    ClientAckTest.h
    CMSListener.h
    CMSProvider.h
    ConnectionStartStopTest.h
    DurableTest.h
    IntegrationCommon.h
    JmsMessageGroupTest.h
    MapMessageTest.h
    MessageTest.h
    QueueBrowserTest.h
    SelectorTest.h
    SimpleRollbackTest.h
    SimpleTest.h
    StreamMessageTest.h
    TextMessageTest.h
    TransactionTest.h
    )

target_sources(brokertest PRIVATE ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(brokertest PRIVATE
                      upmq::client
                      GTest::GTest
                      GTest::Main
                      Threads::Threads
                      ${SYS_LINK_LIBS})

add_test(AllTestsInBrokertest brokertest)