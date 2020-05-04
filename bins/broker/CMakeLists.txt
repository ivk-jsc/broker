cmake_minimum_required(VERSION 3.12...3.13)

project(broker
        VERSION 3.4
        DESCRIPTION "C++ MQ-Server")

add_executable(broker)

include_directories(${SHARE_DIR})
include_directories(${EXTERNAL_DIR}/concurrent_queue)
include_directories(${EXTERNAL_DIR}/optional-lite)

include_directories(app)
include_directories(server)
include_directories(version)
include_directories(asynchandler)
include_directories(connection)
include_directories(defines)
include_directories(log)
include_directories(message)
include_directories(destination)
include_directories(exchange)
include_directories(storage)
include_directories(session)
include_directories(config)
include_directories(subscription)
include_directories(sender)
include_directories(sys)
include_directories(selector)
include_directories(s2s_proto)
include_directories(misc)
include_directories(net)

set(SOURCE_FILES
    version/About.cpp
    version/About.h
    defines/MiscDefines.h
    defines/Defines.h
    defines/Exception.h
    defines/Exception.cpp
    defines/MessageDefines.h
    defines/StorageDefines.h
    defines/Errors.h
    defines/ProtoBuf.h
    log/AsyncLogger.h
    log/AsyncLogger.cpp
    message/MessageDataContainer.h
    message/MessageDataContainer.cpp
    message/MappedDBMessage.cpp
    message/MappedDBMessage.h
    selector/Selector.cpp
    selector/Selector.h
    selector/SelectorExpression.cpp
    selector/SelectorExpression.h
    selector/SelectorToken.cpp
    selector/SelectorToken.h
    selector/SelectorValue.cpp
    selector/SelectorValue.h
    selector/PropertyHandler.h
    selector/SelectorRegex.h
    storage/MessageStorage.cpp
    storage/MessageStorage.h
    subscription/Subscription.cpp
    subscription/Subscription.h
    destination/Destination.cpp
    destination/Destination.h
    destination/TopicDestination.cpp
    destination/TopicDestination.h
    destination/QueueDestination.cpp
    destination/QueueDestination.h
    destination/TemporaryTopicDestination.cpp
    destination/TemporaryTopicDestination.h
    destination/TemporaryQueueDestination.cpp
    destination/TemporaryQueueDestination.h
    exchange/Exchange.cpp
    exchange/Exchange.h
    asynchandler/AsyncTCPHandler.cpp
    asynchandler/AsyncTCPHandler.h
    asynchandler/AsyncHandlerRegestry.cpp
    asynchandler/AsyncHandlerRegestry.h
    app/MainApplication.cpp
    app/MainApplication.h
    app/SWFApplication.h
    app/SWFApplication.cpp
    app/swf_dynamic.h
    misc/Singleton.h
    misc/NextBindParam.h
    misc/MoveableRWLock.h
    misc/MoveableRWLock.cpp
    misc/FixedSizeUnorderedMap.h
    connection/Connection.cpp
    connection/Connection.h
    session/Session.cpp
    session/Session.h
    session/DBMSConnectionPool.cpp
    session/DBMSConnectionPool.h
    server/Broker.cpp
    server/Broker.h
    config/Configuration.cpp
    config/Configuration.h
    destination/DestinationFactory.cpp
    destination/DestinationFactory.h
    misc/CircularQueue.h
    subscription/Consumer.cpp
    subscription/Consumer.h
    message/MessageInfo.cpp
    message/MessageInfo.h
    message/MessagePropertyInfo.cpp
    message/MessagePropertyInfo.h
    sender/Sender.cpp
    sender/Sender.h
    sender/QueueSender.cpp
    sender/QueueSender.h
    sender/TopicSender.cpp
    sender/TopicSender.h
    sender/SenderList.cpp
    sender/SenderList.h
    destination/DestinationOwner.h
    s2s_proto/S2SProto.h
    session/DBMSSession.cpp
    session/DBMSSession.h
    net/SocketReactor.cpp
    net/SocketReactor.h
    net/ParallelSocketReactor.h
    net/ParallelSocketAcceptor.h
    net/SocketNotifier.cpp
    net/SocketNotifier.h
    net/SocketNotification.cpp
    net/SocketNotification.h
    net/NotificationCenter.cpp
    net/NotificationCenter.h)

if (ENABLE_WEB_ADMIN)
  add_definitions(-DENABLE_WEB_ADMIN)
  include_directories(web)
  set(SOURCE_FILES ${SOURCE_FILES}
      web/ClientsPageReplacer.cpp
      web/ClientsPageReplacer.h
      web/ClientsRowPageReplacer.cpp
      web/ClientsRowPageReplacer.h
      web/DestinationRowPageReplacer.cpp
      web/DestinationRowPageReplacer.h
      web/IndexPageReplacer.cpp
      web/IndexPageReplacer.h
      web/MainPageReplacer.cpp
      web/MainPageReplacer.h
      web/MessagesPageReplacer.cpp
      web/MessagesPageReplacer.h
      web/MessagesRowPageReplacer.cpp
      web/MessagesRowPageReplacer.h
      web/QueuesPageReplacer.cpp
      web/QueuesPageReplacer.h
      web/TemplateParamReplacer.cpp
      web/TemplateParamReplacer.h
      web/TopicsPageReplacer.cpp
      web/TopicsPageReplacer.h
      web/WebAdminRequestHandler.cpp
      web/WebAdminRequestHandler.h
      web/WebAdminRequestHandlerFactory.cpp
      web/WebAdminRequestHandlerFactory.h
      )
