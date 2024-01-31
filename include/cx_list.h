/* Linked List Implementation
 
Example
-------

// Defines list of 'ints' using global type allocator
#define cx_list_name ai32
#define cx_list_type int
#define cx_list_static
#define cx_list_inline
#define cx_listy_implement
#include "cx_list.h"

int main() {

    ai32 a1 = ai32_init();
    ai32_push(&a1, 1);
    ai32_push(&a2, 2);
    assert(ai32_len(&a1) = 2);
    assert(*ai32_at(&a1, 1) = 1);
    ai32_free(&a1);
    return 0;
}
 
List configuration defines
---------------------------

Define the name of the list name (mandatory):
    #define cx_list_name <name>

Define the type of the list elements (mandatory):
    #define cx_list_type <name>

Define optional list maximum capacity (default = 32):
    #define cx_list_cap <8|16|32>

Define optional error handler function with type:
void (*handler)(const char* err_msg, const char* func_name)
which will be called if error is detected  (default = no handler):
    #define cx_list_error_handler <func>

Define optional custom allocator pointer or function which return pointer to allocator.
Uses default allocator if not defined.
This allocator will be used for all instances of this list type.
    #define cx_list_allocator <allocator>

Sets if list uses custom allocator per instance.
If set, it is necessary to initialize each list with the desired allocator.
    #define cx_list_instance_allocator

Defines optional list element comparison function used in find().
If not defined, uses 'memcmp()'
    #define cx_list_cmp_el <cmp_func>

Sets if all array functions are prefixed with 'static'
    #define cx_list_static

Sets if all array functions are prefixed with 'inline'
    #define cx_list_inline

Sets to implement functions in this translation unit:
    #define cx_list_implement


List API
--------

Assuming:
#define cx_list_name cxlist   // List type name
#define cx_list_type cxtype   // Type of elements of the list

Initialize list defined with custom allocator
    cxlist cxlist_init(const CxAllocator* a);

Initialize list NOT defined with custom allocator.
It is equivalent to zero initialize the list struct.
    cxlist cxlist_init();

Free list allocated memory
    void cxlist(cxlist* s);

Clone list returning a copy
    cxlist cxlist_clone(const cxlist a);

Returns it the list is empty (length == 0)
    bool cxlist_empty(const cxlist* a);

Pushes one element at the back of the array
    void cxlist_push(cxlist* a, cxtype v);

Pushes all elements from 'src' list at the back of this list
    void cxlist_pushl(cxlist* l, const cxlist* src);

Pops and returns the last element of the array.
Error handler is called if defined and array is empty.
    cxtype cxlist_pop(cxlist* l);

Returns pointer to list element at the specified index 'idx'. (O(n))
Error handler is called if defined and index is invalid,
otherwise NULL is returned.
    cxtype* cxlist_at(const cxlist* l, size_t idx);

Returns the last element of the array without removing it.
Error handler is called if defined and array is empty.
    cxtype cxrray_last(const cxarray* a);

Inserts specified element into the list at index 'idx'
Error handler is called if defined and index is invalid,
    void cxarray_ins(cxarray* a, cxtype v, size_t idx);

Inserts array 'src' into this array at index 'idx'
Error handler is called if defined and index is invalid,
    void cxarray_insa(cxarray* a, const cxarray* src, size_t idx);

Deletes element at index 'idx'
Error handler is called if defined and index is invalid,
    void cxarray_del(cxarray* a, size_t idx);

Finds element in the list returning its pointer or NULL if not found.
    cxtype* cxlist_find(cxlist* a, cxtype v);

*/ 
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

// List type name must be defined
#ifndef cx_list_name
    #error "cx_list_name not defined"
#endif
// List element type name must be defined
#ifndef cx_list_type
    #error "cx_listy_type not defined"
#endif

// Auxiliary internal macros
#define cx_list_concat2_(a, b) a ## b
#define cx_list_concat1_(a, b) cx_list_concat2_(a, b)
#define cx_list_name_(name) cx_list_concat1_(cx_list_name, name)

// list maximum capacity in number of bits
#define cx_list_cap8_     8
#define cx_list_cap16_    16
#define cx_list_cap32_    32

// Default capacity
#ifndef cx_list_cap
    #define cx_list_cap  cx_list_cap32_
#endif
#if cx_list_cap == cx_list_cap8_
    #define cx_list_cap_type_ uint8_t
    #define cx_list_max_cap_  (UINT8_MAX)
#elif cx_list_cap == cx_list_cap16_
    #define cx_list_cap_type_ uint16_t
    #define cx_list_max_cap_  (UINT16_MAX)
#elif cx_list_cap == cx_list_cap32_
    #define cx_list_cap_type_ uint32_t
    #define cx_list_max_cap_  (UINT32_MAX)
#else
    #error "invalid cx list capacity bits"
#endif

// API attributes
#if defined(cx_list_static) && defined(cx_list_inline)
    #define cx_list_api_ static inline
