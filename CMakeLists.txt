cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(cxlibproj C)

set(SOURCES
    include/cx_alloc.h
    include/cx_array.h
    include/cx_bqueue.h
    include/cx_cqueue.h
    include/cx_error.h
    include/cx_hmap.h
    include/cx_hmap2.h
    include/cx_json_build.h
    include/cx_json_parse.h
    include/cx_logger.h
    include/cx_pool_allocator.h
    include/cx_queue.h
    include/cx_str.h
    include/cx_timer.h
    include/cx_tpool.h
    include/cx_tracer.h
    include/cx_var.h
    include/cx_writer.h
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
    src/cx_tracer.c
    src/cx_task.c
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

