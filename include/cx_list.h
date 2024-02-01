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

Defines optional list element comparison function used in find() with type:
int (*cmp)(const void* el1, const void* el2, size_t size);
If not defined, uses 'memcmp()'
    #define cx_list_cmp_el <cmp_func>

Sets if all list functions are prefixed with 'static'
    #define cx_list_static

Sets if all list functions are prefixed with 'inline'
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

Free memory allocated by the list elements.
The list can be reused after this.
    void cxlist_free(cxlist* l);

Clear the list keeping the memory allocated by the list elements in an internal free list.
    void cxlist_free(cxlist* l);

Returns it the list is empty (length == 0)
    bool cxlist_empty(const cxlist* l);

Returns the current number of elements in the list
    size_t cxlist_count(const cxlist* l);

Pushes one element at the back of the list.
This element will be the last one.
    void cxlist_push(cxlist* l, cxtype v);

Pops and returns the last element of the list.
Error handler is called if defined and list is empty.
    cxtype cxlist_pop(cxlist* l);

Pushes one element at the front of the list.
This element will be the first one
    void cxlist_pushf(cxlist* l, cxtype v);

Pops one element at the front of the list.
Error handler is called if defined and list is empty.
    cxtype cxlist_popf(cxlist* l, cxtype v);

Returns pointer to the first element of the list and initializes the supplied iterator.
Returns NULL if the list is empty.
    cxtype* cxlist_first(cxlist_iter* iter);

Returns pointer to the next element of the list from the current iterator state
and updates the supplied iterator. Returns NULL if no next element.
    cxtype* cxlist_next(cxlist_iter* iter);

Returns pointer to the last element of the list and initializes the supplied interator.
Returns NULL if the list is empty.
    cxtype* cxlist_last(cxlist_iter* iter);

Returns pointer to the previous element of the list from the current iterator state
and updates the supplied iterator.
Returns NULL if no prev element.
    cxtype* cxlist_prev(cxlist_iter* iter);

Returns pointer to the current element from the current iterator state
Returns NULL if no current element.
    cxtype* cxlist_curr(cxlist_iter* iter);

Inserts element before the current position of the iterator.
If there is no current element (list is empty) do nothing.
The iterator is not modified.
    void cxlist_ins_before(cxlist_iter* iter, cxtype v);

Inserts element after the current position of the iterator.
If there is no current element (list is empty) do nothing.
The iterator is not modified.
    void cxlist_ins_after(cxlist_iter* iter, cxtype v);

Deletes element at the current position of the iterator if any.
If 'next' is true, the iterator is advanced to the next element
otherwise the iterator is moved to the previous element.
The pointer to the current element is returned.
    cxtype* cxlist_del(cxlist_iter* iter, bool next);

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
    #error "cx_list_type not defined"
#endif

// Auxiliary internal macros
#define cx_list_concat2_(a, b) a ## b
#define cx_list_concat1_(a, b) cx_list_concat2_(a, b)
#define cx_list_name_(name) cx_list_concat1_(cx_list_name, name)

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
    cx_list_alloc_field_            // Optional custom allocator
    cx_list_name_(_el)* first_;     // Pointer to first element
    cx_list_name_(_el)* last_;      // Pointer to last element
    cx_list_name_(_el)* free_;      // List of free elements
    size_t count_;                  // Current number of elements in the list
} cx_list_name;

typedef struct cx_list_name_(_iter) {
    cx_list_name* l;
    cx_list_name_(_el)* curr;
} cx_list_name_(_iter);

#ifdef cx_list_instance_allocator
    cx_list_api_ cx_list_name cx_list_name_(_init)(const CxAllocator*);
#else
    cx_list_api_ cx_list_name cx_list_name_(_init)(void);
