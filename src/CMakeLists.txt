cmake_minimum_required(VERSION 3.22)

enable_language(C)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(SRC_FILES
    ${SRC_DIR}/protocol.c
    ${SRC_DIR}/server.c
    ${SRC_DIR}/cmd_hdlr.c
)

add_library(production_lib ${SRC_FILES})

set(CMAKE_C_STANDARD 99)

target_include_directories(production_lib PRIVATE ${INCLUDE_DIR})

add_executable(redis_exec ${SRC_DIR}/main.c)

target_include_directories(redis_exec PRIVATE ${INCLUDE_DIR})

target_link_libraries(redis_exec PRIVATE production_lib)
