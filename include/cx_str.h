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

String configuration defines
----------------------------

Define the name of the string type (mandatory):
    #define cx_str_name <name>

Define the string maximum capacity (optional, default = 32):
    #define cx_str_cap <8|16|32>

Define optional error handler function with type:
void (*handler)(const char* err_msg, const char* func_name)
which will be called if error is detected (default = no handler):
    #define cx_str_error_handler <func>

Define optional custom allocator function which must return pointer to allocator interface.
Uses default allocator if not defined.
This allocator will be used for all instances of this type.
    #define cx_str_allocator <alloc_func>

Define optional custom allocator pointer or function which return pointer to allocator.
Uses default allocator if not defined.
This allocator will be used for all instances of this array type.
    #define cx_str_allocator <allocator>

Sets if string uses custom allocator per instance
If set, it is necessary to initialize each string with the desired allocator.
    #define cx_str_instance_allocator

Sets if all string functions are prefixed with 'static'
    #define cx_str_static

Sets if all string functions are prefixed with 'inline'
    #define cx_str_inline

Sets to implement functions in this translation unit:
    #define cx_str_implement


String API
----------
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

Reserve string capacity for at least 'n' bytes
    void cxstr_reserve(cxtr* s, size_t n);

Returns current string capacity in number of bytes
    size_t cxstr_cap(const cxstr* s);

Returns current string length in number of bytes
    size_t cxstr_len(const cx_str_name* s);

Returns current number of UTF8 codepoints in the string
    size_t cxstr_lencp(const cx_str_name* s);

Returns const pointer to string data
    const char* cxstr_data(const cx_str_name* s);

Returns if the string is empty
    bool cxstr_empty(const cx_str_name* s);

Sets string capacity to at least 'cap'
    void cxstr_setcap(cxstr* s, size_t cap);

Copy 'n' bytes from 'src' to 's', replacing current text
    void cxstr_cpyn(cxstr* s, const char* src, size_t n);

Copy bytes from nul terminated 'src' string to 's', replacing current text
    void cxstr_cpy(cx_str_name* s, const char* src);

Copy cxstr 'src' to 's', replacing current text
    void cxstr_cpys(cx_str_name* s, const cx_str_name* src);

Appends 'n' bytes from 'src' to 's'
    void cxstr_catn(cxstr* s, const char* src, size_t n);

Appends nul terminated string 'src' to 's'
    void cxstr_cat(cx_str_name* s, const char* src);

Appends character to the string
    void cxstr_catc(cx_str_name* s, int c);

Appends cxstr 'src' to 's'
    void cxstr_cats(cx_str_name* s, const cx_str_name* src);

Appends UTF8 bytes encoded from codepoint 'cp' to 's'
    void cxstr_catcp(cx_str_name* s, int32_t cp);

Inserts 'n' bytes from 'src' into 's' at index 'idx'
    void cxstr_insn)(cxstr* s, const char* src, size_t n, size_t idx);

Inserts nul terminated string 'src' to 's' at index 'idx'
    void cxstr_ins(cx_str_name* s, const char* src, size_t idx);

Inserts cxstr 'src' to 's' at index idx
    void cxstr_inss(cxstr* s, const cx_str_name* src, size_t idx);

Deletes 'n' bytes from 's' starting at 'idx'
    void void cxstr_deln(cxstr* s, size_t idx, size_t n);

Compares 'n' bytes from 'src' with bytes from 's'.
Return 0 if equal, -1 or 1 (as memcmp())
    int cxstr_cmpn(cxstr* s, const char* src, size_t n);

Compares bytes from nul terminated string 'src' with bytes from 's'.
Return 0 if equal, -1 or 1 (as memcmp())
    int cxstr_cmp(cxstr* s, const char* src);

Compares bytes from cxstr 'src' with bytes from 's'.
Return 0 if equal, -1 or 1 (as memcmp())
    int cxstr_cmps(cxstr* s, const cx_str_name* src);

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

// String maximum capacity in number of bits
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

// Default array allocator
#ifndef cx_str_allocator
    #define cx_str_allocator cx_def_allocator()
#endif

// Use custom instance allocator
#ifdef cx_str_instance_allocator
    #define cx_str_alloc_field_\
        const CxAllocator* alloc_;
    #define cx_str_alloc_(s,n)\
        cx_alloc_malloc(s->alloc_, n)
    #define cx_str_realloc_(s,oldp,oldn,n)\
        cx_alloc_realloc(s->alloc_, oldp,oldn, n)
    #define cx_str_free_(s,p,n)\
        cx_alloc_free(s->alloc_, p, n)