endif ()

if (ENABLE_USING_SENDFILE)
  add_definitions(-DENABLE_USING_SENDFILE)
  set(SOURCE_FILES ${SOURCE_FILES}
      asynchandler/sendfile/SendFile.cpp
      asynchandler/sendfile/SendFile.h
      asynchandler/sendfile/fileno_stream.cpp
      asynchandler/sendfile/fileno_stream.h
      )
endif (ENABLE_USING_SENDFILE)

foreach (SOURCE IN ITEMS ${SOURCE_FILES})
  set(SOURCE_FILES_NEW ${SOURCE_FILES_NEW} "${SOURCE}")
endforeach ()

foreach (SOURCE IN ITEMS ${SOURCE_FILES_NEW})
  get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
  string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
  source_group("${SOURCE_PATH_MSVC}" FILES "${SOURCE}")
endforeach ()

target_sources(broker PRIVATE main.cpp)
if (WIN32)
  target_sources(broker PRIVATE app/broker.rc)
endif ()
target_sources(broker PRIVATE ${SOURCE_FILES_NEW})

set(POCO_LIBS
    Poco::Foundation
    Poco::Util
    Poco::Net
    Poco::XML
    Poco::JSON
    )

get_target_property(Poco_VERSION_MAJOR Poco::Foundation Poco_VERSION_MAJOR)
get_target_property(Poco_VERSION_MINOR Poco::Foundation Poco_VERSION_MINOR)
add_definitions(-DPOCO_VERSION_MAJOR=${Poco_VERSION_MAJOR})
add_definitions(-DPOCO_VERSION_MINOR=${Poco_VERSION_MINOR})
if (Poco_VERSION_MAJOR GREATER 1)
  list(APPEND POCO_LIBS Poco::SQL Poco::SQLSQLite)
  if (ENABLE_POSTGRESQL)
    add_definitions(-DHAS_POSTGRESQL)
    list(APPEND POCO_LIBS Poco::SQLPostgreSQL)
  endif ()
  if (ENABLE_ODBC)
    add_definitions(-DHAS_ODBC)
    list(APPEND POCO_LIBS Poco::SQLODBC)
  endif ()
else ()
  list(APPEND POCO_LIBS Poco::Data Poco::DataSQLite)
  if (ENABLE_POSTGRESQL)
    add_definitions(-DHAS_POSTGRESQL)
    list(APPEND POCO_LIBS Poco::DataPostgreSQL)
  endif ()
  if (ENABLE_ODBC)
    add_definitions(-DHAS_ODBC)
    list(APPEND POCO_LIBS Poco::DataODBC)
  endif ()
endif ()

target_link_libraries(broker PRIVATE upmq::protocol protobuf::libprotobuf ${CMAKE_DL_LIBS} ${POCO_LIBS})
if (NOT WIN32)
  target_link_libraries(broker PRIVATE Threads::Threads)
else ()
  target_link_libraries(broker PRIVATE ws2_32 mswsock iphlpapi)
endif ()

macro(copy_poco_lib APP DLL)
  # find the release *.dll file
  get_target_property(Poco${DLL}Location Poco::${DLL} LOCATION)
  # find the debug *d.dll file
  get_target_property(Poco${DLL}LocationDebug Poco::${DLL} IMPORTED_LOCATION_DEBUG)

  set(POCOLIB $<$<CONFIG:Debug>:${Poco${DLL}LocationDebug}> $<$<NOT:$<CONFIG:Debug>>:${Poco${DLL}Location}>)

  string(FIND "${POCOLIB}" ".dll" HAS_DLL_EXT)

  if (${HAS_DLL_EXT} GREATER -1)
    add_custom_command(TARGET ${APP} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Debug>:${Poco${DLL}LocationDebug}> $<$<NOT:$<CONFIG:Debug>>:${Poco${DLL}Location}> $<TARGET_FILE_DIR:${APP}>)

    install(FILES $<$<CONFIG:Debug>:${Poco${DLL}LocationDebug}> $<$<NOT:$<CONFIG:Debug>>:${Poco${DLL}Location}>
            DESTINATION bin
            COMPONENT server)
  endif ()
endmacro()

if (WIN32)
  foreach (LIB IN ITEMS ${POCO_LIBS})
    string(SUBSTRING ${LIB} 6 -1 POCO_LIB)
    copy_poco_lib(broker ${POCO_LIB})
  endforeach ()
endif ()
install(TARGETS broker
        RUNTIME
        DESTINATION bin
        COMPONENT server)

install(FILES ${CMAKE_SOURCE_DIR}/configs/broker.xml
        DESTINATION etc/upmq
        COMPONENT server)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/configs/www
        DESTINATION share/upmq
        COMPONENT server)
