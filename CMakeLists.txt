cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(cxlibproj C)

set(SOURCES
    include/cx_alloc.h
    include/cx_array.h
    include/cx_str.h
    include/cx_hmap.h
    include/cx_cqueue.h
    include/cx_logger.h
    src/cx_alloc.c
    src/cx_pool_allocator.c
    src/cx_logger.c
    src/cx_str.c
    src/cx_hmap.c
    src/cx_timer.c
    src/cx_var.c
    src/cx_bqueue.c
    src/cx_json_build.c
    src/cx_json_parse.c
    src/json.c
    src/json.h
    src/cx_tpool.c
    src/cx_writer.c
)

add_library(cxlib ${SOURCES})
set_property(TARGET cxlib PROPERTY C_STANDARD  11)
target_include_directories(cxlib
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

