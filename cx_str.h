/* Dynamic String Implementation

Example
-------

#define cx_str_name cxstr
#define cx_str_static
#define cx_str_inline
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

Configuration before including header file
------------------------------------------

Define the name of the string type (mandatory):
    #define cx_str_name <name>

Define the string maximum capacity (optional, default = 32):
    #define cx_str_cap <8|16|32>

Define error handler function (optional):
    #define cx_str_error_handler <func>

Sets if string uses custom allocator per instance
    #define cx_str_allocator

Sets if all string functions are prefixed with 'static'
    #define cx_str_static

Sets if all string functions are prefixed with 'inline'
    #define cx_str_inline

Sets to implement functions in this translation unit:
    #define cx_str_implement


API
---
Assuming: #define cx_str_name cxstr

Initialize string with custom allocator
    cxstr cxstr_init(const CxAllocator* a);

Initialize string with custom allocator from pointer and count
    cxstr cxstr_initn(const CxAllocator* a, const char* src, size_t n);

Initialize string with custom allocator from nul terminated C string
    cxstr cxstr_initc(const CxAllocator* a, const char* src);

Initialize string with custom allocator from another cxstr
    cxstr cxtr_inits(const CxAllocator* a, const cxstr* src);

Initialize string
    cxstr cxstr_init();

Initialize string from pointer and count
    cxstr cxstr_initn(const char* src, size_t n);

Initialize string from nul terminated C string
    cxstr cxstr_initc(const char* src);

Initialize string from another cxstr
    cxstr cxtr_inits(const cxstr* src);

Free string allocated memory
    void cxstr_free(cxstr* s);

Clear the strings setting is length to zero without deallocating its memory
    void cxstr_clear(cxstr* s);

*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "cx_alloc.h"

// String type name must be defined
#ifndef cx_str_name
    #error "cx_str_name not defined" 
#endif

// Auxiliary internal macros
#define cx_str_concat2_(a, b) a ## b
#define cx_str_concat1_(a, b) cx_str_concat2_(a, b)
#define cx_str_name_(name) cx_str_concat1_(cx_str_name, name)

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

// API attributes
#if defined(cx_str_static) && defined(cx_str_inline)
    #define cx_str_api_ static inline
#elif defined(cx_str_static)
    #define cx_str_api_ static
#elif defined(cx_str_inline)
    #define cx_str_api_ inline
#else
    #define cx_str_api_
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
        static const CxAllocator* cx_str_name_(_allocator) = NULL;
    #define cx_str_alloc_(s,n)\
        cx_alloc_alloc(cx_str_name_(_allocator),n)
    #define cx_str_free_(s,p,n)\
        cx_alloc_free(cx_str_name_(_allocator),p,n)
#endif

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
#define name_cpyn           _cpyn
#define name_cpy            _cpy
#define name_cpys           _cpys
#define name_ncat           _ncat
#define name_cat            _cat
#define name_cats           _cats
#define name_catcp          _catcp
#define name_insn           _insn
#define name_ins            _ins
#define name_inss           _inss
#define name_deln           _deln
#define name_del            _del
#define name_cmpn           _cmpn
#define name_cmp            _cmp
#define name_cmps           _cmps
#define name_icmp           _icmp
#define name_icmps          _icmps
#define name_vprintf        _vprintf
#define name_printf         _printf
#define name_findn          _findn
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
    cx_str_alloc_field_
    cx_str_cap_type_ len_;
    cx_str_cap_type_ cap_;
    char* data;
} cx_str_name;

#ifdef cx_str_allocator
    cx_str_api_ cx_str_name cx_str_name_(name_init)(const CxAllocator* a);
    cx_str_api_ cx_str_name cx_str_name_(name_initn)(const CxAllocator* a, const char* src, size_t n);
    cx_str_api_ cx_str_name cx_str_name_(name_initc)(const CxAllocator* a, const char* src);
    cx_str_api_ cx_str_name cx_str_name_(name_inits)(const CxAllocator* a, const cx_str_name* src);
#else
    cx_str_api_ cx_str_name cx_str_name_(name_init)(void);
    cx_str_api_ cx_str_name cx_str_name_(name_initn)(const char* src, size_t n);
    cx_str_api_ cx_str_name cx_str_name_(name_initc)(const char* src);
    cx_str_api_ cx_str_name cx_str_name_(name_inits)(const cx_str_name* src);
#endif
cx_str_api_ void cx_str_name_(name_free)(cx_str_name* s);
cx_str_api_ void cx_str_name_(name_clear)(cx_str_name* s);
cx_str_api_ void cx_str_name_(name_reserve)(cx_str_name* s, size_t n);
cx_str_api_ size_t cx_str_name_(name_cap)(const cx_str_name* s);
cx_str_api_ size_t cx_str_name_(name_len)(const cx_str_name* s);
cx_str_api_ size_t cx_str_name_(name_lencp)(const cx_str_name* s);
cx_str_api_ const char* cx_str_name_(name_data)(const cx_str_name* s);
cx_str_api_ bool cx_str_name_(name_empty)(const cx_str_name* s);
cx_str_api_ void cx_str_name_(name_setcap)(cx_str_name* s, size_t cap);
cx_str_api_ void cx_str_name_(name_cpyn)(cx_str_name* s, const char* src, size_t n);
cx_str_api_ void cx_str_name_(name_cpy)(cx_str_name* s, const char* src);
cx_str_api_ void cx_str_name_(name_cpys)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(name_ncat)(cx_str_name* s, const char* src, size_t n);
cx_str_api_ void cx_str_name_(name_cat)(cx_str_name* s, const char* src);
cx_str_api_ void cx_str_name_(name_cats)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(name_catcp)(cx_str_name* s, int32_t cp);
cx_str_api_ void cx_str_name_(name_insn)(cx_str_name* s, const char* src, size_t n, size_t idx);
cx_str_api_ void cx_str_name_(name_ins)(cx_str_name* s, const char* src, size_t idx);
cx_str_api_ void cx_str_name_(name_inss)(cx_str_name* s, const cx_str_name* src, size_t idx);
cx_str_api_ void cx_str_name_(name_deln)(cx_str_name* s, size_t idx, size_t deln);
cx_str_api_ void cx_str_name_(name_del)(cx_str_name* s, size_t idx);
cx_str_api_ int  cx_str_name_(name_cmpn)(cx_str_name* s, const char* src, size_t n);
cx_str_api_ int  cx_str_name_(name_cmp)(cx_str_name* s, const char* src);
cx_str_api_ int  cx_str_name_(name_cmps)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ int  cx_str_name_(name_icmp)(cx_str_name* s, const char* src);
cx_str_api_ int  cx_str_name_(name_icmps)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(name_vprintf)(cx_str_name* s, const char *fmt, va_list ap);
cx_str_api_ void cx_str_name_(name_printf)(cx_str_name* s, const char *fmt, ...);
cx_str_api_ ptrdiff_t cx_str_name_(name_findn)(cx_str_name* s, size_t start, const char *src, size_t n);
cx_str_api_ ptrdiff_t cx_str_name_(name_find)(cx_str_name* s, const char *src);
cx_str_api_ ptrdiff_t cx_str_name_(name_finds)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ ptrdiff_t cx_str_name_(name_findcp)(cx_str_name* s, int32_t cp);
cx_str_api_ ptrdiff_t cx_str_name_(name_ifind)(cx_str_name* s, const char *src);
cx_str_api_ ptrdiff_t cx_str_name_(name_ifinds)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(name_substr)(const cx_str_name* s, size_t start, size_t len, cx_str_name* dst);
cx_str_api_ void cx_str_name_(name_replace)(cx_str_name* s, const char* old, const char* new, size_t count);
cx_str_api_ bool cx_str_name_(name_validu8)(const cx_str_name* s);
cx_str_api_ void cx_str_name_(name_upper)(cx_str_name* s);
cx_str_api_ void cx_str_name_(name_lower)(cx_str_name* s);
cx_str_api_ char* cx_str_name_(name_ncp)(cx_str_name* s, char* iter, int32_t* cp);
cx_str_api_ void cx_str_name_(name_ltrim)(cx_str_name* s, const char* cset);
cx_str_api_ void cx_str_name_(name_rtrim)(cx_str_name* s, const char* cset);


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
static void cx_str_name_(_grow_)(cx_str_name* s, size_t addLen, size_t minCap) {

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

    cx_str_api_ cx_str_name cx_str_name_(name_init)(const CxAllocator* a) {

        return (cx_str_name) {.alloc = a};
    }

    cx_str_api_ cx_str_name cx_str_name_(name_initn)(const CxAllocator* a, const char* src, size_t n) {

        cx_str_name s = {.alloc = a};
        cx_str_name_(_cpyn)(&s, src, n);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(name_initc)(const CxAllocator* a, const char* src) {

        cx_str_name s = {.alloc = a};
        cx_str_name_(_cpy)(&s, src);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(name_inits)(const CxAllocator* a, const cx_str_name* src) {

        cx_str_name s = {.alloc = a};
        cx_str_name_(_cpys)(&s, src);
        return s;
    }

#else

    cx_str_api_ cx_str_name cx_str_name_(name_init)(void) {
        if (cx_str_name_(_allocator) == NULL) {
            cx_str_name_(_allocator) = cxDefaultAllocator();
        }
        return (cx_str_name) {0};
    }

    cx_str_api_ cx_str_name cx_str_name_(name_initn)(const char* src, size_t n) {

        cx_str_name s = cx_str_name_(name_init)();
        cx_str_name_(_cpyn)(&s, src, n);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(name_initc)(const char* src) {

        cx_str_name s = cx_str_name_(name_init)();
        cx_str_name_(_cpy)(&s, src);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(name_inits)(const cx_str_name* src) {

        cx_str_name s = cx_str_name_(name_init)();
        cx_str_name_(_cpys)(&s, src);
        return s;
    }
#endif


cx_str_api_ void cx_str_name_(name_free)(cx_str_name* s) {

    cx_str_free_(s, s->data, s->cap_);
    s->cap_ = 0;
    s->len_ = 0;
    s->data = NULL;
}

cx_str_api_ void cx_str_name_(name_clear)(cx_str_name* s) {

    s->len_ = 0;
}

cx_str_api_ void cx_str_name_(name_reserve)(cx_str_name* s, size_t n) {

    if (s->len_ + n < s->cap_) {
        cx_str_name_(_grow_)(s, 0, s->len_ + n);
    }
}

cx_str_api_ size_t cx_str_name_(name_cap)(const cx_str_name* s) {

    return s->cap_;
}

cx_str_api_ size_t cx_str_name_(name_len)(const cx_str_name* s) {

    return s->len_;
}

cx_str_api_ size_t cx_str_name_(name_lencp)(const cx_str_name* s) {

    return utf8len(s->data);
}

cx_str_api_ const char* cx_str_name_(name_data)(const cx_str_name* s) {

    return s->data;
}

cx_str_api_ bool cx_str_name_(name_empty)(const cx_str_name* s) {

    return s->len_ == 0;
}

cx_str_api_ void cx_str_name_(name_setcap)(cx_str_name* s, size_t cap) {

    cx_str_name_(_grow_)(s, 0, cap);
}

cx_str_api_ void cx_str_name_(name_cpyn)(cx_str_name* s, const char* src, size_t n) {

    if (src == NULL) {
        return;
    }
    if (n > s->cap_) {
        cx_str_name_(_grow_)(s, 0, n);
    }
    memcpy(s->data, src, n);
    s->len_ = n;
    if (s->len_) {
        s->data[s->len_] = 0;
    }
}

cx_str_api_ void cx_str_name_(name_cpy)(cx_str_name* s, const char* src) {

    if (src == NULL) {
        return;
    }
    cx_str_name_(name_cpyn)(s, src, strlen(src));
}

cx_str_api_ void cx_str_name_(name_cpys)(cx_str_name* s, const cx_str_name* src) {

    cx_str_name_(name_cpyn)(s, src->data, src->len_);
}

cx_str_api_ void cx_str_name_(name_ncat)(cx_str_name* s, const char* src, size_t n) {

    if (s->len_ + n > s->cap_) {
        cx_str_name_(_grow_)(s, s->len_ + n, 0);
    }
    memcpy(s->data + s->len_, src, n);
    s->len_ += n;
    if (s->len_) {
        s->data[s->len_] = 0;
    }
}

cx_str_api_ void cx_str_name_(name_cat)(cx_str_name* s, const char* src) {

    cx_str_name_(name_ncat)(s, src, strlen(src));
}

cx_str_api_ void cx_str_name_(name_cats)(cx_str_name* s, const cx_str_name* src) {

    cx_str_name_(name_ncat)(s, src->data, src->len_);
}

cx_str_api_ void cx_str_name_(name_catcp)(cx_str_name* s, int32_t cp) {

    const size_t size = utf8codepointsize(cp);
    if (s->len_ + size > s->cap_) {
        cx_str_name_(_grow_)(s, s->len_ + size, 0);
    }
    utf8catcodepoint(s->data + s->len_, cp, size);
    s->len_ += size;
    s->data[s->len_] = 0;
}

cx_str_api_ void cx_str_name_(name_insn)(cx_str_name* s, const char* src, size_t n, size_t idx) {

    if (idx > s->len_) {
        cx_str_error_handler("invalid index");
        return;
    }
    if (s->len_ + n > s->cap_) {
        cx_str_name_(_grow_)(s, n, 0);
    }
    memmove(s->data + idx + n, s->data + idx, s->len_ - idx);
    memcpy(s->data + idx, src, n);
    s->len_ += n;
    s->data[s->len_] = 0;
}

cx_str_api_ void cx_str_name_(name_ins)(cx_str_name* s, const char* src, size_t idx) {

    cx_str_name_(name_insn)(s, src, strlen(src), idx);
}

cx_str_api_ void cx_str_name_(name_inss)(cx_str_name* s, const cx_str_name* src, size_t idx) {

    cx_str_name_(name_insn)(s, src->data, src->len_, idx);
}

cx_str_api_ void cx_str_name_(name_deln)(cx_str_name* s, size_t idx, size_t deln) {

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

cx_str_api_ void cx_str_name_(name_del)(cx_str_name* s, size_t idx) {

    cx_str_name_(name_deln)(s, idx, 1);
}

cx_str_api_ int  cx_str_name_(name_cmpn)(cx_str_name* s, const char* src, size_t n) {

    if (s->len_ > n) {
        return 1;
    }
    if (s->len_ < n) {
        return -1;
    }
    return memcmp(s->data, src, n);
}

cx_str_api_ int cx_str_name_(name_cmp)(cx_str_name* s, const char* src) {

    return cx_str_name_(name_cmpn)(s, src, strlen(src));
}

cx_str_api_ int cx_str_name_(name_cmps)(cx_str_name* s, const cx_str_name* src) {

    return cx_str_name_(name_cmpn)(s, src->data, src->len_);
}

cx_str_api_ int cx_str_name_(name_icmp)(cx_str_name* s, const char* src) {

    return utf8casecmp(s->data, src);
}

cx_str_api_ int cx_str_name_(name_icmps)(cx_str_name* s, const cx_str_name* src) {

    if (s->len_ > src->len_) {
        return 1;
    }
    if (s->len_ < src->len_) {
        return -1;
    }
    return utf8casecmp(s->data, src->data);
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
cx_str_api_ void cx_str_name_(name_vprintf)(cx_str_name* s, const char *fmt, va_list ap) {
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
    cx_str_name_(name_ncat)(s, buf, bufstrlen);
    if (buf != staticbuf) {
        cx_str_free_(s, buf, buflen);
    }
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
cx_str_api_ void cx_str_name_(name_printf)(cx_str_name* s, const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    cx_str_name_(_vprintf)(s, fmt, ap);
    va_end(ap);
}

cx_str_api_ ptrdiff_t cx_str_name_(name_findn)(cx_str_name* s, size_t start, const char *src, size_t n) {

    if (start >= s->len_) {
        return -1;
    }
    if (n > s->len_) {
        return -1;
    }
    const size_t maxIndex = s->len_ - n;
    for (size_t i = start; i <= maxIndex; i++) {
        if (memcmp(s->data + i, src, n) == 0) {
            return i;
        }
    }
    return -1;
}

cx_str_api_ ptrdiff_t cx_str_name_(name_find)(cx_str_name* s, const char *src) {

    return cx_str_name_(name_findn)(s, 0, src, strlen(src));
}

cx_str_api_ ptrdiff_t cx_str_name_(name_finds)(cx_str_name* s, const cx_str_name* src) {

    return cx_str_name_(name_findn)(s, 0, src->data, src->len_);
}

cx_str_api_ ptrdiff_t cx_str_name_(name_findcp)(cx_str_name* s, int32_t cp) {

    char* n = utf8chr(s->data, cp);
    if (n == NULL) {
        return -1;
    }
    return n - s->data;
}

cx_str_api_ ptrdiff_t cx_str_name_(name_ifind)(cx_str_name* s, const char *src) {

    char *n = utf8casestr(s->data, src);
    if (n == NULL) {
        return -1;
    }
    return n - s->data;
}

cx_str_api_ ptrdiff_t cx_str_name_(name_ifinds)(cx_str_name* s, const cx_str_name* src) {

    if (s->len_ < src->len_) {
        return -1;
    }
    return cx_str_name_(name_ifind)(s, src->data);
}

cx_str_api_ void cx_str_name_(name_substr)(const cx_str_name* s, size_t start, size_t len, cx_str_name* dst) {

     if (start >= s->len_) {
         dst->len_ = 0;
         return ;
     }
    const size_t maxSize = s->len_ - start;
    len = len > maxSize ? maxSize : len;
    cx_str_name_(name_cpyn)(dst, s->data + start, len);
}

cx_str_api_ void cx_str_name_(name_replace)(cx_str_name* s, const char* old, const char* new, size_t count) {

    const size_t olen = strlen(old);
    const size_t nlen = strlen(new);
    size_t start = 0;
    while (1) {
        ptrdiff_t pos = cx_str_name_(name_findn)((cx_str_name*)s, start, old, olen);
        if (pos < 0) {
            break;
        }
        if (olen > nlen) {
            cx_str_name_(name_deln)(s, pos, olen - nlen);
        } else if (olen < nlen) {
            const size_t addLen = nlen - olen;
            if (s->len_ + addLen > s->cap_) {
                cx_str_name_(_grow_)(s, addLen, 0);
            }
            memmove(s->data + pos + olen + addLen, s->data + pos + olen, s->len_ - pos);
            s->len_ += addLen;
            s->data[s->len_] = 0;
        }
        memcpy(s->data + pos, new, nlen);
        if (count) {
            count--;
            if (count == 0) {
                break;
            }
        }
        start += pos + nlen;
    }
}


cx_str_api_ bool cx_str_name_(name_validu8)(const cx_str_name* s) {

    return utf8valid(s->data) == 0;
}

cx_str_api_ void cx_str_name_(name_upper)(cx_str_name* s) {

    utf8upr(s->data);
}

cx_str_api_ void cx_str_name_(name_lower)(cx_str_name* s) {

    utf8lwr(s->data);
}

cx_str_api_ char* cx_str_name_(name_ncp)(cx_str_name* s, char* iter, int32_t* cp) {

    if (iter < s->data || iter >= (s->data + s->len_)) {
        return NULL;
    }
    return utf8codepoint(iter, cp);
}

cx_str_api_ void cx_str_name_(name_ltrim)(cx_str_name* s, const char* cset)  {

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
    cx_str_name_(name_deln)(s, 0, deln);
}

cx_str_api_ void cx_str_name_(name_rtrim)(cx_str_name* s, const char* cset) {

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
    cx_str_name_(name_deln)(s, s->len_ - deln, deln);
}

#endif // cx_str_implement

// Undefine config  macros
#undef cx_str_name
#undef cx_str_static
#undef cx_str_inline
#undef cx_str_cap
#undef cx_str_allocator
#undef cx_str_error_handler
#undef cx_str_implement

// Undefine internal macros
#undef cx_str_concat2_
#undef cx_str_concat1_
#undef cx_str_name_
#undef cx_str_cap_type_
#undef cx_str_cap8_
#undef cx_str_cap16_
#undef cx_str_cap32_
#undef cx_str_max_cap_
#undef cx_str_alloc_field_
#undef cx_str_alloc_global_
#undef cx_str_alloc_
#undef cx_str_free_
#undef cx_str_api_


