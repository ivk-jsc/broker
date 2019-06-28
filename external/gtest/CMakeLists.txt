find_package(GTest REQUIRED)

if (GTest_FOUND)
    set_target_properties(GTest::GTest
                          GTest::Main
                          PROPERTIES IMPORTED_GLOBAL TRUE)
else ()
    message(STATUS "Try to set GTEST_ROOT_DIR, for example cmake -DGEST_ROOT_DIR=/usr/local")
endif ()