#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "cx_var.h"
#include "cx_writer.h"
#include "cx_json.h"

typedef struct BuildState {
    CxJsonBuildCfg* cfg;
    CxWriter*       out;
} BuildState;

static int cx_json_build_val(BuildState* bs, const CxVar* var);
static int cx_json_build_null(BuildState* bs, const CxVar* var);
static int cx_json_build_bool(BuildState* bs, const CxVar* var);
static int cx_json_build_int(BuildState* bs, const CxVar* var);
static int cx_json_build_float(BuildState* bs, const CxVar* var);
static int cx_json_build_str(BuildState* bs, const CxVar* var);
static int cx_json_build_arr(BuildState* bs, const CxVar* var);

#define CHKRES(CALL) {int res = CALL; if (res) { return res; }}


int cx_json_build(const CxVar* var, CxJsonBuildCfg* cfg, CxWriter* out) {

    BuildState bs = {.cfg = cfg, .out = out};
    return cx_json_build_val(&bs, var);
}


static int cx_json_build_val(BuildState* bs, const CxVar* var) {

    int res;
    switch(cx_var_get_type(var)) {
        case CxVarNull:
            res = cx_json_build_null(bs, var);
            break;
        case CxVarBool:
            res = cx_json_build_bool(bs, var);
            break;
        case CxVarInt:
            res = cx_json_build_int(bs, var);
            break;
        case CxVarFloat:
            res = cx_json_build_float(bs, var);
            break;
        case CxVarStr:
            res = cx_json_build_str(bs, var);
            break;
        case CxVarArr:
            res = cx_json_build_arr(bs, var);
            break;
        default:
            return 1;
    }
    if (res < 0) {
        return 1;
    }
    return 0;
}

static int cx_json_build_null(BuildState* bs, const CxVar* var) {

    return cx_writer_write_str(bs->out, "null");
}

static int cx_json_build_bool(BuildState* bs, const CxVar* var) {

    bool val;
    cx_var_get_bool(var, &val);
    if (val) {
        return cx_writer_write_str(bs->out, "true");
    } else {
        return cx_writer_write_str(bs->out, "false");
    }
}

static int cx_json_build_int(BuildState* bs, const CxVar* var) {

    int64_t val;
    cx_var_get_int(var, &val);
    char fmtbuf[256];
    sprintf(fmtbuf, "%zd", val);
    return cx_writer_write_str(bs->out, fmtbuf);
}

static int cx_json_build_float(BuildState* bs, const CxVar* var) {

    double val;
    cx_var_get_float(var, &val);
    char fmtbuf[256];
    sprintf(fmtbuf, "%f", val);
    return cx_writer_write_str(bs->out, fmtbuf);
}

static int cx_json_build_str(BuildState* bs, const CxVar* var) {

    const char* str;
    cx_var_get_str(var, &str);
    return cx_writer_write_str(bs->out, str);
}

static int cx_json_build_arr(BuildState* bs, const CxVar* var) {

    size_t len;
    cx_var_get_arr_len(var, &len);
    CHKRES(cx_writer_write_str(bs->out, "["));
    for (size_t i = 0; i < len; i++) {
        CxVar el;
        cx_var_get_arr_val(var, i, &el);
        int res = cx_json_build_val(bs, var);
        if (res) { return res; }
        if (i < len - 1) {
            CHKRES(cx_writer_write_str(bs->out, ","));
        }
    }
    CHKRES(cx_writer_write_str(bs->out, "]"));
}

static int cx_json_build_map(BuildState* bs, const CxVar* var) {

    size_t count;
    cx_var_get_map_count(var, &count);

    CHKRES(cx_writer_write_str(bs->out, "{"));
    size_t idx = 0;
    cxvar_map_iter iter = {0};
    while (true) {
        cxvar_map_entry* e = cxvar_map_next(var->v.map, &iter);
        if (e == NULL) {
            break;
        }
        CHKRES(cx_writer_write_str(bs->out, "\""));
        CHKRES(cx_writer_write_str(bs->out, e->key));
        CHKRES(cx_writer_write_str(bs->out, "\":"));
        CHKRES(cx_json_build_val(bs, &e->val));
        if (idx < count - 1) {
            CHKRES(cx_writer_write_str(bs->out, ","));
        }
    }
    return cx_writer_write_str(bs->out, "}");
}

