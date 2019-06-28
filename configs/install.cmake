install(FILES ${CMAKE_SOURCE_DIR}/configs/broker.xml         
            DESTINATION etc/upmq
            COMPONENT server)

install(FILES ${CMAKE_SOURCE_DIR}/configs/s2s.xml         
            DESTINATION etc/upmq
            COMPONENT server)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/configs/www         
            DESTINATION share/upmq
            COMPONENT server)