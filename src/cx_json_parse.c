#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include "cx_alloc.h"

#define cx_str_name cxstr
#define cx_str_static
#define cx_str_instance_allocator
#define cx_str_implement
#include "cx_str.h"

#include "cx_var.h"
#include "cx_json_parse.h"

#include "jsmn.h"

typedef struct ParseState {
    const CxAllocator* alloc;
    const char* data;       // JSON string to parse
    int         count;      // Number of tokens
    jsmntok_t*  tok;        // Pointer to current token
    jsmntok_t*  tok_last;   // Pointer to last token
    cxstr       sprim;      // Temporary aux string for primitives
    cxstr       skey;       // Temporary aux string for keys
} ParseState;

static int cx_json_parse_token(ParseState* ps, CxVar** var);
static int cx_json_parse_prim(ParseState* ps, CxVar** var);
static int cx_json_parse_str_token(ParseState* ps, cxstr* str);
static int cx_json_parse_str(ParseState* ps, CxVar** var);
static int cx_json_parse_arr(ParseState* ps, CxVar** var);
static int cx_json_parse_obj(ParseState* ps, CxVar** var);

#define CHK(CALL) {int res = CALL; if (res) {return res;}}

int cx_json_parse(const char* data, size_t len, CxVar** var, const CxAllocator* alloc) {

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
        .sprim = cxstr_init(alloc),
        .skey = cxstr_init(alloc),
    };

    while (ps.tok <= ps.tok_last) {
        res = cx_json_parse_token(&ps, var);
        if (res) {
            break;
        }
    }

    cxstr_free(&ps.sprim);
    cxstr_free(&ps.skey);
    cx_alloc_free(alloc, tokens, nalloc); 
    return res;
}

// Parses the current token
static int cx_json_parse_token(ParseState* ps, CxVar** var) {

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


static int cx_json_parse_prim(ParseState* ps, CxVar** var) {

    // Copy current primitive to auxiliary string
    cxstr_clear(&ps->sprim);
    const char* pstart = &ps->data[ps->tok->start];
    cxstr_cpyn(&ps->sprim, pstart, ps->tok->end - ps->tok->start);

    if (cxstr_cmp(&ps->sprim, "null") == 0) {
        cx_var_set_null(*var);
        ps->tok++;
        return 0;
    }

    if (cxstr_cmp(&ps->sprim, "true") == 0) {
        cx_var_set_bool(*var, true);
        ps->tok++;
        return 0;
    }

    if (cxstr_cmp(&ps->sprim, "false") == 0) {
        cx_var_set_bool(*var, false);
        ps->tok++;
        return 0;
    }

    int start = pstart[0];
    if (start == '-' || start >= '0' && start <= '9') {
        // If number string contains '.' try to convert to double,
        // Otherwise try to convert to long int.
        char* pend;
        errno = 0;
        if (cxstr_find(&ps->sprim, ".") >= 0) {
            const double v = strtod(ps->sprim.data, &pend);
            cx_var_set_float(*var, v);
        } else {
            const int64_t v = strtod(ps->sprim.data, &pend);
            cx_var_set_int(*var, v);
        }

        // Checks for invalid number chars
        if (ps->sprim.data + cxstr_len(&ps->sprim) != pend) {
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

static int cx_json_parse_str_token(ParseState* ps, cxstr* str) {

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
            cxstr_catc(str, *pnext);
            pnext++;
            continue;
        }
        if (state == Escape) {
            state = Normal;
            switch (*pnext) {
                case '"':
                    cxstr_catc(str, '"');
                    break;
                case '\\':
                    cxstr_catc(str, '\\');
                    break;
                case '/':
                    cxstr_catc(str, '/');
                    break;
                case 'b':
                    cxstr_catc(str, '\b');
                    break;
                case 'f':
                    cxstr_catc(str, '\f');
                    break;
                case 'n':
                    cxstr_catc(str, '\n');
                    break;
                case 'r':
                    cxstr_catc(str, '\r');
                    break;
                case 't':
                    cxstr_catc(str, '\t');
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
            cxstr_catcp(str, cp);
            state = Normal;
            continue;
        }
        assert(0);
    }
    return 0;
}

static int cx_json_parse_str(ParseState* ps, CxVar** var) {

    cxstr_clear(&ps->sprim);
    CHK(cx_json_parse_str_token(ps, &ps->sprim));
    cx_var_set_str(*var, ps->sprim.data);
    ps->tok++;
    return 0;
}

static int cx_json_parse_arr(ParseState* ps, CxVar** var) {

    size_t arr_len = ps->tok->size;
    cx_var_set_arr(*var);
    ps->tok++;
    while (arr_len) {
        CxVar* el = cx_var_new(cx_var_allocator(*var));
        CHK(cx_json_parse_token(ps, &el));
        cx_var_push_arr_val(*var, el);
        arr_len--;
    }
    return 0;
}

static int cx_json_parse_obj(ParseState* ps, CxVar** var) {

    size_t obj_len = ps->tok->size;
    cx_var_set_map(*var);
    ps->tok++;
    for (; obj_len > 0; obj_len--) {
        // Key
        cxstr_clear(&ps->skey);
        CHK(cx_json_parse_str_token(ps, &ps->skey));
        ps->tok++;
        // Value
        CxVar* value = cx_var_new(cx_var_allocator(*var));
        CHK(cx_json_parse_token(ps, &value));
        cx_var_set_map_valn(*var, ps->skey.data, cxstr_len(&ps->skey), value);
    }
    return 0;
}

