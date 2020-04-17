#include <assert.h>
#include <node_api.h>

extern "C" {
#include <dap_chain_node_cli.h>
}

#include "utils.h"
#include "config.h"




/*
    Chain Node CLI
*/


napi_value js_dap_chain_node_cli_init(napi_env env, napi_callback_info info)
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

    int result = dap_chain_node_cli_init(config->config_);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_node_cli_delete(napi_env env, napi_callback_info info)
{
    dap_chain_node_cli_delete();

    return nullptr;
}


/*
*/


napi_value ChainNetInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_chain_node_cli_init", js_dap_chain_node_cli_init),
        DECLARE_NAPI_METHOD("dap_chain_node_cli_delete", js_dap_chain_node_cli_delete),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
