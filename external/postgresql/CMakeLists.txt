find_package(PostgreSQL REQUIRED)

if (PostgreSQL_FOUND)
  set_target_properties(PostgreSQL::PostgreSQL
                        PROPERTIES IMPORTED_GLOBAL TRUE)
endif ()