find_package(Threads REQUIRED)

if (Threads_FOUND)
  set_target_properties(Threads::Threads
                        PROPERTIES IMPORTED_GLOBAL TRUE)
endif ()