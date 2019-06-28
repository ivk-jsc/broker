find_package(ODBC REQUIRED)

if (ODBC_FOUND)
    set_target_properties(ODBC::ODBC
                          PROPERTIES IMPORTED_GLOBAL TRUE)
endif ()