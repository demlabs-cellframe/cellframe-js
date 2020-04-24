#include <assert.h>
#include <node_api.h>
#include <uv.h>

extern "C" {
#include <dap_chain_gdb.h>
}

#include "utils.h"


#define LOG_TAG "main_chain_gdb_nodejs"


/*
    Chain Node GDB
*/


napi_value js_dap_chain_gdb_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_chain_gdb_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}


/*
*/


napi_value ChainGDBInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_chain_gdb_init", js_dap_chain_gdb_init),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
