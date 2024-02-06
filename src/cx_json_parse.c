#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include "cx_alloc.h"
#include "cx_var.h"
#include "cx_json_parse.h"

#include "jsmn.h"

typedef struct ParseState {
    const CxAllocator* alloc;
    const char* data;       // JSON string to parse
    int         count;      // Number of tokens
    jsmntok_t*  tok;        // Pointer to current token
    jsmntok_t*  tok_last;   // Pointer to last token
} ParseState;

static int cx_json_parse_token(ParseState* ps, CxVar* var);
static int cx_json_parse_prim(ParseState* ps, CxVar* var);
static int cx_json_parse_str_token(ParseState* ps, cxvar_str* str);
static int cx_json_parse_str(ParseState* ps, CxVar* var);
static int cx_json_parse_arr(ParseState* ps, CxVar* var);
static int cx_json_parse_obj(ParseState* ps, CxVar* var);

#define CHK(CALL) {int res = CALL; if (res) {return res;}}

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
        .tok = &tokens[0],
        .tok_last = &tokens[count-1],
    };

    while (ps.tok <= ps.tok_last) {
        res = cx_json_parse_token(&ps, var);
        if (res) {
            break;
        }
    }

exit:
    cx_alloc_free(alloc, tokens, nalloc); 
    return res;
}

// Parses the current token
static int cx_json_parse_token(ParseState* ps, CxVar* var) {

    int res;
    switch (ps->tok->type) {
        case JSMN_PRIMITIVE:
            res = cx_json_parse_prim(ps, var);
            break;
        case JSMN_STRING:
            res = cx_json_parse_str(ps, var);
            break;
        case JSMN_ARRAY:
            res = cx_json_parse_arr(ps, var);
            break;
        case JSMN_OBJECT:
            res = cx_json_parse_obj(ps, var);
            break;
        default:
            res = 1;
            break;
    }
    return res;
}


static int cx_json_parse_prim(ParseState* ps, CxVar* var) {

    // Creates temporary string with primitive text
    // NO NEED to deallocate created string as it uses the same
    // allocator of the parser and will be freed at the end.
    cxvar_str prim = cxvar_str_init(ps->alloc);
    const char* pstart = &ps->data[ps->tok->start];
    cxvar_str_cpyn(&prim, pstart, ps->tok->end - ps->tok->start);

    if (cxvar_str_cmp(&prim, "null") == 0) {
        *var = cx_var_new_null();
        ps->tok++;
        return 0;
    }

    if (cxvar_str_cmp(&prim, "true") == 0) {
        *var = cx_var_new_bool(true);
        ps->tok++;
        return 0;
    }

    if (cxvar_str_cmp(&prim, "false") == 0) {
        *var = cx_var_new_bool(false);
        ps->tok++;
        return 0;
    }

    int start = pstart[0];
    if (start == '-' || start >= '0' && start <= '9') {
        // If number string contains '.' try to convert to double,
        // Otherwise try to convert to long int.
        char* pend;
        errno = 0;
        if (cxvar_str_find(&prim, ".") >= 0) {
            const double v = strtod(prim.data, &pend);
            *var = cx_var_new_float(v);
        } else {
            const int64_t v = strtod(prim.data, &pend);
            *var = cx_var_new_int(v);
        }

        // Checks for invalid number chars
        if (prim.data + cxvar_str_len(&prim) != pend) {
            return 1;
        }
        // Checks for overflow/underflow
        if (errno) {
            return 1;
        }
        ps->tok++;
        return 0;
    }

    // Invalid primitive
    return 1;
}

static int cx_json_parse_str_token(ParseState* ps, cxvar_str* str) {

    *str = cxvar_str_init(ps->alloc);
    const char* pnext = &ps->data[ps->tok->start];
    const char* pend = pnext + ps->tok->end - ps->tok->start;
    enum {Normal, Escape, Ucode} ;
    int state = Normal;
    int ucount = 0;
    char xdigits[5];
    while (pnext < pend) {
        if (state == Normal) {
            if (*pnext == '\\') {
                state = Escape;
                pnext++;
                continue;
            }
            cxvar_str_catc(str, *pnext);
            pnext++;
            continue;
        }
        if (state == Escape) {
            state = Normal;
            switch (*pnext) {
                case '"':
                    cxvar_str_catc(str, '"');
                    break;
                case '\\':
                    cxvar_str_catc(str, '\\');
                    break;
                case '/':
                    cxvar_str_catc(str, '/');
                    break;
                case 'b':
                    cxvar_str_catc(str, '\b');
                    break;
                case 'f':
                    cxvar_str_catc(str, '\f');
                    break;
                case 'n':
                    cxvar_str_catc(str, '\n');
                    break;
                case 'r':
                    cxvar_str_catc(str, '\r');
                    break;
                case 't':
                    cxvar_str_catc(str, '\t');
                    break;
                case 'u':
                    if (pend - pnext < 4) {
                        return 1;
                    }
                    state = Ucode;
                    ucount = 0;
                    break;
                default:
                    return 1;
            }
            pnext++;
            continue;
        }
        if (state == Ucode) {
            if (!isxdigit(*pnext)) {
                return 1;
            }
            xdigits[ucount++] = *pnext;
            pnext++;
            if (ucount < 4) {
                continue;
            }

            xdigits[5] = 0;
            long int cp = strtol(xdigits, NULL, 16);
            cxvar_str_catcp(str, cp);
            state = Normal;
            continue;
        }
        assert(0);
    }
    return 0;
}

static int cx_json_parse_str(ParseState* ps, CxVar* var) {

    cxvar_str str;
    CHK(cx_json_parse_str_token(ps, &str));
    *var = cx_var_new_str(str.data, ps->alloc);
    ps->tok++;
    return 0;
}

static int cx_json_parse_arr(ParseState* ps, CxVar* var) {

    size_t arr_len = ps->tok->size;
    *var = cx_var_new_arr(ps->alloc);
    ps->tok++;
    while (arr_len) {
        CxVar el;
        CHK(cx_json_parse_token(ps, &el));
        CHK(cx_var_arr_push(var, el));
        arr_len--;
    }
    return 0;
}

static int cx_json_parse_obj(ParseState* ps, CxVar* var) {

    size_t obj_len = ps->tok->size;
    *var = cx_var_new_map(ps->alloc);
    ps->tok++;
    for (; obj_len > 0; obj_len--) {
        // Key
        cxvar_str key;
        CHK(cx_json_parse_str_token(ps, &key));
        ps->tok++;
        // Value
        CxVar value;
        CHK(cx_json_parse_token(ps, &value));
        CHK(cx_var_map_setn(var, key.data, cxvar_str_len(&key), value));
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
