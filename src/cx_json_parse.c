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
    cxvar_str   sprim;      // Temporary aux string for primitives
    cxvar_str   skey;       // Temporary aux string for keys
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
        .sprim = cxvar_str_init(alloc),
        .skey = cxvar_str_init(alloc),
    };

    while (ps.tok <= ps.tok_last) {
        res = cx_json_parse_token(&ps, var);
        if (res) {
            break;
        }
    }

    cxvar_str_free(&ps.sprim);
    cxvar_str_free(&ps.skey);
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

    // Copy current primitive to auxiliary string
    cxvar_str_clear(&ps->sprim);
    const char* pstart = &ps->data[ps->tok->start];
    cxvar_str_cpyn(&ps->sprim, pstart, ps->tok->end - ps->tok->start);

    if (cxvar_str_cmp(&ps->sprim, "null") == 0) {
        *var = cx_var_new_null();
        ps->tok++;
        return 0;
    }

    if (cxvar_str_cmp(&ps->sprim, "true") == 0) {
        *var = cx_var_new_bool(true);
        ps->tok++;
        return 0;
    }

    if (cxvar_str_cmp(&ps->sprim, "false") == 0) {
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
        if (cxvar_str_find(&ps->sprim, ".") >= 0) {
            const double v = strtod(ps->sprim.data, &pend);
            *var = cx_var_new_float(v);
        } else {
            const int64_t v = strtod(ps->sprim.data, &pend);
            *var = cx_var_new_int(v);
        }

        // Checks for invalid number chars
        if (ps->sprim.data + cxvar_str_len(&ps->sprim) != pend) {
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

            xdigits[4] = 0;
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

    cxvar_str_clear(&ps->sprim);
    CHK(cx_json_parse_str_token(ps, &ps->sprim));
    *var = cx_var_new_str(ps->sprim.data, ps->alloc);
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
        CHK(cx_var_push_arr_val(var, el));
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
        cxvar_str_clear(&ps->skey);
        CHK(cx_json_parse_str_token(ps, &ps->skey));
        ps->tok++;
        // Value
        CxVar value;
        CHK(cx_json_parse_token(ps, &value));
        CHK(cx_var_set_map_val2(var, ps->skey.data, cxvar_str_len(&ps->skey), value));
    }
    return 0;
}