// Use global type allocator
#else
    #define cx_str_alloc_field_
    #define cx_str_alloc_(s,n)\
        cx_alloc_malloc(cx_str_allocator,n)
    #define cx_str_realloc_(s,oldp,oldn,n)\
        cx_alloc_realloc(cx_str_allocator,oldp,oldn, n)
    #define cx_str_free_(s,p,n)\
        cx_alloc_free(cx_str_allocator,p,n)
#endif

//
// Declarations
//
typedef struct cx_str_name {
    cx_str_alloc_field_
    cx_str_cap_type_ len_;
    cx_str_cap_type_ cap_;
    char* data;
} cx_str_name;

#ifdef cx_str_instance_allocator
    cx_str_api_ cx_str_name cx_str_name_(_init)(const CxAllocator* a);
    cx_str_api_ cx_str_name cx_str_name_(_initn)(const CxAllocator* a, const char* src, size_t n);
    cx_str_api_ cx_str_name cx_str_name_(_initc)(const CxAllocator* a, const char* src);
    cx_str_api_ cx_str_name cx_str_name_(_inits)(const CxAllocator* a, const cx_str_name* src);
#else
    cx_str_api_ cx_str_name cx_str_name_(_init)(void);
    cx_str_api_ cx_str_name cx_str_name_(_initn)(const char* src, size_t n);
    cx_str_api_ cx_str_name cx_str_name_(_initc)(const char* src);
    cx_str_api_ cx_str_name cx_str_name_(_inits)(const cx_str_name* src);
