#include <assert.h>
#include <node_api.h>
#include <uv.h>

extern "C" {
#include <dap_chain_cs_dag.h>
#include <dap_chain_cs_dag_poa.h>
#include <dap_chain_cs_dag_pos.h>
}

#include "utils.h"


#define LOG_TAG "main_chain_cs_dag_nodejs"


/*
    Chain Node CS DAG
*/


napi_value js_dap_chain_cs_dag_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_chain_cs_dag_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_cs_dag_deinit(napi_env env, napi_callback_info info)
{
    dap_chain_cs_dag_deinit();

    return nullptr;
}

napi_value js_dap_chain_cs_dag_poa_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_chain_cs_dag_poa_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_cs_dag_poa_deinit(napi_env env, napi_callback_info info)
{
    dap_chain_cs_dag_poa_deinit();

    return nullptr;
}

napi_value js_dap_chain_cs_dag_pos_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_chain_cs_dag_pos_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_cs_dag_pos_deinit(napi_env env, napi_callback_info info)
{
    dap_chain_cs_dag_pos_deinit();

    return nullptr;
}


/*
*/


napi_value ChainCSDAGInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_chain_cs_dag_init", js_dap_chain_cs_dag_init),
        DECLARE_NAPI_METHOD("dap_chain_cs_dag_deinit", js_dap_chain_cs_dag_deinit),
        DECLARE_NAPI_METHOD("dap_chain_cs_dag_poa_init", js_dap_chain_cs_dag_poa_init),
        DECLARE_NAPI_METHOD("dap_chain_cs_dag_poa_deinit", js_dap_chain_cs_dag_poa_deinit),
        DECLARE_NAPI_METHOD("dap_chain_cs_dag_pos_init", js_dap_chain_cs_dag_pos_init),
        DECLARE_NAPI_METHOD("dap_chain_cs_dag_pos_deinit", js_dap_chain_cs_dag_pos_deinit),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
