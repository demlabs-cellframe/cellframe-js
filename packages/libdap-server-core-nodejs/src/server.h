#pragma once

#include <node_api.h>
extern "C" {
#include <dap_server.h>
}

struct Server
{
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
    static bool InstanceOf(napi_env env, napi_value object);

    explicit Server(const char *addr, uint16_t port, dap_server_type_t type);
    ~Server();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value Loop(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

    dap_server_t* server_;
};

napi_value js_dap_server_init(napi_env env, napi_callback_info info);
napi_value js_dap_server_deinit(napi_env env, napi_callback_info info);
