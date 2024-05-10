#include <stdlib.h>

#define json_int_t  int64_t
#include "json.h"
#include "cx_alloc.h"
#include "cx_json_parse.h"


// Forward declarations of local functions
static CxError cx_json_val2var(const CxJsonParseCfg* cfg, json_value* jval, CxVar* var);
static inline void cx_json_parse_replacer(CxVar* val, void* userdata){};


CxError cx_json_parse(const char* data, size_t len, CxVar* var, const CxJsonParseCfg* pcfg) {

    // Sets configuration
    CxJsonParseCfg cfg = {0};
    if (pcfg) {
        cfg = *pcfg;
    }
    if (cfg.alloc == NULL) {
        cfg.alloc = cx_def_allocator();
    }
    if (cfg.replacer_fn == NULL) {
        cfg.replacer_fn = cx_json_parse_replacer;
    }

    // Uses json-parser to parse specified JSON data
    json_settings settings = {0};
    if (cfg.comments) {
        settings.settings |=json_enable_comments;
    }
    char errmsg[json_error_max+1];
    json_value* jval = json_parse_ex(&settings, data, len, errmsg);
    if (jval == NULL) {
        return CXERROR(1, errmsg);
    }

    // Converts json-parser return value to CxVar
    CxError err = cx_json_val2var(&cfg, jval, var); 
    json_value_free(jval);
    return err;
}

static CxError cx_json_val2var(const CxJsonParseCfg* cfg, json_value* jval, CxVar* var) {

    switch (jval->type) {
        case json_none:
            break;
        case json_object:
            cx_var_set_map(var);
            for (size_t i = 0; i < jval->u.object.length; i++) {
                json_object_entry src = jval->u.object.values[i];
                CxVar* dst = cx_var_set_map_null(var, src.name);
                cx_json_val2var(cfg, src.value, dst);
            }
            break;
        case json_array:
            cx_var_set_arr(var);
            for (size_t i = 0; i < jval->u.array.length; i++) {
                json_value* src = jval->u.array.values[i];
                CxVar* dst = cx_var_push_arr_null(var);
                cx_json_val2var(cfg, src, dst);
            }
            break;
        case json_integer:
            cx_var_set_int(var, jval->u.integer);
            break;
        case json_double:
            cx_var_set_float(var, jval->u.dbl);
            break;
        case json_string:
            cx_var_set_strn(var, jval->u.string.ptr, jval->u.string.length);
            break;
        case json_boolean:
            cx_var_set_bool(var, jval->u.boolean);
            break;
        case json_null:
            cx_var_set_null(var);
            break;
        default:
            return CXERROR(1, "invalid json type");
    }
    cfg->replacer_fn(var, cfg->replacer_data);
    return CXERROR_OK();
}

static void* json_parse_alloc(size_t size, int zero, void* user_data) {

    CxJsonParseCfg* cfg = user_data;
    if (zero) {
        return cx_alloc_mallocz(cfg->alloc, size); 
    }
    return cx_alloc_malloc(cfg->alloc, size); 
}


   // void * (* mem_alloc) (size_t, int zero, void * user_data);
   // void (* mem_free) (void *, void * user_data);
   //
   // void * user_data;  /* will be passed to mem_alloc and mem_free */