#elif defined(cx_list_static)
    #define cx_list_api_ static
#elif defined(cx_list_inline)
    #define cx_list_api_ inline
#else
    #define cx_list_api_
#endif

// Default element comparison function
#ifndef cx_list_cmp_el
    #define cx_list_cmp_el memcmp
#endif

// Default list allocator
#ifndef cx_list_allocator
    #define cx_list_allocator cxDefaultAllocator()
#endif

// Use custom instance allocator
#ifdef cx_list_instance_allocator
    #define cx_list_alloc_field_\
        const CxAllocator* alloc;
    #define cx_list_alloc_global_
    #define cx_list_alloc_(s,n)\
        cx_alloc_malloc(s->alloc, n)
    #define cx_list_free_(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_list_alloc_field_
    #define cx_list_alloc_(s,n)\
        cx_alloc_malloc(cx_list_allocator,n)
    #define cx_list_free_(s,p,n)\
        cx_alloc_free(cx_list_allocator,p,n)
#endif

//
// Declarations
//

// List element
typedef struct cx_list_name_(_el) {
    struct cx_list_name_(_el)* next_;
    struct cx_list_name_(_el)* prev_;
    cx_list_type data;
} cx_list_name_(_el);

// List state
typedef struct cx_list_name {
    cx_list_alloc_field_
    cx_list_name_(_el)* first_;
    cx_list_name_(_el)* last_;
    size_t count_;
} cx_list_name;

#ifdef cx_list_instance_allocator
    cx_list_api_ cx_list_name cx_list_name_(_init)(const CxAllocator*);
#else
    cx_list_api_ cx_list_name cx_list_name_(_init)(void);
#endif
cx_list_api_ void cx_list_name_(_free)(cx_list_name* l);
// cx_list_api_ cx_list_name cx_list_name_(_clone)(cx_list_name* a);
cx_list_api_ bool cx_list_name_(_empty)(cx_list_name* a);
cx_list_api_ void cx_list_name_(_push)(cx_list_name* a, cx_list_type v);
// cx_list_api_ void cx_list_name_(_pusha)(cx_list_name* a, const cx_list_name* src);
// cx_list_api_ cx_list_type cx_list_name_(_pop)(cx_list_name* a);
// cx_list_api_ cx_list_type* cx_list_name_(_at)(cx_list_name* a, size_t idx);
// cx_list_api_ cx_list_type cx_list_name_(_last)(const cx_list_name* a);
// cx_list_api_ void cx_list_name_(_reserve)(cx_list_name* a, size_t n);
// cx_list_api_ void cx_list_name_(_insn)(cx_list_name* a, cx_list_type* src, size_t n, size_t idx);
// cx_list_api_ void cx_list_name_(_ins)(cx_list_name* a, cx_list_type v, size_t idx);
// cx_list_api_ void cx_list_name_(_insa)(cx_list_name* a, const cx_list_name* src, size_t idx);
// cx_list_api_ void cx_list_name_(_deln)(cx_list_name* a, size_t idx, size_t n);
// cx_list_api_ void cx_list_name_(_del)(cx_list_name* a, size_t idx);
// cx_list_api_ void cx_list_name_(_delswap)(cx_list_name* a, size_t i);
// cx_list_api_ void cx_list_name_(_sort)(cx_list_name* a, int (*f)(const cx_list_type*, const cx_list_type*));
// cx_list_api_ ssize_t cx_list_name_(_find)(cx_list_name* a, cx_list_type v);

//
// Implementation
//
#ifdef cx_list_implement

#ifdef cx_list_instance_allocator
    cx_list_api_ cx_list_name cx_list_name_(_init)(const CxAllocator* alloc) {
        return (cx_list_name) {
            .alloc = alloc == NULL ? cxDefaultAllocator() : alloc,
        };
    }
#else
    cx_list_api_ cx_list_name cx_list_name_(_init)(void) {
        return (cx_list_name){0};
    }
#endif

cx_list_api_ void cx_list_name_(_free)(cx_list_name* l) {

    cx_list_name_(_el)* curr = l->first_;
    while (curr) {
        cx_list_name_(_el)* next = curr->next_;
        cx_list_free_(l, curr, sizeof(cx_list_name_(_el))); 
        curr = next;
    }
    l->first_ = NULL;
    l->last_ = NULL;
    l->count_ = 0;
}

cx_list_api_ bool cx_list_name_(_empty)(cx_list_name* l) {

    return l->count_ == 0;
}

cx_list_api_ void cx_list_name_(_push)(cx_list_name* l, cx_list_type v) {

    cx_list_name_(_el)* el = cx_list_alloc_(l, sizeof(cx_list_name_(_el)));
    el->data = v;
    if (l->last_) {
        el->prev_ = l->last_;
        l->last_->next_ = el;
        l->last_ = el;
    } else {
        l->first_ = el;
        l->last_ = el;
    }
    l->count_++;
}


#endif // cx_list_implement
