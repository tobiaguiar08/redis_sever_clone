cmake_minimum_required(VERSION 3.22)

enable_language(CXX)

set(CPPUTEST_HOME ${CMAKE_CURRENT_SOURCE_DIR}/cpputest)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

find_package(CppUTest REQUIRED)

set(CMAKE_CXX_FLAGS "-include ${CPPUTEST_HOME}/include/CppUTest/MemoryLeakDetectorNewMacros.h")

set(TEST_FILES
        test_protocol.cpp
)

add_executable(${CMAKE_PROJECT_NAME} main.cpp ${TEST_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${CPPUTEST_HOME}/include"
    "${INCLUDE_DIR}"
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wall -Wextra
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    CppUTest::CppUTest
    CppUTest::CppUTestExt
    production_lib
)
