/*
#include <stdlib.h>

#define cx_hmap_name mapcs
#define cx_hmap_key  char*
#define cx_hmap_val  char* 
#define cx_hmap_cmp_key(k1,k2,s)\
    strcmp(*(char**)k1,*(char**)k2)
#define cx_hmap_hash_key(k,s)\
    cx_hmap_hash_fnv1a32(*((char**)k), strlen(*(char**)k))
#define cx_hmap_free_key(k)\
    free(*k)
#define cx_hmap_free_val(k)\
    free(*k)
#define cx_hmap_implement
#include "cx_hmap2.h"

#define cx_str_name cxstr
#define cx_str_implement
#include "cx_str.h"

#define cx_hmap_name mapcxs
#define cx_hmap_key  cxstr
#define cx_hmap_val  cxstr 
#define cx_hmap_cmp_key(k1,k2,s)\
    cxstr_cmps(k1,k2)
#define cx_hmap_hash_key(k,s)\
    cx_hmap_hash_fnv1a32(k->data, cxstr_len((cxstr*)k))
#define cx_hmap_free_key cxstr_free
#define cx_hmap_free_val cxstr_free
#define cx_hmap_implement
#include "cx_hmap2.h"

int main() {

    {
        mapcs m = mapcs_init(0);
        char* key = strdup("key1");
        char* val = strdup("val1");
        mapcs_set(&m, key, val);
        mapcs_del(&m, "key1");
        mapcs_free(&m);
    }

    {
        mapcxs m = mapcxs_init(0);
        cxstr key = cxstr_initc("key");
        cxstr val = cxstr_initc("val");
        mapcxs_set(&m, key, val);
        // cxstr key2 = cxstr_initc("key");
        // mapcxs_del(&m, key2);
        // cxstr_free(&key2);
        mapcxs_free(&m);
    }
}
*/

#include <asm-generic/errno.h>
#include "cx_pool_allocator.h"

#include "logger.h"
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"
#include "queue.h"
#include "list.h"
#include "timer.h"
#include "var.h"
#include "json_build.h"
#include "json_parse.h"

int main() {

    LOG_INIT();
    LOGW("START");
    // cxAllocPoolTests();
    // cxArrayTests();
    cxHmapTests();
    // cxStrTests();
    // cxQueueTests();
    // cx_list_tests();
    // cx_timer_tests();
    // cx_var_tests();
    //json_build_tests();
    //json_parse_tests();
    LOGW("END");
}

