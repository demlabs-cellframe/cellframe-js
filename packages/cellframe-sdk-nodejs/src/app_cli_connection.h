#pragma once

#include <node_api.h>
extern "C" {
#include <dap_app_cli_net.h>
}

struct AppCLIConnection
{
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

    explicit AppCLIConnection(char* socket_path);
    ~AppCLIConnection();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value PostCommand(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);
    static napi_value ShellReaderLoop(napi_env env, napi_callback_info info);
    static napi_value IsOK(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

    dap_app_cli_connect_param_t* connection_;
};

