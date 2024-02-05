#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "cx_var.h"
#include "cx_json.h"

#define WRITE(W,D,L)    W->write(W->ctx,D,L)
#define WRITESTR(W,S)   W->write(W->ctx,S,strlen(S))


int cx_json_build(const CxVar* json, CxJsonBuildCfg* cfg, CxWriter* out) {

    char fmtbuf[256];
    switch(cx_var_get_type(json)) {

        case CxVarNull:
            WRITESTR(out, "null");
            break;

        case CxVarBool: {
            bool val;
            cx_var_get_bool(json, &val);
            if (val) {
                WRITESTR(out, "true");
            } else {
                WRITESTR(out, "false");
            }
            break;
        }

        case CxVarInt: {
            int64_t val;
            cx_var_get_int(json, &val);
            snprintf(fmtbuf, sizeof(fmtbuf), "%zd", val);
            WRITESTR(out, fmtbuf);
        }

        default:
            return 1;
    }
    return 0;



}




