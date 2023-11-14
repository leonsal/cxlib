#include <stdio.h>
#include <assert.h>

#define cx_chan_name mychan
#define cx_chan_type int
#define cx_chan_implement
#include "cx_chan.h"


int main() {

    mychan c1 = mychan_init(0);

    return 0;
}

// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
// #include "queue.h"
//
// int main() {
//
//     cxAllocBlockTests();
//     cxArrayTests();
//     cxHmapTests();
//     cxStrTests();
//     cxQueueTests();
//
//     return 0;
// }
//
