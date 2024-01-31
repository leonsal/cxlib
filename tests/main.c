#include <stdio.h>
#include <asm-generic/errno.h>
#include <stdio.h>
#include "cx_alloc.h"
#include "cx_pool_allocator.h"

#define cx_list_name    list
#define cx_list_type    int
#define cx_list_implement
#define cx_list_instance_allocator
#include "cx_list.h"


int main() {

    list l = list_init(NULL);
    list_push(&l, 1);
    list_push(&l, 2);
}



// #include <stdio.h>
// #include <asm-generic/errno.h>
// #include <stdio.h>
// #include "cx_alloc.h"
// #include "cx_pool_allocator.h"
//
// #include "logger.h"
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
// #include "queue.h"
//
//
// int main() {
//
//     LOG_INIT();
//     LOGW("START");
//     cxAllocPoolTests();
//     cxArrayTests();
//     cxHmapTests();
//     cxStrTests();
//     cxQueueTests();
//     LOGW("END");
// }

