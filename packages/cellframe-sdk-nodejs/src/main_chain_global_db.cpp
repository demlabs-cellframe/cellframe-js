#include <assert.h>
#include <node_api.h>
#include <uv.h>

extern "C" {
#include <dap_chain_global_db.h>
}

#include "utils.h"
#include "config.h"


#define LOG_TAG "main_chain_global_db_nodejs"


/*
    Chain Net Service
*/


napi_value js_dap_chain_global_db_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    bool isConfig = InstanceOf<Config>(env, args[0]);
    if (isConfig == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong argument: Config instance expected");
        return nullptr;
    }

    Config* config;
    CHECK(napi_unwrap(env, args[0], reinterpret_cast<void**>(&config)));

    int result = dap_chain_global_db_init(config->config_);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_global_db_deinit(napi_env env, napi_callback_info info)
{
    dap_chain_global_db_deinit();

    return nullptr;
}


/*
*/


napi_value ChainGlobalDBInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_chain_global_db_init", js_dap_chain_global_db_init),
        DECLARE_NAPI_METHOD("dap_chain_global_db_deinit", js_dap_chain_global_db_deinit),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
