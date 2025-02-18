cmake_minimum_required(VERSION 3.22)

enable_language(C)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(production_lib ${SRC_DIR}/protocol.c)

set(CMAKE_C_STANDARD 99)

target_include_directories(production_lib PRIVATE ${INCLUDE_DIR})
