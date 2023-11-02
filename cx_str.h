/********************************************

Dynamic String Implementation


Example
-------

#define cx_str_name str
#define cx_str_implement
#include "cx_str.h"

#define cx_str_name str8
#define cx_str_cap 8
#define cx_str_implement
#include "cx_str.h"


int main() {

    str s1 = cxstr_initc("hello");
    str_catc(&s1, " world");
    printf("%s\n", str.data); 
    str_free(&s1);

    str8 s2 = str8_init();
    str8_printf("counter:%d\n", 42);
    printf("%s\n", str.data); 
    str8_free(&s2);

    return 0;
}

API documentation:
------------------

Configuration defines:
-   cx_str_name <name>
    Sets the name of the string type

-   cx_str_cap <8|16|32>
    Sets the string maximum capacity

-   cx_str_error_handler <func>
    Sets error handler function

-   cx_str_allocator
    Uses custom allocator per instance

-   cx_str_static
    Prefix functions with 'static'

-   cx_str_camel_case
    Uses camel case names instead of snake case

-   cx_str_implement
    Implements functions



********************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "cx_alloc.h"

// String capacity in number of bits
#define cx_str_cap8     8
#define cx_str_cap16    16
#define cx_str_cap32    32

// Default capacity
#ifndef cx_str_cap
    #define cx_str_cap  cx_str_cap32
#endif
#if cx_str_cap == cx_str_cap8
    #define cx_str_cap_type_ uint8_t
    #define cx_str_max_cap_  (UINT8_MAX)
#elif cx_str_cap == cx_str_cap16
    #define cx_str_cap_type_ uint16_t
    #define cx_str_max_cap_  (UINT16_MAX)
#elif cx_str_cap == cx_str_cap32
    #define cx_str_cap_type_ uint32_t
    #define cx_str_max_cap_  (UINT32_MAX)
#else
    #error "invalid cx string capacity bits"
#endif

// Error handler
#ifndef cx_str_error_handler
    #define cx_str_error_handler(msg)\
        printf("CXLIB STR ERROR:%s\n",msg);abort()
#endif

#ifdef cx_str_static
    #define linkage static
#else
    #define linkage
#endif

// Use custom instance allocator
#ifdef cx_str_allocator
    #define cx_str_alloc_field_\
        const CxAllocator* alloc;
    #define cx_str_alloc_global_
    #define str_alloc(s,n)\
        cx_alloc_alloc(s->alloc, n)
    #define str_free(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_str_alloc_field_
    #define cx_str_alloc_global_\
        linkage const CxAllocator* type_name(_allocator) = NULL;
    #define str_alloc(s,n)\
        cx_alloc_alloc(type_name(_allocator),n)
    #define str_free(s,p,n)\
        cx_alloc_free(type_name(_allocator),p,n)
#endif

// Auxiliary internal macros
#define concat2_(a, b) a ## b
#define concat1_(a, b) concat2_(a, b)
#define type_name(name) concat1_(cx_str_name, name)


//
// Function names
//
#ifdef cx_str_camel_case
    #define name_init           Init
    #define name_initn          Initn
    #define name_initc          Initc
    #define name_inits          Inits
    #define name_free           Free
    #define name_clear          Clear
    #define name_clone          Clone
    #define name_reserve        Reserve
    #define name_cap            Cap
    #define name_len            Len
    #define name_data           Data
    #define name_empty          Empty
    #define name_setcap         SetCap
    #define name_setlen         SetLen
    #define name_setn           Setn
    #define name_setc           Setc
    #define name_sets           Sets
    #define name_catn           Catn
    #define name_catc           Catc
    #define name_cats           Cats
    #define name_insn           Insn
    #define name_insc           Insc
    #define name_inss           Inss
    #define name_deln           Deln
    #define name_del            Del
    #define name_cmpn           Cmpn
    #define name_cmpc           Cmpc
    #define name_cmps           Cmpc
    #define name_vprintf        Vprintf
    #define name_printf         Printf
    #define name_findn          Findn
    #define name_findc          Findc
    #define name_finds          Finds
#else
    #define name_init           _init
    #define name_initn          _initn
    #define name_initc          _initc
    #define name_inits          _inits
    #define name_free           _free
    #define name_clear          _clear
    #define name_clone          _clone
    #define name_reserve        _reserve
    #define name_cap            _cap
    #define name_len            _len
    #define name_data           _data
    #define name_empty          _empty
    #define name_setcap         _setcap
    #define name_setlen         _setlen
    #define name_setn           _setn
    #define name_setc           _setc
    #define name_sets           _sets
    #define name_catn           _catn
    #define name_catc           _catc
    #define name_cats           _cats
    #define name_insn           _insn
    #define name_insc           _insc
    #define name_inss           _inss
    #define name_deln           _deln
    #define name_del            _del
    #define name_cmpn           _cmpn
    #define name_cmpc           _cmpc
    #define name_cmps           _cmps
    #define name_vprintf        _vprintf
    #define name_printf         _printf
    #define name_findn          _findn
    #define name_findc          _findc
    #define name_finds          _finds
#endif

//
// Declarations
//
typedef struct cx_str_name {
    cx_str_alloc_field_;
    cx_str_cap_type_ len;
    cx_str_cap_type_ cap;
    char* data;
} cx_str_name;

#ifdef cx_str_allocator
    linkage cx_str_name type_name(name_init)(const CxAllocator* a);
    linkage cx_str_name type_name(name_initn)(const CxAllocator* a, const char* src, size_t n);
    linkage cx_str_name type_name(name_initc)(const CxAllocator* a, const char* src);
    linkage cx_str_name type_name(name_inits)(const CxAllocator* a, cx_str_name* src);
#else
    linkage cx_str_name type_name(name_init)(void);
    linkage cx_str_name type_name(name_initn)(const char* src, size_t n);
    linkage cx_str_name type_name(name_initc)(const char* src);
    linkage cx_str_name type_name(name_inits)(const cx_str_name* src);
#endif
linkage void type_name(name_set)(cx_str_name* s, const char* src);
linkage void type_name(name_free)(cx_str_name* s);
linkage void type_name(name_clear)(cx_str_name* s);
linkage cx_str_name type_name(name_clone)(const cx_str_name* s);
linkage cx_str_name type_name(name_reserve)(cx_str_name* s, size_t n);
linkage size_t type_name(name_cap)(const cx_str_name* s);
linkage size_t type_name(name_len)(const cx_str_name* s);
linkage const char* type_name(name_data)(const cx_str_name* s);
linkage bool type_name(name_empty)(const cx_str_name* s);
linkage void type_name(name_setcap)(cx_str_name* s, size_t cap);
linkage void type_name(name_setlen)(cx_str_name* s, size_t len);
linkage void type_name(name_setn)(cx_str_name* s, const char* src, size_t n);
linkage void type_name(name_setc)(cx_str_name* s, const char* src);
linkage void type_name(name_sets)(cx_str_name* s, const cx_str_name* src);
linkage void type_name(name_catn)(cx_str_name* s, const char* src, size_t n);
linkage void type_name(name_catc)(cx_str_name* s, const char* src);
linkage void type_name(name_cats)(cx_str_name* s, const cx_str_name* src);
linkage void type_name(name_insn)(cx_str_name* s, const char* src, size_t n, size_t idx);
linkage void type_name(name_insc)(cx_str_name* s, const char* src, size_t idx);
linkage void type_name(name_inss)(cx_str_name* s, const cx_str_name* src, size_t idx);
linkage void type_name(name_deln)(cx_str_name* s, size_t idx, size_t deln);
linkage void type_name(name_del)(cx_str_name* s, size_t idx);
linkage int  type_name(name_cmpn)(cx_str_name* s, const char* src, size_t n);
linkage int  type_name(name_cmpc)(cx_str_name* s, const char* src);
linkage int  type_name(name_cmps)(cx_str_name* s, const cx_str_name* src);
linkage void type_name(name_vprintf)(cx_str_name* s, const char *fmt, va_list ap);
linkage void type_name(name_printf)(cx_str_name* s, const char *fmt, ...);
linkage ptrdiff_t type_name(name_findn)(cx_str_name* s, const char *src, size_t n);
linkage ptrdiff_t type_name(name_findc)(cx_str_name* s, const char *src);
linkage ptrdiff_t type_name(name_finds)(cx_str_name* s, const cx_str_name* src);


//
// Implementations
//
#ifdef cx_str_implement

    cx_str_alloc_global_;

// Internal string reallocation function
static void type_name(_grow_)(cx_str_name* s, size_t addLen, size_t minCap) {

    size_t minLen = s->len + addLen;
    if (minLen > minCap) {
        minCap = minLen;
    }
    if (minCap <= s->cap) {
        return;
    }

    // Increase needed capacity to guarantee O(1) amortized
    if (minCap < 2 * s->cap) {
        minCap = 2 * s->cap;
    }
    else if (minCap < 4) {
        minCap = 4;
    }
    if (minCap + 1 > cx_str_max_cap_) {
        cx_str_error_handler("capacity exceeded");
        return;
    }

    // Allocates new capacity
    const size_t elemSize = sizeof(*(s->data));
    const size_t allocSize = elemSize * (minCap + 1);
    void* new = str_alloc(s, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    if (s->data) {
        memcpy(new, s->data, (s->len + 1) * elemSize);
        str_free(s, s->data, (s->len + 1) * elemSize);
    }
    s->data = new;
    s->cap = minCap;
}

#ifdef cx_str_allocator

    linkage cx_str_name type_name(name_init)(const CxAllocator* a) {

        return (cx_str_name) {.alloc = a};
    }

    linkage cx_str_name type_name(name_initn)(const CxAllocator* a, const char* src, size_t n) {

        cx_str_name s = {.alloc = a};
        type_name(_setn)(&s, src, n);
        return s;
    }

    linkage cx_str_name type_name(name_initc)(const CxAllocator* a, const char* src) {

        cx_str_name s = {.alloc = a};
        type_name(_setc)(&s, src);
        return s;
    }

    linkage cx_str_name type_name(name_inits)(const CxAllocator* a, cx_str_name* src) {

        cx_str_name s = {.alloc = a};
        type_name(_sets)(&s, src);
        return s;
    }

#else

    linkage cx_str_name type_name(name_init)(void) {
        if (type_name(_allocator) == NULL) {
            type_name(_allocator) = cxDefaultAllocator();
        }
        return (cx_str_name) {0};
    }

    linkage cx_str_name type_name(name_initn)(const char* src, size_t n) {

        cx_str_name s = type_name(name_init)();
        type_name(_setn)(&s, src, n);
        return s;
    }

    linkage cx_str_name type_name(name_initc)(const char* src) {

        cx_str_name s = type_name(name_init)();
        type_name(_setc)(&s, src);
        return s;
    }

    linkage cx_str_name type_name(name_inits)(const cx_str_name* src) {

        cx_str_name s = type_name(name_init)();
        type_name(_sets)(&s, src);
        return s;
    }
#endif


linkage void type_name(name_free)(cx_str_name* s) {

    str_free(s, s->data, s->cap);
    s->cap = 0;
    s->len = 0;
    s->data = NULL;
}

linkage void type_name(name_clear)(cx_str_name* s) {

    s->len = 0;
}

linkage cx_str_name type_name(name_clone)(const cx_str_name* s) {

    cx_str_name cloned = *s;
    cloned.data = str_alloc(s, s->cap + 1);
    memcpy(cloned.data, s->data, s->len + 1);
    return cloned;
}

linkage cx_str_name type_name(name_reserve)(cx_str_name* s, size_t n) {

    if (s->len + n < s->cap) {
        type_name(_grow_)(s, 0, s->len + n);
    }
}

linkage size_t type_name(name_cap)(const cx_str_name* s) {

    return s->cap;
}

linkage size_t type_name(name_len)(const cx_str_name* s) {

    return s->len;
}

linkage const char* type_name(name_data)(const cx_str_name* s) {

    return s->data;
}

linkage bool type_name(name_empty)(const cx_str_name* s) {

    return s->len == 0;
}

linkage void type_name(name_setcap)(cx_str_name* s, size_t cap) {

    type_name(_grow_)(s, 0, cap);
}

linkage void type_name(name_setlen)(cx_str_name* s, size_t len) {

    if (len > s->cap) {
        type_name(_grow_)(s, len, 0);
    }
    s->len = len;
    s->data[s->len] = 0;
}

linkage void type_name(name_setn)(cx_str_name* s, const char* src, size_t n) {

    if (n > s->cap) {
        type_name(_grow_)(s, 0, n);
    }
    memcpy(s->data, src, n);
    s->len = n;
    if (s->len) {
        s->data[s->len] = 0;
    }
}

linkage void type_name(name_setc)(cx_str_name* s, const char* src) {

    type_name(name_setn)(s, src, strlen(src));
}

linkage void type_name(name_sets)(cx_str_name* s, const cx_str_name* src) {

    type_name(name_setn)(s, src->data, src->len);
}

linkage void type_name(name_catn)(cx_str_name* s, const char* src, size_t n) {

    if (s->len + n > s->cap) {
        type_name(_grow_)(s, s->len + n, 0);
    }
    memcpy(s->data + s->len, src, n);
    s->len += n;
    if (s->len) {
        s->data[s->len] = 0;
    }
}

linkage void type_name(name_catc)(cx_str_name* s, const char* src) {

    type_name(name_catn)(s, src, strlen(src));
}

linkage void type_name(name_cats)(cx_str_name* s, const cx_str_name* src) {

    type_name(name_catn)(s, src->data, src->len);
}

linkage void type_name(name_insn)(cx_str_name* s, const char* src, size_t n, size_t idx) {

    if (idx > s->len) {
        cx_str_error_handler("invalid index");
        return;
    }
    if (s->len + n > s->cap) {
        type_name(_grow_)(s, s->len + n, 0);
    }
    memmove(s->data + idx + n, s->data + idx, s->len - idx);
    memcpy(s->data + idx, src, n);
    s->len += n;
    s->data[s->len] = 0;
}

linkage void type_name(name_insc)(cx_str_name* s, const char* src, size_t idx) {

    type_name(name_insn)(s, src, strlen(src), idx);
}

linkage void type_name(name_inss)(cx_str_name* s, const cx_str_name* src, size_t idx) {

    type_name(name_insn)(s, src->data, src->len, idx);
}

linkage void type_name(name_deln)(cx_str_name* s, size_t idx, size_t deln) {

    if (idx >= s->len) {
        return;
    }
    const size_t maxDel = s->len - idx;
    deln = deln > maxDel ? maxDel : deln;
    memmove(s->data + idx, s->data + idx + deln, s->len - idx - deln);
    s->len -= deln;
    if (s->len) {
        s->data[s->len] = 0;
    }
}

linkage void type_name(name_del)(cx_str_name* s, size_t idx) {

    type_name(name_deln)(s, idx, 1);
}

linkage int  type_name(name_cmpn)(cx_str_name* s, const char* src, size_t n) {

    if (s->len > n) {
        return 1;
    }
    if (s->len < n) {
        return -1;
    }
    return memcmp(s->data, src, n);
}

linkage int type_name(name_cmpc)(cx_str_name* s, const char* src) {

    return type_name(name_cmpn)(s, src, strlen(src));
}

linkage int type_name(name_cmps)(cx_str_name* s, const cx_str_name* src) {

    return type_name(name_cmpn)(s, src->data, src->len);
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
linkage void type_name(name_vprintf)(cx_str_name* s, const char *fmt, va_list ap) {
    va_list cpy;
    char  staticbuf[1024];
    char* buf = staticbuf;
    char* *t;
    size_t buflen = strlen(fmt)*2;
    int bufstrlen;

    // We try to start using a static buffer for speed.
    // If not possible we revert to heap allocation.
    if (buflen > sizeof(staticbuf)) {
        buf = str_alloc(s, buflen);
        if (buf == NULL) {
            return;
        }
    } else {
        buflen = sizeof(staticbuf);
    }

    // Alloc enough space for buffer and \0 after failing to
    // fit the string in the current buffer size.
    while(1) {
        va_copy(cpy,ap);
        bufstrlen = vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (bufstrlen < 0) {
            if (buf != staticbuf) {
                str_free(s, buf, buflen);
            }
            return;
        }
        if (((size_t)bufstrlen) >= buflen) {
            if (buf != staticbuf) {
                str_free(s, buf, buflen);
            }
            buflen = ((size_t)bufstrlen) + 1;
            buf = str_alloc(s, buflen);
            if (buf == NULL) {
                return;
            }
            continue;
        }
        break;
    }

    // Finally concat the obtained string to the SDS string and return it.
    type_name(name_catn)(s, buf, bufstrlen);
    if (buf != staticbuf) {
        str_free(s, buf, buflen);
    }
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
linkage void type_name(name_printf)(cx_str_name* s, const char *fmt, ...) {

    va_list ap;
    char *t;
    va_start(ap, fmt);
    type_name(_vprintf)(s, fmt, ap);
    va_end(ap);
}

linkage ptrdiff_t type_name(name_findn)(cx_str_name* s, const char *src, size_t n) {

    if (n > s->len) {
        return -1;
    }
    const size_t maxIndex = s->len - n;
    for (size_t i = 0; i <= maxIndex; i++) {
        if (memcmp(s->data + i, src, n) == 0) {
            return i;
        }
    }
    return -1;
}

linkage ptrdiff_t type_name(name_findc)(cx_str_name* s, const char *src) {

    return type_name(name_findn)(s, src, strlen(src));
}

linkage ptrdiff_t type_name(name_finds)(cx_str_name* s, const cx_str_name* src) {

    return type_name(name_findn)(s, src->data, src->len);
}


#endif // cx_str_implement

//
// Undefine all defined macros
//
#undef cx_str_name
#undef cx_str_cap8
#undef cx_str_cap16
#undef cx_str_cap32
#undef cx_str_camel_case
#undef cx_str_cap
#undef cx_str_allocator
#undef cx_str_error_handler
#undef cx_str_cap_type_
#undef cx_str_max_cap_
#undef cx_str_alloc_field_
#undef cx_str_alloc_global_
#undef str_alloc
#undef str_free

