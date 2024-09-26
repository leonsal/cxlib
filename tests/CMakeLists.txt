cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(cxlibtests C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(SOURCES
    src/main.c
    src/alloc.c
    src/array.c
    src/hmap.c
    src/string.c
    src/cqueue.c
    src/list.c
    src/var.c 
    src/json_build.c
    src/json_parse.c
    src/tpool.c
    src/bqueue.c
    src/tracer.c
    src/task.c
    #bench_hmap.c
    #timer.c 
)

add_subdirectory(.. cxlib)
add_executable(tests ${SOURCES})
set_property(TARGET tests PROPERTY C_STANDARD  11)
target_compile_options(tests PRIVATE -Wall -Wno-unused-function)
target_link_libraries(tests cxlib m)