#endif
cx_list_api_ void cx_list_name_(_free)(cx_list_name* l);
cx_list_api_ void cx_list_name_(_clear)(cx_list_name* l);
cx_list_api_ bool cx_list_name_(_empty)(const cx_list_name* l);
cx_list_api_ size_t cx_list_name_(_count)(const cx_list_name* l);
cx_list_api_ void cx_list_name_(_push)(cx_list_name* l, cx_list_type v);
cx_list_api_ cx_list_type cx_list_name_(_pop)(cx_list_name* l);
cx_list_api_ void cx_list_name_(_pushf)(cx_list_name* l, cx_list_type v);
cx_list_api_ cx_list_type cx_list_name_(_popf)(cx_list_name* l);
cx_list_api_ cx_list_type* cx_list_name_(_first)(cx_list_name* l, cx_list_name_(_iter)* iter);
cx_list_api_ cx_list_type* cx_list_name_(_next)(cx_list_name_(_iter)* iter);
cx_list_api_ cx_list_type* cx_list_name_(_last)(cx_list_name* l, cx_list_name_(_iter)* iter);
cx_list_api_ cx_list_type* cx_list_name_(_prev)(cx_list_name_(_iter)* iter);
cx_list_api_ cx_list_type* cx_list_name_(_curr)(cx_list_name_(_iter)* iter);
cx_list_api_ void cx_list_name_(_ins_before)(cx_list_name_(_iter)* iter, cx_list_type v);
cx_list_api_ void cx_list_name_(_ins_after)( cx_list_name_(_iter)* iter, cx_list_type v);
cx_list_api_ cx_list_type* cx_list_name_(_del)(cx_list_name_(_iter)* iter, bool next);
cx_list_api_ cx_list_type* cx_list_name_(_find)(cx_list_name* l, cx_list_type v, cx_list_name_(_iter)* iter);

//
// Implementation
//
#ifdef cx_list_implement

// Internal function to get pointer to new element from the free list or from new allocation
cx_list_api_ cx_list_name_(_el)* cx_list_name_(_new_el_)(cx_list_name* l) {

    cx_list_name_(_el)* el;
    if (l->free_) {
        el = l->free_;
        l->free_ = l->free_->next_;
        return el;
    }
    return cx_list_alloc_(l, sizeof(cx_list_name_(_el)));
}

// Internal function to dispose element by adding to the free list
cx_list_api_ void cx_list_name_(_del_el_)(cx_list_name* l, cx_list_name_(_el)* el) {

    el->next_ = l->free_;
    l->free_ = el;
}

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
    cx_list_name_(_el)* free = l->free_;
    while (free) {
        cx_list_name_(_el)* next = free->next_;
        cx_list_free_(l, free, sizeof(cx_list_name_(_el))); 
        free = next;
    }
    l->first_ = NULL;
    l->last_  = NULL;
    l->free_  = NULL;
    l->count_ = 0;
}

cx_list_api_ void cx_list_name_(_clear)(cx_list_name* l) {

    if (l->last_) {
        l->last_->next_ = l->free_;
        l->free_ = l->first_;
    }
    l->first_ = NULL;
    l->last_  = NULL;
    l->count_ = 0;
}

cx_list_api_ bool cx_list_name_(_empty)(const cx_list_name* l) {

    return l->count_ == 0;
}

cx_list_api_ size_t cx_list_name_(_count)(const cx_list_name* l) {

    return l->count_;
}

cx_list_api_ void cx_list_name_(_push)(cx_list_name* l, cx_list_type v) {

    cx_list_name_(_el)* el = cx_list_name_(_new_el_)(l);
    el->data = v;
    if (l->last_) {
        el->prev_ = l->last_;
        el->next_ = NULL;
        l->last_->next_ = el;
        l->last_ = el;
    } else {
        el->prev_ = NULL;
        el->next_ = NULL;
        l->first_ = el;
        l->last_ = el;
    }
    l->count_++;
}

cx_list_api_ cx_list_type cx_list_name_(_pop)(cx_list_name* l) {

    if (l->count_) {
        cx_list_name_(_el)* last = l->last_;
        cx_list_type val = last->data;
        cx_list_name_(_el)* prev = l->last_->prev_;
        if (prev) {
            prev->next_ = NULL;
        }
        l->last_ = prev;
        l->count_--;
        if (l->count_ == 0) {
            l->first_ = NULL;
        }
        cx_list_name_(_del_el_)(l, last);
        return val;
    }
#ifdef cx_list_error_handler
    cx_list_error_handler("list empty", __func__);
#endif
    const cx_list_type val = {0};
    return val;
}

cx_list_api_ void cx_list_name_(_pushf)(cx_list_name* l, cx_list_type v) {

    cx_list_name_(_el)* el = cx_list_name_(_new_el_)(l);
    el->data = v;
    if (l->first_) {
        el->next_ = l->first_;
        el->prev_ = NULL;
        l->first_->prev_ = el;
        l->first_ = el;
    } else {
        el->prev_ = NULL;
        el->next_ = NULL;
        l->first_ = el;
        l->last_ = el;
    }
    l->count_++;
}

