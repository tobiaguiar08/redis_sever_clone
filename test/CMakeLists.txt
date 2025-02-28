cmake_minimum_required(VERSION 3.22)

enable_language(CXX)

set(CPPUTEST_HOME ${CMAKE_CURRENT_SOURCE_DIR}/cpputest)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

find_package(CppUTest REQUIRED)

set(CMAKE_CXX_FLAGS "-include ${CPPUTEST_HOME}/include/CppUTest/MemoryLeakDetectorNewMacros.h")

set(TEST_FILES
        test_protocol.cpp
        test_server.cpp
        test_command_hdlr.cpp
)

add_executable(redis_server_tests main.cpp ${TEST_FILES})

target_include_directories(redis_server_tests PRIVATE
    "${CPPUTEST_HOME}/include"
    "${INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(redis_server_tests PRIVATE
    -Wall -Wextra
)

target_link_libraries(redis_server_tests PRIVATE
    CppUTest::CppUTest
    CppUTest::CppUTestExt
    production_lib
)
