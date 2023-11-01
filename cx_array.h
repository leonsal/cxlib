#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

#ifndef cx_array_name
    #error "cx_array_name not defined"
#endif
#ifndef cx_array_type
    #error "cx_array_type not defined"
#endif

#define concat2_(a, b) a ## b
#define concat1_(a, b) concat2_(a, b)
#define func_name(name) concat1_(cx_array_name, name)

#ifdef cx_array_static
    #define linkage static
#else
    #define linkage
#endif

//
// Function names
//
#ifdef cx_array_camel_case
    #define name_init           Init
    #define name_init2          Init2
    #define name_free           Free
    #define name_clear          Clear
    #define name_clone          Clone
    #define name_cap            Cap
    #define name_len            Len
    #define name_empty          Empty
    #define name_setcap         SetCap
    #define name_setlen         SetLen
    #define name_push           Push
    #define name_pop            Pop
    #define name_append         Append
    #define name_append_array   AppendArray
    #define name_at             At
    #define name_last           Last
    #define name_reserve        Reserve
    #define name_insn           Insn
    #define name_ins            Ins
    #define name_deln           Deln
    #define name_del            Del
    #define name_delswap        DelSwap
    #define name_sort           Sort
#else
    #define name_init           _init
    #define name_init2          _init2
    #define name_free           _free
    #define name_clear          _clear
    #define name_clone          _clone
    #define name_cap            _cap
    #define name_len            _len
    #define name_empty          _empty
    #define name_setcap         _setcap
    #define name_setlen         _setlen
    #define name_push           _push
    #define name_pop            _pop
    #define name_append         _append
    #define name_append_array   _append_array
    #define name_at             _at
    #define name_last           _last
    #define name_reserve        _reserve
    #define name_insn           _insn
    #define name_ins            _ins
    #define name_deln           _deln
    #define name_del            _del
    #define name_delswap        _del_swap
    #define name_sort           _sort
#endif

//
// Declarations
//
typedef struct cx_array_name {
    const CxAllocator* alloc;
    ptrdiff_t   len;
    ptrdiff_t   cap;
    cx_array_type*  data;
} cx_array_name;

linkage cx_array_name func_name(name_init)(void);
linkage cx_array_name func_name(name_init2)(const CxAllocator*);
linkage void func_name(name_free)(cx_array_name* a);
linkage void func_name(name_clear)(cx_array_name* a);
linkage cx_array_name func_name(name_clone)(cx_array_name* a);
linkage ptrdiff_t func_name(name_cap)(cx_array_name* a);
linkage ptrdiff_t func_name(name_len)(cx_array_name* a);
linkage bool func_name(name_empty)(cx_array_name* a);
linkage void func_name(name_setcap)(cx_array_name* a, size_t cap);
linkage void func_name(name_setlen)(cx_array_name* a, size_t len);
linkage void func_name(name_push)(cx_array_name* a, cx_array_type v);
linkage cx_array_type func_name(name_pop)(cx_array_name* a);
linkage void func_name(name_append)(cx_array_name* a, cx_array_type* p, size_t n);
linkage void func_name(name_append_array)(cx_array_name* a, const cx_array_name* src);
linkage cx_array_type* func_name(name_at)(cx_array_name* a, size_t idx);
linkage cx_array_type func_name(name_last)(const cx_array_name* a);
linkage void func_name(name_reserve)(cx_array_name* a, size_t n);
linkage void func_name(name_insn)(cx_array_name* a, size_t i, size_t n);
linkage void func_name(name_ins)(cx_array_name* a, size_t i, cx_array_type v);
linkage void func_name(name_deln)(cx_array_name* a, size_t i, size_t n);
linkage void func_name(name_del)(cx_array_name* a, size_t i);
linkage void func_name(name_delswap)(cx_array_name* a, size_t i);
linkage void func_name(name_sort)(cx_array_name* a, int (*f)(const cx_array_type*, const cx_array_type*));

//
// Implementations
//
#ifdef cx_array_implement
void cxArrayGrowFn(void* ag, size_t elemsize, size_t addlen, size_t min_cap);

