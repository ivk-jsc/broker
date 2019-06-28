list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
find_package(UUID REQUIRED)

if (UUID_FOUND)
    set_target_properties(UUID::UUID
                          PROPERTIES IMPORTED_GLOBAL TRUE)
endif ()