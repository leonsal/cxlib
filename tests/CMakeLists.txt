set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(cxtests
    main.c
    registry.c
    alloc.c
    array.c
    hmap.c
    string.c
    cqueue.c
    list.c
    var.c 
    json_build.c
    json_parse.c
    tpool.c
    bqueue.c
    tracer.c
    timer.c
    tflow.c
)
target_link_libraries(cxtests cxlib m)

