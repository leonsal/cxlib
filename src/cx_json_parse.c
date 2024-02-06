#include "cx_alloc.h"
#include "cx_var.h"
#include "cx_json_parse.h"

#include "jsmn.h"

typedef struct ParseState {
    const CxAllocator* alloc;
    const char* data;
    int         count;
    jsmntok_t*  tokens;
    size_t      toki;
    CxVar       root;
} ParseState;

static int cx_json_parse_token(ParseState* ps, CxVar* var);
static int cx_json_parse_prim(ParseState* ps, CxVar* var);
static int cx_json_parse_arr(ParseState* ps, CxVar* var);
static int cx_json_parse_obj(ParseState* ps, CxVar* var);

int cx_json_parse(const char* data, size_t len, CxVar* var, const CxAllocator* alloc) {

    // Count the number of tokens
    jsmn_parser p;
    jsmn_init(&p);
    int count = jsmn_parse(&p, data, len, NULL, 0);
    if (count < 0) {
        return count;
    }

    // Allocate memory for tokens arra
    size_t nalloc = sizeof(jsmntok_t) * count;
    jsmntok_t* tokens = cx_alloc_malloc(alloc, nalloc);

    // Parses the data again
    jsmn_init(&p);
    int res = jsmn_parse(&p, data, len, tokens, count);
    if (res < 0) {
        return res;
    }

    ParseState ps = {
        .alloc = alloc,
        .data = data,
        .count = count,
        .tokens = tokens,
    };

    CxVar root;
    while (ps.toki < count) {
        res = cx_json_parse_token(&ps, &root);
    }

exit:
    cx_alloc_free(alloc, tokens, nalloc); 
    return 0;
}

// Parses the current token
static int cx_json_parse_token(ParseState* ps, CxVar* var) {

    jsmntok_t* tok = &ps->tokens[ps->toki];
    switch (tok->type) {
        case JSMN_PRIMITIVE:
            return cx_json_parse_prim(ps, var);
        case JSMN_STRING:
            break;
        case JSMN_ARRAY:
            return cx_json_parse_arr(ps, var);
        case JSMN_OBJECT:
            return cx_json_parse_obj(ps, var);
            break;
        default:
            return -1;
    }
}


static int cx_json_parse_prim(ParseState* ps, CxVar* var) {

    jsmntok_t* tok = &ps->tokens[ps->toki];
    if (ps->data[tok->start] == 'n') {
        *var = cx_var_new_null();
        return 0;
    }
    if (ps->data[tok->start] == 't') {
        *var = cx_var_new_bool(true);
        return 0;
    }
    if (ps->data[tok->start] == 'f') {
        *var = cx_var_new_bool(false);
        return 0;
    }

    return 0;
}

static int cx_json_parse_arr(ParseState* ps, CxVar* var) {

    *var = cx_var_new_arr(ps->alloc);
    jsmntok_t* tok = &ps->tokens[ps->toki];
    ps->toki++;
    for (size_t idx = 0; idx < tok->size; idx++) {
        CxVar el;
        cx_json_parse_token(ps, &el);
        cx_var_arr_push(var, el);
        ps->toki++;
    }
    return 0;
}

static int cx_json_parse_obj(ParseState* ps, CxVar* var) {

    *var = cx_var_new_map(ps->alloc);
    jsmntok_t* tok = &ps->tokens[ps->toki];
    ps->toki++;
    for (size_t idx = 0; idx < tok->size; idx++) {
        // Key
        // Value
        ps->toki++;
    }
    return 0;
}

// #include "../jsmn.h"
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
//
// /* Function realloc_it() is a wrapper function for standard realloc()
//  * with one difference - it frees old memory pointer in case of realloc
//  * failure. Thus, DO NOT use old data pointer in anyway after call to
//  * realloc_it(). If your code has some kind of fallback algorithm if
//  * memory can't be re-allocated - use standard realloc() instead.
//  */
// static inline void *realloc_it(void *ptrmem, size_t size) {
//   void *p = realloc(ptrmem, size);
//   if (!p) {
//     free(ptrmem);
//     fprintf(stderr, "realloc(): errno=%d\n", errno);
//   }
//   return p;
// }
//
// /*
//  * An example of reading JSON from stdin and printing its content to stdout.
//  * The output looks like YAML, but I'm not sure if it's really compatible.
//  */
//
// static int dump(const char *js, jsmntok_t *t, size_t count, int indent) {
//   int i, j, k;
//   jsmntok_t *key;
//   if (count == 0) {
//     return 0;
//   }
//   if (t->type == JSMN_PRIMITIVE) {
//     printf("%.*s", t->end - t->start, js + t->start);
//     return 1;
//   } else if (t->type == JSMN_STRING) {
//     printf("'%.*s'", t->end - t->start, js + t->start);
//     return 1;
//   } else if (t->type == JSMN_OBJECT) {
//     printf("\n");
//     j = 0;
//     for (i = 0; i < t->size; i++) {
//       for (k = 0; k < indent; k++) {
//         printf("  ");
//       }
//       key = t + 1 + j;
//       j += dump(js, key, count - j, indent + 1);
//       if (key->size > 0) {
//         printf(": ");
//         j += dump(js, t + 1 + j, count - j, indent + 1);
//       }
//       printf("\n");
//     }
//     return j + 1;
//   } else if (t->type == JSMN_ARRAY) {
//     j = 0;
//     printf("\n");
//     for (i = 0; i < t->size; i++) {
//       for (k = 0; k < indent - 1; k++) {
//         printf("  ");
//       }
//       printf("   - ");
//       j += dump(js, t + 1 + j, count - j, indent + 1);
//       printf("\n");
//     }
//     return j + 1;
//   }
//   return 0;
// }
//
// int main() {
//   int r;
//   int eof_expected = 0;
//   char *js = NULL;
//   size_t jslen = 0;
//   char buf[BUFSIZ];
//
//   jsmn_parser p;
//   jsmntok_t *tok;
//   size_t tokcount = 2;
//
//   /* Prepare parser */
//   jsmn_init(&p);
//
//   /* Allocate some tokens as a start */
//   tok = malloc(sizeof(*tok) * tokcount);
//   if (tok == NULL) {
//     fprintf(stderr, "malloc(): errno=%d\n", errno);
//     return 3;
//   }
//
//   for (;;) {
//     /* Read another chunk */
//     r = fread(buf, 1, sizeof(buf), stdin);
//     if (r < 0) {
//       fprintf(stderr, "fread(): %d, errno=%d\n", r, errno);
//       return 1;
//     }
//     if (r == 0) {
//       if (eof_expected != 0) {
//         return 0;
//       } else {
//         fprintf(stderr, "fread(): unexpected EOF\n");
//         return 2;
//       }
//     }
//
//     js = realloc_it(js, jslen + r + 1);
//     if (js == NULL) {
//       return 3;
//     }
//     strncpy(js + jslen, buf, r);
//     jslen = jslen + r;
//
//   again:
//     r = jsmn_parse(&p, js, jslen, tok, tokcount);
//     if (r < 0) {
//       if (r == JSMN_ERROR_NOMEM) {
//         tokcount = tokcount * 2;
//         tok = realloc_it(tok, sizeof(*tok) * tokcount);
//         if (tok == NULL) {
//           return 3;
//         }
//         goto again;
//       }
//     } else {
//       dump(js, tok, p.toknext, 0);
//       eof_expected = 1;
//     }
//   }
//
//   return EXIT_SUCCESS;
// }