#endif
cx_str_api_ void cx_str_name_(_free)(cx_str_name* s);
cx_str_api_ void cx_str_name_(_clear)(cx_str_name* s);
cx_str_api_ void cx_str_name_(_reserve)(cx_str_name* s, size_t n);
cx_str_api_ size_t cx_str_name_(_cap)(const cx_str_name* s);
cx_str_api_ size_t cx_str_name_(_len)(const cx_str_name* s);
cx_str_api_ size_t cx_str_name_(_lencp)(const cx_str_name* s);
cx_str_api_ const char* cx_str_name_(_data)(const cx_str_name* s);
cx_str_api_ bool cx_str_name_(_empty)(const cx_str_name* s);
cx_str_api_ void cx_str_name_(_setcap)(cx_str_name* s, size_t cap);
cx_str_api_ void cx_str_name_(_cpyn)(cx_str_name* s, const char* src, size_t n);
cx_str_api_ void cx_str_name_(_cpy)(cx_str_name* s, const char* src);
cx_str_api_ void cx_str_name_(_cpys)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(_catn)(cx_str_name* s, const char* src, size_t n);
cx_str_api_ void cx_str_name_(_cat)(cx_str_name* s, const char* src);
cx_str_api_ void cx_str_name_(_catc)(cx_str_name* s, int c);
cx_str_api_ void cx_str_name_(_cats)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(_catcp)(cx_str_name* s, int32_t cp);
cx_str_api_ void cx_str_name_(_insn)(cx_str_name* s, const char* src, size_t n, size_t idx);
cx_str_api_ void cx_str_name_(_ins)(cx_str_name* s, const char* src, size_t idx);
cx_str_api_ void cx_str_name_(_inss)(cx_str_name* s, const cx_str_name* src, size_t idx);
cx_str_api_ void cx_str_name_(_deln)(cx_str_name* s, size_t idx, size_t n);
cx_str_api_ int  cx_str_name_(_cmpn)(const cx_str_name* s, const char* src, size_t n);
cx_str_api_ int  cx_str_name_(_cmp)(const cx_str_name* s, const char* src);
cx_str_api_ int  cx_str_name_(_cmps)(const cx_str_name* s, const cx_str_name* src);
cx_str_api_ int  cx_str_name_(_icmp)(cx_str_name* s, const char* src);
cx_str_api_ int  cx_str_name_(_icmps)(cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(_vprintf)(cx_str_name* s, const char *fmt, va_list ap);
cx_str_api_ void cx_str_name_(_printf)(cx_str_name* s, const char *fmt, ...);
cx_str_api_ ptrdiff_t cx_str_name_(_findn)(const cx_str_name* s, size_t start, const char *src, size_t n);
cx_str_api_ ptrdiff_t cx_str_name_(_find)(const cx_str_name* s, const char *src);
cx_str_api_ ptrdiff_t cx_str_name_(_finds)(const cx_str_name* s, const cx_str_name* src);
cx_str_api_ ptrdiff_t cx_str_name_(_findcp)(const cx_str_name* s, int32_t cp);
cx_str_api_ ptrdiff_t cx_str_name_(_ifind)(const cx_str_name* s, const char *src);
cx_str_api_ ptrdiff_t cx_str_name_(_ifinds)(const cx_str_name* s, const cx_str_name* src);
cx_str_api_ void cx_str_name_(_substr)(const cx_str_name* s, size_t start, size_t len, cx_str_name* dst);
cx_str_api_ void cx_str_name_(_replace)(cx_str_name* s, const char* old, const char* new, size_t count);
cx_str_api_ bool cx_str_name_(_validu8)(const cx_str_name* s);
cx_str_api_ void cx_str_name_(_upper)(cx_str_name* s);
cx_str_api_ void cx_str_name_(_lower)(cx_str_name* s);
cx_str_api_ char* cx_str_name_(_ncp)(cx_str_name* s, char* iter, int32_t* cp);
cx_str_api_ void cx_str_name_(_ltrim)(cx_str_name* s, const char* cset);
cx_str_api_ void cx_str_name_(_rtrim)(cx_str_name* s, const char* cset);


//
// Implementations
//
#ifdef cx_str_implement

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
// The requested 'new_len' does not count the nul terminating byte.
static void cx_str_name_(_grow_)(cx_str_name* s, size_t new_len) {

    // Special case for empty string
    if (new_len == 0 && s->data == NULL) {
        new_len = 1;
    } else {
        if (new_len <= s->len_) {
            return;
        }
        if (new_len + 1 < s->cap_) {
            return;
        }
    }
    size_t new_cap = 2 * new_len;

    const size_t elem_size = sizeof(*(s->data));
    const size_t alloc_size = elem_size * new_cap;
    void* new = cx_str_realloc_(s, s->data, s->cap_, alloc_size);
    if (new == NULL) {
        return;
    }
    s->data = new;
    s->cap_ = new_cap;
}


#ifdef cx_str_instance_allocator

    cx_str_api_ cx_str_name cx_str_name_(_init)(const CxAllocator* a) {

        return (cx_str_name) {.alloc_ = a};
    }

    cx_str_api_ cx_str_name cx_str_name_(_initn)(const CxAllocator* a, const char* src, size_t n) {

        cx_str_name s = {.alloc_ = a};
        cx_str_name_(_cpyn)(&s, src, n);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(_initc)(const CxAllocator* a, const char* src) {

        cx_str_name s = {.alloc_ = a};
        cx_str_name_(_cpy)(&s, src);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(_inits)(const CxAllocator* a, const cx_str_name* src) {

        cx_str_name s = {.alloc_ = a};
        cx_str_name_(_cpys)(&s, src);
        return s;
    }

#else

    cx_str_api_ cx_str_name cx_str_name_(_init)(void) {
        return (cx_str_name) {0};
    }

    cx_str_api_ cx_str_name cx_str_name_(_initn)(const char* src, size_t n) {

        cx_str_name s = cx_str_name_(_init)();
        cx_str_name_(_cpyn)(&s, src, n);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(_initc)(const char* src) {

        cx_str_name s = cx_str_name_(_init)();
        cx_str_name_(_cpy)(&s, src);
        return s;
    }

    cx_str_api_ cx_str_name cx_str_name_(_inits)(const cx_str_name* src) {

        cx_str_name s = cx_str_name_(_init)();
        cx_str_name_(_cpys)(&s, src);
        return s;
    }
#endif


cx_str_api_ void cx_str_name_(_free)(cx_str_name* s) {

    cx_str_free_(s, s->data, s->cap_);
    s->cap_ = 0;
    s->len_ = 0;
    s->data = NULL;
}

cx_str_api_ void cx_str_name_(_clear)(cx_str_name* s) {

    s->len_ = 0;
}

cx_str_api_ void cx_str_name_(_reserve)(cx_str_name* s, size_t n) {

    cx_str_name_(_grow_)(s, s->len_ + n);
}

cx_str_api_ size_t cx_str_name_(_cap)(const cx_str_name* s) {

    return s->cap_;
}

cx_str_api_ size_t cx_str_name_(_len)(const cx_str_name* s) {

    return s->len_;
}

cx_str_api_ size_t cx_str_name_(_lencp)(const cx_str_name* s) {

    return utf8len(s->data);
}

cx_str_api_ const char* cx_str_name_(_data)(const cx_str_name* s) {

    return s->data;
}

cx_str_api_ bool cx_str_name_(_empty)(const cx_str_name* s) {

    return s->len_ == 0;
}

cx_str_api_ void cx_str_name_(_setcap)(cx_str_name* s, size_t cap) {

    cx_str_name_(_grow_)(s, cap);
}

cx_str_api_ void cx_str_name_(_cpyn)(cx_str_name* s, const char* src, size_t n) {

    if (src == NULL) {
        return;
    }
    cx_str_name_(_grow_)(s, n);
    memcpy(s->data, src, n);
    s->len_ = n;
    s->data[s->len_] = 0;
}

cx_str_api_ void cx_str_name_(_cpy)(cx_str_name* s, const char* src) {

    if (src == NULL) {
        return;
    }
    cx_str_name_(_cpyn)(s, src, strlen(src));
}

cx_str_api_ void cx_str_name_(_cpys)(cx_str_name* s, const cx_str_name* src) {

    cx_str_name_(_cpyn)(s, src->data, src->len_);
}

cx_str_api_ void cx_str_name_(_catn)(cx_str_name* s, const char* src, size_t n) {

    cx_str_name_(_grow_)(s, s->len_ + n);
    memcpy(s->data + s->len_, src, n);
    s->len_ += n;
    if (s->len_) {
        s->data[s->len_] = 0;
    }
}

cx_str_api_ void cx_str_name_(_cat)(cx_str_name* s, const char* src) {

    cx_str_name_(_catn)(s, src, strlen(src));
}

cx_str_api_ void cx_str_name_(_catc)(cx_str_name* s, int c) {

    const char cc = (char)c;
    cx_str_name_(_catn)(s, &cc, 1);
}

cx_str_api_ void cx_str_name_(_cats)(cx_str_name* s, const cx_str_name* src) {

    cx_str_name_(_catn)(s, src->data, src->len_);
}

cx_str_api_ void cx_str_name_(_catcp)(cx_str_name* s, int32_t cp) {

    const size_t size = utf8codepointsize(cp);
    cx_str_name_(_grow_)(s, s->len_ + size);
    utf8catcodepoint(s->data + s->len_, cp, size);
    s->len_ += size;
    s->data[s->len_] = 0;
}

cx_str_api_ void cx_str_name_(_insn)(cx_str_name* s, const char* src, size_t n, size_t idx) {

#ifdef cx_str_error_handler
    if (idx > s->len_) {
        cx_str_error_handler("invalid index", __func__);
        return;
    }
#endif

    cx_str_name_(_grow_)(s, s->len_ + n);
    memmove(s->data + idx + n, s->data + idx, s->len_ - idx);
    memcpy(s->data + idx, src, n);
    s->len_ += n;
    s->data[s->len_] = 0;
}

cx_str_api_ void cx_str_name_(_ins)(cx_str_name* s, const char* src, size_t idx) {

    cx_str_name_(_insn)(s, src, strlen(src), idx);
}

cx_str_api_ void cx_str_name_(_inss)(cx_str_name* s, const cx_str_name* src, size_t idx) {

    cx_str_name_(_insn)(s, src->data, src->len_, idx);
}

cx_str_api_ void cx_str_name_(_deln)(cx_str_name* s, size_t idx, size_t n) {

#ifdef cx_str_error_handler
    if (idx >= s->len_) {
        cx_str_error_handler("invalid index", __func__);
        return;
    }
#endif

    const size_t maxDel = s->len_ - idx;
    n = n > maxDel ? maxDel : n;
    memmove(s->data + idx, s->data + idx + n, s->len_ - idx - n);
    s->len_ -= n;
    if (s->len_) {
        s->data[s->len_] = 0;
    }
}

cx_str_api_ int  cx_str_name_(_cmpn)(const cx_str_name* s, const char* src, size_t n) {

    if (s->len_ > n) {
        return 1;
    }
    if (s->len_ < n) {
        return -1;
    }
    return memcmp(s->data, src, n);
}

cx_str_api_ int cx_str_name_(_cmp)(const cx_str_name* s, const char* src) {

    return cx_str_name_(_cmpn)(s, src, strlen(src));
}

cx_str_api_ int cx_str_name_(_cmps)(const cx_str_name* s, const cx_str_name* src) {

    return cx_str_name_(_cmpn)(s, src->data, src->len_);
}

cx_str_api_ int cx_str_name_(_icmp)(cx_str_name* s, const char* src) {

    return utf8casecmp(s->data, src);
}

cx_str_api_ int cx_str_name_(_icmps)(cx_str_name* s, const cx_str_name* src) {

    if (s->len_ > src->len_) {
        return 1;
    }
    if (s->len_ < src->len_) {
        return -1;
    }
    return utf8casecmp(s->data, src->data);
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
cx_str_api_ void cx_str_name_(_vprintf)(cx_str_name* s, const char *fmt, va_list ap) {
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
    cx_str_name_(_catn)(s, buf, bufstrlen);
    if (buf != staticbuf) {
        cx_str_free_(s, buf, buflen);
    }
}

// Based on https://github.com/antirez/sds/blob/master/sds.c
cx_str_api_ void cx_str_name_(_printf)(cx_str_name* s, const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    cx_str_name_(_vprintf)(s, fmt, ap);
    va_end(ap);
}

cx_str_api_ ptrdiff_t cx_str_name_(_findn)(const cx_str_name* s, size_t start, const char *src, size_t n) {

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

cx_str_api_ ptrdiff_t cx_str_name_(_find)(const cx_str_name* s, const char *src) {

    return cx_str_name_(_findn)(s, 0, src, strlen(src));
}

cx_str_api_ ptrdiff_t cx_str_name_(_finds)(const cx_str_name* s, const cx_str_name* src) {

    return cx_str_name_(_findn)(s, 0, src->data, src->len_);
}

cx_str_api_ ptrdiff_t cx_str_name_(_findcp)(const cx_str_name* s, int32_t cp) {

    char* n = utf8chr(s->data, cp);
    if (n == NULL) {
        return -1;
    }
    return n - s->data;
}

cx_str_api_ ptrdiff_t cx_str_name_(_ifind)(const cx_str_name* s, const char *src) {

    char *n = utf8casestr(s->data, src);
    if (n == NULL) {
        return -1;
    }
    return n - s->data;
}

cx_str_api_ ptrdiff_t cx_str_name_(_ifinds)(const cx_str_name* s, const cx_str_name* src) {

    if (s->len_ < src->len_) {
        return -1;
    }
    return cx_str_name_(_ifind)(s, src->data);
}

cx_str_api_ void cx_str_name_(_substr)(const cx_str_name* s, size_t start, size_t len, cx_str_name* dst) {

     if (start >= s->len_) {
         dst->len_ = 0;
         return ;
     }
    const size_t maxSize = s->len_ - start;
    len = len > maxSize ? maxSize : len;
    cx_str_name_(_cpyn)(dst, s->data + start, len);
}

cx_str_api_ void cx_str_name_(_replace)(cx_str_name* s, const char* old, const char* new, size_t count) {

    const size_t olen = strlen(old);
    const size_t nlen = strlen(new);
    size_t start = 0;
    while (1) {
        ptrdiff_t pos = cx_str_name_(_findn)((cx_str_name*)s, start, old, olen);
        if (pos < 0) {
            break;
        }
        if (olen > nlen) {
            cx_str_name_(_deln)(s, pos, olen - nlen);
        } else if (olen < nlen) {
            const size_t addLen = nlen - olen;
            cx_str_name_(_grow_)(s, s->len_ + addLen);
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


cx_str_api_ bool cx_str_name_(_validu8)(const cx_str_name* s) {

    return utf8valid(s->data) == 0;
}

cx_str_api_ void cx_str_name_(_upper)(cx_str_name* s) {

    utf8upr(s->data);
}

cx_str_api_ void cx_str_name_(_lower)(cx_str_name* s) {

    utf8lwr(s->data);
}

cx_str_api_ char* cx_str_name_(_ncp)(cx_str_name* s, char* iter, int32_t* cp) {

    if (iter < s->data || iter >= (s->data + s->len_)) {
        return NULL;
    }
    return utf8codepoint(iter, cp);
}

cx_str_api_ void cx_str_name_(_ltrim)(cx_str_name* s, const char* cset)  {

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
    cx_str_name_(_deln)(s, 0, deln);
}

cx_str_api_ void cx_str_name_(_rtrim)(cx_str_name* s, const char* cset) {

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
    cx_str_name_(_deln)(s, s->len_ - deln, deln);
}

#endif // cx_str_implement

// Undefine config  macros
#undef cx_str_name
#undef cx_str_static
#undef cx_str_inline
#undef cx_str_cap
#undef cx_str_instance_allocator
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


