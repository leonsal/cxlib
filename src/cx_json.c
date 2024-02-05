#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "cx_var.h"
#include "cx_json.h"


int cx_json_build_str(const CxVar* json, CxJsonBuildCfg* cfg, cxstr* dst) {


    switch(cx_var_get_type(json)) {
        case CxVarNull:
            cxstr_cat(dst, "null");
            break;
        case CxVarBool: {
            bool vb;
            cx_var_get_bool(json, &vb);

            break;
        }
        case CxVarInt:
            break;
        case CxVarFloat:
            break;
        case CxVarStr:
            break;
        case CxVarArr:
            break;
        case CxVarMap:
            break;
        default:
            return 1;
    }






    return 0;
}

