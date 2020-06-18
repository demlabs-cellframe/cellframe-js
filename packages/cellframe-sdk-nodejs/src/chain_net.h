#pragma once

#include <node_api.h>
extern "C" {
#include <dap_chain_net.h>
}

struct ChainNet
{
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

    explicit ChainNet(char* net_name);
    ~ChainNet();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value ProcMempool(napi_env env, napi_callback_info info);
    static napi_value GoSync(napi_env env, napi_callback_info info);
    static napi_value SyncGDB(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

    dap_chain_net_t* chain_net_;
};