cx_list_api_ cx_list_type cx_list_name_(_popf)(cx_list_name* l) {

    if (l->count_) {
        cx_list_name_(_el)* first = l->first_;
        cx_list_type val = first->data;
        cx_list_name_(_el)* next = l->first_->next_;
        if (next) {
            next->prev_ = NULL;
        }
        l->first_ = next;
        l->count_--;
        if (l->count_ == 0) {
            l->last_ = NULL;
        }
        cx_list_name_(_del_el_)(l, first);
        return val;
    }
#ifdef cx_list_error_handler
    cx_list_error_handler("list empty", __func__);
#endif
    const cx_list_type val = {0};
    return val;
}

cx_list_api_ cx_list_type* cx_list_name_(_first)(cx_list_name* l, cx_list_name_(_iter)* iter) {

    iter->l = l;
    iter->curr = l->first_;
    if (l->first_) {
        return &l->first_->data;
    }
    return NULL;
}

cx_list_api_ cx_list_type* cx_list_name_(_next)(cx_list_name_(_iter)* iter) {

    if (iter->curr->next_) {
        cx_list_type* res = &iter->curr->next_->data;
        iter->curr = iter->curr->next_;
        return res;
    }
    return NULL;
}

cx_list_api_ cx_list_type* cx_list_name_(_last)(cx_list_name* l, cx_list_name_(_iter)* iter) {

    iter->l = l;
    iter->curr = l->last_;
    if (l->last_) {
        return &l->last_->data;
    }
    return NULL;
}

cx_list_api_ cx_list_type* cx_list_name_(_prev)(cx_list_name_(_iter)* iter) {

    if (iter->curr->prev_) {
        cx_list_type* res = &iter->curr->prev_->data;
        iter->curr = iter->curr->prev_;
        return res;
    }
    return NULL;
}

cx_list_api_ cx_list_type* cx_list_name_(_curr)(cx_list_name_(_iter)* iter) {

    if (iter->curr) {
        return &iter->curr->data;
    }
    return NULL;
}

cx_list_api_ void cx_list_name_(_ins_before)( cx_list_name_(_iter)* iter, cx_list_type v) {

    if (iter->curr == NULL) {
        return;
    }

    cx_list_name_(_el)* el = cx_list_alloc_(iter->l, sizeof(cx_list_name_(_el)));
    el->data = v;
    el->prev_ = iter->curr->prev_;
    el->next_ = iter->curr;
    if (iter->curr->prev_ == NULL) {
        iter->l->first_ = el;
    }
    iter->curr->prev_ = el;
    iter->l->count_++; 
}

cx_list_api_ void cx_list_name_(_ins_after)( cx_list_name_(_iter)* iter, cx_list_type v) {

    if (iter->curr == NULL) {
        return;
    }

    cx_list_name_(_el)* el = cx_list_alloc_(iter->l, sizeof(cx_list_name_(_el)));
    el->data = v;
    el->prev_ = iter->curr;
    el->next_ = iter->curr->next_;
    if (iter->curr->next_ == NULL) {
        iter->l->last_ = el;
    }
    iter->curr->next_ = el;
    iter->l->count_++; 
}

cx_list_api_ cx_list_type* cx_list_name_(_del)(cx_list_name_(_iter)* iter, bool next) {

    if (iter->curr == NULL) {
        return NULL;
    }

    cx_list_name_(_el)* curr = iter->curr;
    if (next) {
        iter->curr = curr->next_;
    } else {
        iter->curr = curr->prev_;
    }

    if (curr->prev_) {
        curr->prev_->next_ = curr->next_;
    } else {
        iter->l->first_ = curr->next_;
    }
    if (curr->next_) {
        curr->next_->prev_ = curr->prev_;
    } else {
        iter->l->last_ = curr->prev_;
    }
    iter->l->count_--;
    cx_list_name_(_del_el_)(iter->l, curr);
    if (iter->curr) { 
        return &iter->curr->data;
    } else {
        return NULL;
    }
}

cx_list_api_ cx_list_type* cx_list_name_(_find)(cx_list_name* l, cx_list_type v, cx_list_name_(_iter)* iter) {

    iter->l = l;
    iter->curr = l->first_;
    while (iter->curr) {
        if (cx_list_cmp_el(&v, &iter->curr->data, sizeof(cx_list_type)) == 0) {
            return &iter->curr->data;
        }
        iter->curr = iter->curr->next_;
    }
    return NULL;
}


#endif // cx_list_implement