linkage cx_array_name func_name(name_init)(void) {
    return (cx_array_name) {
        .alloc = cxDefaultAllocator(),
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

linkage cx_array_name func_name(name_init2)(const CxAllocator* alloc) {
    return (cx_array_name) {
        .alloc = alloc,
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

linkage void func_name(name_free)(cx_array_name* a) {
    a->alloc->free(a->alloc->ctx, a->data, a->cap * sizeof(*(a->data)));
    a->len = 0;
    a->cap = 0;
    a->data = NULL;
}

linkage void func_name(name_clear)(cx_array_name* a) {
    a->len = 0;
}

linkage cx_array_name func_name(name_clone)(cx_array_name* a) {
    const size_t alloc_size = a->len * sizeof(*(a->data));
    cx_array_name cloned = {
        .alloc = a->alloc,
        .len   = a->len,
        .cap   = a->len,
        .data  = a->alloc->alloc(a->alloc->ctx, alloc_size),
    };\
    memcpy(cloned.data, a->data, alloc_size);
    return cloned;
}

linkage ptrdiff_t func_name(name_cap)(cx_array_name* a) {
    return a->cap;
}

linkage ptrdiff_t func_name(name_len)(cx_array_name* a) {
    return a->len;
}

linkage bool func_name(name_empty)(cx_array_name* a) {
    return a->len == 0;
} 

linkage void func_name(name_setcap)(cx_array_name* a, size_t cap) {
    cxArrayGrowFn(a, sizeof(*(a->data)), 0, cap);
}

linkage void func_name(name_setlen)(cx_array_name* a, size_t len) {
    if (a->cap < len) {
        cxArrayGrowFn(a, sizeof(*(a->data)), len, 0);
    }
    a->len = len;
}

linkage void func_name(name_push)(cx_array_name* a, cx_array_type v) {
    if (a->len >= a->cap) {
        cxArrayGrowFn(a, sizeof(*(a->data)), 1, 0);
    }
    a->data[a->len++] = v;
}
 
linkage cx_array_type func_name(name_pop)(cx_array_name* a) {
    a->len--;
    return a->data[a->len];
}

linkage void func_name(name_append)(cx_array_name* a, cx_array_type* p, size_t n) {
    if (a->len + n > a->cap) {
        cxArrayGrowFn(a, sizeof(*(a->data)), n, 0);
    }
    memcpy(&a->data[a->len], p, n * sizeof(*(a->data)));
    a->len += n;
}

linkage void func_name(name_append_array)(cx_array_name* a, const cx_array_name* src) {
    func_name(name_append)(a, src->data, src->len);
}

linkage cx_array_type* func_name(name_at)(cx_array_name* a, size_t idx) {
    if (idx > a->len) {
        abort();
    }
    return &a->data[idx];
}

linkage cx_array_type func_name(name_last)(const cx_array_name* a) {
    return a->data[a->len-1];
}

linkage void func_name(name_reserve)(cx_array_name* a, size_t n) {
    if (a->cap < a->len + n) {
        cxArrayGrowFn(a, sizeof(*(a->data)), 0, a->len+n);
    }
}

linkage void func_name(name_insn)(cx_array_name* a, size_t i, size_t n) {
    if (a->len + n > a->cap) {
        cxArrayGrowFn(a, sizeof(*(a->data)),n,0);
    }
    a->len += n;
    memmove(&a->data[i+n], &a->data[i], sizeof(*(a->data)) * (a->len-n-i));
}

linkage void func_name(name_ins)(cx_array_name* a, size_t i, cx_array_type v) {
    func_name(name_insn)(a, i, 1);
    a->data[i] = v;
}

linkage void func_name(name_deln)(cx_array_name* a, size_t i, size_t n) {
    memmove(&a->data[i], &a->data[i+n], sizeof(*(a->data)) * (a->len-n-i));
    a->len -= n;
}

linkage void func_name(name_del)(cx_array_name* a, size_t i) {
    func_name(name_deln)(a, i, 1);
}

linkage void func_name(name_delswap)(cx_array_name* a, size_t i) {
    a->data[i] = func_name(name_last)(a);
    a->len--;
}

linkage void func_name(name_sort)(cx_array_name* a, int (*f)(const cx_array_type*, const cx_array_type*)) {
    qsort(a->data,a->len,sizeof(*(a->data)),(int (*)(const void*,const void*))f);
}

#endif

#undef cx_array_name
#undef cx_array_type
#undef cx_array_static
#undef cx_array_camel_case
#undef concat2_
#undef concat1_
#undef func_name
#undef linkage
#undef name_init
#undef name_init2
#undef name_free
#undef name_clear
#undef name_clone
#undef name_cap
#undef name_len
#undef name_empty
#undef name_setcap
#undef name_setlen
#undef name_push
#undef name_pop
#undef name_append
#undef name_append_array
#undef name_at
#undef name_last
#undef name_reserve
#undef name_insn
#undef name_ins
#undef name_deln
#undef name_del
#undef name_delswap
#undef name_sort


