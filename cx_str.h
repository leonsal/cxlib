/********************************************

Dynamic String Implementation


Example
-------

#define cx_str_name cxstr
#define cx_str_implement
#include "cx_str.h"


int main() {

    cxstr s1 = cxstr_initc("hello");
    cxstr_cat(&s1, " world");
    printf("%s\n", str.data); 
    cxstr_free(&s1);

    cxstr s2 = cxstr_inits(&s1);
    cxstr_printf("counter:%d\n", 42);
    printf("%s\n", s2.data); 
    cxstr_free(&s2);

    return 0;
}

API documentation:
------------------

Configuration defines:
-   cx_str_name <name>
    Sets the name of the string type

-   cx_str_cap <8|16|32> (optional, default=32)
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

-   cx_str_thread (TODO thread safe)


********************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "cx_alloc.h"

#ifndef cx_str_name
    #error "cx_str_name not defined" 
#endif


// String capacity in number of bits
#define cx_str_cap8_     8
#define cx_str_cap16_    16
#define cx_str_cap32_    32

// Default capacity
#ifndef cx_str_cap
    #define cx_str_cap  cx_str_cap32_
#endif
#if cx_str_cap == cx_str_cap8_
    #define cx_str_cap_type_ uint8_t
    #define cx_str_max_cap_  (UINT8_MAX)
#elif cx_str_cap == cx_str_cap16_
    #define cx_str_cap_type_ uint16_t
    #define cx_str_max_cap_  (UINT16_MAX)
#elif cx_str_cap == cx_str_cap32_
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
    #define cx_str_alloc_(s,n)\
        cx_alloc_alloc(s->alloc, n)
    #define cx_str_free_(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_str_alloc_field_
    #define cx_str_alloc_global_\
        linkage const CxAllocator* type_name(_allocator) = NULL;
    #define cx_str_alloc_(s,n)\
        cx_alloc_alloc(type_name(_allocator),n)
    #define cx_str_free_(s,p,n)\
        cx_alloc_free(type_name(_allocator),p,n)
#endif

// Auxiliary internal macros
#define concat2_(a, b) a ## b
#define concat1_(a, b) concat2_(a, b)
#define type_name(name) concat1_(cx_str_name, name)


//
// Function names
//
#define name_init           _init
#define name_initn          _initn
#define name_initc          _initc
#define name_inits          _inits
#define name_free           _free
#define name_clear          _clear
#define name_reserve        _reserve
#define name_cap            _cap
#define name_len            _len
#define name_lencp           _lencp
#define name_data           _data
#define name_empty          _empty
#define name_setcap         _setcap
#define name_ncpy           _ncpy
#define name_cpy            _cpy
#define name_cpys           _cpys
#define name_ncat           _ncat
#define name_cat            _cat
#define name_cats           _cats
#define name_catcp          _catcp
#define name_nins           _nins
#define name_ins            _ins
#define name_inss           _inss
#define name_ndel           _ndel
#define name_del            _del
#define name_ncmp           _ncmp
#define name_cmp            _cmp
#define name_cmps           _cmps
#define name_icmp           _icmp
#define name_icmps          _icmps
#define name_vprintf        _vprintf
#define name_printf         _printf
#define name_nfind          _nfind
#define name_find           _find
#define name_finds          _finds
#define name_findcp         _findcp
#define name_ifind          _ifind
#define name_ifinds         _ifinds
#define name_substr         _substr
#define name_replace        _replace
#define name_validu8        _validu8
#define name_upper          _upper
#define name_lower          _lower
#define name_ncp            _ncp
#define name_ltrim          _ltrim
#define name_rtrim          _rtrim

//
// Declarations
//
typedef struct cx_str_name {
    cx_str_alloc_field_;
    cx_str_cap_type_ len_;
    cx_str_cap_type_ cap_;
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
linkage void type_name(name_reserve)(cx_str_name* s, size_t n);
linkage size_t type_name(name_cap)(const cx_str_name* s);
linkage size_t type_name(name_len)(const cx_str_name* s);
linkage size_t type_name(name_lencp)(const cx_str_name* s);
linkage const char* type_name(name_data)(const cx_str_name* s);
linkage bool type_name(name_empty)(const cx_str_name* s);
linkage void type_name(name_setcap)(cx_str_name* s, size_t cap);
linkage void type_name(name_ncpy)(cx_str_name* s, const char* src, size_t n);
linkage void type_name(name_cpy)(cx_str_name* s, const char* src);
linkage void type_name(name_cpys)(cx_str_name* s, const cx_str_name* src);
linkage void type_name(name_ncat)(cx_str_name* s, const char* src, size_t n);
linkage void type_name(name_cat)(cx_str_name* s, const char* src);
linkage void type_name(name_cats)(cx_str_name* s, const cx_str_name* src);
linkage void type_name(name_catcp)(cx_str_name* s, int32_t cp);
linkage void type_name(name_nins)(cx_str_name* s, const char* src, size_t n, size_t idx);
linkage void type_name(name_ins)(cx_str_name* s, const char* src, size_t idx);
linkage void type_name(name_inss)(cx_str_name* s, const cx_str_name* src, size_t idx);
linkage void type_name(name_ndel)(cx_str_name* s, size_t idx, size_t deln);
linkage void type_name(name_del)(cx_str_name* s, size_t idx);
linkage int  type_name(name_ncmp)(cx_str_name* s, const char* src, size_t n);
linkage int  type_name(name_cmp)(cx_str_name* s, const char* src);
linkage int  type_name(name_cmps)(cx_str_name* s, const cx_str_name* src);
linkage int  type_name(name_icmp)(cx_str_name* s, const char* src);
linkage int  type_name(name_icmps)(cx_str_name* s, const cx_str_name* src);
linkage void type_name(name_vprintf)(cx_str_name* s, const char *fmt, va_list ap);
linkage void type_name(name_printf)(cx_str_name* s, const char *fmt, ...);
linkage ptrdiff_t type_name(name_nfind)(cx_str_name* s, const char *src, size_t n);
linkage ptrdiff_t type_name(name_find)(cx_str_name* s, const char *src);
linkage ptrdiff_t type_name(name_finds)(cx_str_name* s, const cx_str_name* src);
linkage ptrdiff_t type_name(name_findcp)(cx_str_name* s, int32_t cp);
linkage ptrdiff_t type_name(name_ifind)(cx_str_name* s, const char *src);
linkage ptrdiff_t type_name(name_ifinds)(cx_str_name* s, const cx_str_name* src);
linkage void type_name(name_substr)(const cx_str_name* s, size_t start, size_t len, cx_str_name* dst);
linkage void type_name(name_replace)(cx_str_name* s, const char* old, const char* new, size_t count);
linkage bool type_name(name_validu8)(const cx_str_name* s);
linkage void type_name(name_upper)(cx_str_name* s);
linkage void type_name(name_lower)(cx_str_name* s);
linkage char* type_name(name_ncp)(cx_str_name* s, char* iter, int32_t* cp);
linkage void type_name(name_ltrim)(cx_str_name* s, const char* cset);
linkage void type_name(name_rtrim)(cx_str_name* s, const char* cset);


//
// Implementations
//
#ifdef cx_str_implement

    cx_str_alloc_global_;
    extern int utf8casecmp(const char* src1, const char* src2);
    extern size_t utf8len(const char* str);
    extern char* utf8valid(const char* str);
    extern void utf8upr(char* str);
    extern void utf8lwr(char* str);
    extern char* utf8codepoint(char* str, int32_t* out_codepoint);
    extern char* utf8casestr(const char* haystack, const char* needle);
    extern char* utf8chr(const char* src, int32_t chr);
    extern size_t utf8codepointsize(int32_t chr);
    extern char* utf8catcodepoint(char* str, int32_t chr, size_t n);

// Internal string reallocation function
static void type_name(_grow_)(cx_str_name* s, size_t addLen, size_t minCap) {

    size_t minLen = s->len_ + addLen;
    if (minLen > minCap) {
        minCap = minLen;
    }
    if (minCap <= s->cap_) {
        return;
    }

    // Increase needed capacity to guarantee O(1) amortized
    if (minCap < 2 * s->cap_) {
        minCap = 2 * s->cap_;
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
    void* new = cx_str_alloc_(s, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    if (s->data) {
        memcpy(new, s->data, (s->len_ + 1) * elemSize);
        cx_str_free_(s, s->data, (s->len_ + 1) * elemSize);
    }
    s->data = new;
    s->cap_ = minCap;
}

#ifdef cx_str_allocator

    linkage cx_str_name type_name(name_init)(const CxAllocator* a) {

        return (cx_str_name) {.alloc = a};
    }

    linkage cx_str_name type_name(name_initn)(const CxAllocator* a, const char* src, size_t n) {

        cx_str_name s = {.alloc = a};
        type_name(_ncpy)(&s, src, n);
        return s;
    }

    linkage cx_str_name type_name(name_initc)(const CxAllocator* a, const char* src) {

        cx_str_name s = {.alloc = a};
        type_name(_cpy)(&s, src);
        return s;
    }

    linkage cx_str_name type_name(name_inits)(const CxAllocator* a, cx_str_name* src) {

        cx_str_name s = {.alloc = a};
        type_name(_cpys)(&s, src);
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
        type_name(_ncpy)(&s, src, n);
        return s;
    }

    linkage cx_str_name type_name(name_initc)(const char* src) {

        cx_str_name s = type_name(name_init)();
        type_name(_cpy)(&s, src);
        return s;
    }

    linkage cx_str_name type_name(name_inits)(const cx_str_name* src) {

        cx_str_name s = type_name(name_init)();
        type_name(_cpys)(&s, src);
        return s;
    }
#endif


linkage void type_name(name_free)(cx_str_name* s) {

    cx_str_free_(s, s->data, s->cap_);
    s->cap_ = 0;
    s->len_ = 0;
    s->data = NULL;
}

linkage void type_name(name_clear)(cx_str_name* s) {

    s->len_ = 0;
}

linkage void type_name(name_reserve)(cx_str_name* s, size_t n) {

    if (s->len_ + n < s->cap_) {
        type_name(_grow_)(s, 0, s->len_ + n);
    }
}

linkage size_t type_name(name_cap)(const cx_str_name* s) {

    return s->cap_;
}

linkage size_t type_name(name_len)(const cx_str_name* s) {

    return s->len_;
}

linkage size_t type_name(name_lencp)(const cx_str_name* s) {

    return utf8len(s->data);
}

linkage const char* type_name(name_data)(const cx_str_name* s) {

    return s->data;
}

linkage bool type_name(name_empty)(const cx_str_name* s) {

    return s->len_ == 0;
}

linkage void type_name(name_setcap)(cx_str_name* s, size_t cap) {

    type_name(_grow_)(s, 0, cap);
}

linkage void type_name(name_ncpy)(cx_str_name* s, const char* src, size_t n) {

    if (src == NULL) {
        return;
    }
    if (n > s->cap_) {
        type_name(_grow_)(s, 0, n);
    }
    memcpy(s->data, src, n);
    s->len_ = n;
    if (s->len_) {
        s->data[s->len_] = 0;
    }
}

linkage void type_name(name_cpy)(cx_str_name* s, const char* src) {

    if (src == NULL) {
        return;
    }
    type_name(name_ncpy)(s, src, strlen(src));
}

linkage void type_name(name_cpys)(cx_str_name* s, const cx_str_name* src) {

    type_name(name_ncpy)(s, src->data, src->len_);
}

linkage void type_name(name_ncat)(cx_str_name* s, const char* src, size_t n) {

    if (s->len_ + n > s->cap_) {
        type_name(_grow_)(s, s->len_ + n, 0);
    }
    memcpy(s->data + s->len_, src, n);
    s->len_ += n;
    if (s->len_) {
        s->data[s->len_] = 0;
    }
}

linkage void type_name(name_cat)(cx_str_name* s, const char* src) {

    type_name(name_ncat)(s, src, strlen(src));
}

linkage void type_name(name_cats)(cx_str_name* s, const cx_str_name* src) {

    type_name(name_ncat)(s, src->data, src->len_);
}

linkage void type_name(name_catcp)(cx_str_name* s, int32_t cp) {

    const size_t size = utf8codepointsize(cp);
    if (s->len_ + size > s->cap_) {
        type_name(_grow_)(s, s->len_ + size, 0);
    }
    utf8catcodepoint(s->data + s->len_, cp, size);
    s->len_ += size;
    s->data[s->len_] = 0;
}

linkage void type_name(name_nins)(cx_str_name* s, const char* src, size_t n, size_t idx) {

    if (idx > s->len_) {
        cx_str_error_handler("invalid index");
        return;
    }
    if (s->len_ + n > s->cap_) {
        type_name(_grow_)(s, s->len_ + n, 0);
    }
    memmove(s->data + idx + n, s->data + idx, s->len_ - idx);
    memcpy(s->data + idx, src, n);
    s->len_ += n;
    s->data[s->len_] = 0;
}

linkage void type_name(name_ins)(cx_str_name* s, const char* src, size_t idx) {

    type_name(name_nins)(s, src, strlen(src), idx);
}

linkage void type_name(name_inss)(cx_str_name* s, const cx_str_name* src, size_t idx) {

    type_name(name_nins)(s, src->data, src->len_, idx);
}

linkage void type_name(name_ndel)(cx_str_name* s, size_t idx, size_t deln) {

    if (idx >= s->len_) {
        cx_str_error_handler("invalid index");
        return;
    }
    const size_t maxDel = s->len_ - idx;
    deln = deln > maxDel ? maxDel : deln;
    memmove(s->data + idx, s->data + idx + deln, s->len_ - idx - deln);
    s->len_ -= deln;
    if (s->len_) {
        s->data[s->len_] = 0;
    }
}

linkage void type_name(name_del)(cx_str_name* s, size_t idx) {

    type_name(name_ndel)(s, idx, 1);
}

linkage int  type_name(name_ncmp)(cx_str_name* s, const char* src, size_t n) {

    if (s->len_ > n) {
        return 1;
    }
    if (s->len_ < n) {
        return -1;
    }
    return memcmp(s->data, src, n);
}

linkage int type_name(name_cmp)(cx_str_name* s, const char* src) {

    return type_name(name_ncmp)(s, src, strlen(src));
}

linkage int type_name(name_cmps)(cx_str_name* s, const cx_str_name* src) {

    return type_name(name_ncmp)(s, src->data, src->len_);
}

linkage int type_name(name_icmp)(cx_str_name* s, const char* src) {

    return utf8casecmp(s->data, src);
}

linkage int type_name(name_icmps)(cx_str_name* s, const cx_str_name* src) {

    if (s->len_ > src->len_) {
        return 1;
    }
    if (s->len_ < src->len_) {
        return -1;
    }
    return utf8casecmp(s->data, src->data);
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
linkage void type_name(name_vprintf)(cx_str_name* s, const char *fmt, va_list ap) {
    va_list cpy;
    char  staticbuf[1024];
    char* buf = staticbuf;
    size_t buflen = strlen(fmt)*2;
    int bufstrlen;

    // We try to start using a static buffer for speed.
    // If not possible we revert to heap allocation.
    if (buflen > sizeof(staticbuf)) {
        buf = cx_str_alloc_(s, buflen);
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
                cx_str_free_(s, buf, buflen);
            }
            return;
        }
        if (((size_t)bufstrlen) >= buflen) {
            if (buf != staticbuf) {
                cx_str_free_(s, buf, buflen);
            }
            buflen = ((size_t)bufstrlen) + 1;
            buf = cx_str_alloc_(s, buflen);
            if (buf == NULL) {
                return;
            }
            continue;
        }
        break;
    }

    // Finally concat the obtained string to the SDS string and return it.
    type_name(name_ncat)(s, buf, bufstrlen);
    if (buf != staticbuf) {
        cx_str_free_(s, buf, buflen);
    }
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
linkage void type_name(name_printf)(cx_str_name* s, const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    type_name(_vprintf)(s, fmt, ap);
    va_end(ap);
}

linkage ptrdiff_t type_name(name_nfind)(cx_str_name* s, const char *src, size_t n) {

    if (n > s->len_) {
        return -1;
    }
    const size_t maxIndex = s->len_ - n;
    for (size_t i = 0; i <= maxIndex; i++) {
        if (memcmp(s->data + i, src, n) == 0) {
            return i;
        }
    }
    return -1;
}

linkage ptrdiff_t type_name(name_find)(cx_str_name* s, const char *src) {

    return type_name(name_nfind)(s, src, strlen(src));
}

linkage ptrdiff_t type_name(name_finds)(cx_str_name* s, const cx_str_name* src) {

    return type_name(name_nfind)(s, src->data, src->len_);
}

linkage ptrdiff_t type_name(name_findcp)(cx_str_name* s, int32_t cp) {

    char* n = utf8chr(s->data, cp);
    if (n == NULL) {
        return -1;
    }
    return n - s->data;
}

linkage ptrdiff_t type_name(name_ifind)(cx_str_name* s, const char *src) {

    char *n = utf8casestr(s->data, src);
    if (n == NULL) {
        return -1;
    }
    return n - s->data;
}

linkage ptrdiff_t type_name(name_ifinds)(cx_str_name* s, const cx_str_name* src) {

    if (s->len_ < src->len_) {
        return -1;
    }
    return type_name(name_ifind)(s, src->data);
}

linkage void type_name(name_substr)(const cx_str_name* s, size_t start, size_t len, cx_str_name* dst) {

     if (start >= s->len_) {
         dst->len_ = 0;
         return ;
     }
    const size_t maxSize = s->len_ - start;
    len = len > maxSize ? maxSize : len;
    type_name(name_ncpy)(dst, s->data + start, len);
}

linkage void type_name(name_replace)(cx_str_name* s, const char* old, const char* new, size_t count) {

    const size_t olen = strlen(old);
    const size_t nlen = strlen(new);
    while (1) {
        ptrdiff_t pos = type_name(name_find)((cx_str_name*)s, old);
        if (pos < 0) {
            break;
        }
        if (olen > nlen) {
            type_name(name_ndel)(s, pos, olen - nlen);
            memcpy(s->data + pos, new, nlen);
        } else if (olen < nlen) {
            type_name(name_nins)(s, new, nlen, pos);
        }
        if (count) {
            count--;
            if (count == 0) {
                break;
            }
        }
    }
}


linkage bool type_name(name_validu8)(const cx_str_name* s) {

    return utf8valid(s->data) == 0;
}

linkage void type_name(name_upper)(cx_str_name* s) {

    utf8upr(s->data);
}

linkage void type_name(name_lower)(cx_str_name* s) {

    utf8lwr(s->data);
}

linkage char* type_name(name_ncp)(cx_str_name* s, char* iter, int32_t* cp) {

    if (iter < s->data || iter >= (s->data + s->len_)) {
        return NULL;
    }
    return utf8codepoint(iter, cp);
}

linkage void type_name(name_ltrim)(cx_str_name* s, const char* cset)  {

    if (s->len_ == 0) {
        return;
    }
    char* data = s->data;
    int32_t codepoint;
    size_t deln = 0;
    while (1) {
        data = utf8codepoint(data, &codepoint);
        if (data >= s->data + s->len_) {
            break;
        }
        if (utf8chr(cset, codepoint) == NULL) {
            break;
        }
        deln += utf8codepointsize(codepoint);
    }
    if (deln == 0) {
        return;
    }
    type_name(name_ndel)(s, 0, deln);
}

linkage void type_name(name_rtrim)(cx_str_name* s, const char* cset) {

    if (s->len_ == 0) {
        return;
    }
    char* data = s->data + s->len_ - 1;
    int32_t codepoint;
    size_t deln = 0;
    while (data >= s->data) {
        // Looks for start of last codepoint
        // leading bytes: 0XXXXXXX | 11XXXXXX
        if (!((*data & 0x80) == 0 || (*data & 0x40) != 0)) {
            data--;
            continue;
        }
        utf8codepoint(data, &codepoint);
        if (utf8chr(cset, codepoint) == NULL) {
            break;
        }
        deln += utf8codepointsize(codepoint);
        data--;
    }
    if (deln == 0) {
        return;
    }
    type_name(name_ndel)(s, s->len_ - deln, deln);
}

#endif // cx_str_implement

//
// Undefine config  macros
//
#undef cx_str_name
#undef cx_str_camel_case
#undef cx_str_cap
#undef cx_str_allocator
#undef cx_str_error_handler

//
// Undefine internal macros
//
#undef cx_str_cap_type_
#undef cx_str_cap8_
#undef cx_str_cap16_
#undef cx_str_cap32_
#undef cx_str_max_cap_
#undef cx_str_alloc_field_
#undef cx_str_alloc_global_
#undef cx_str_alloc_
#undef cx_str_free_

