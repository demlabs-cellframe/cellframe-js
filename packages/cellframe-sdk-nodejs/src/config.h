#pragma once

#include <node_api.h>
#include <dap_config.h>

struct Config
{
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);


    explicit Config(const char* app_name = 0);
    ~Config();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value Close(napi_env env, napi_callback_info info);
    static napi_value GetString(napi_env env, napi_callback_info info);
    static napi_value GetStringArray(napi_env env, napi_callback_info info);
    static napi_value GetInt32(napi_env env, napi_callback_info info);
    static napi_value GetInt64(napi_env env, napi_callback_info info);
    static napi_value GetUint64(napi_env env, napi_callback_info info);
    static napi_value GetUint16(napi_env env, napi_callback_info info);
    static napi_value GetDouble(napi_env env, napi_callback_info info);
    static napi_value GetBool(napi_env env, napi_callback_info info);
    static napi_value MakeItGlobal(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

    dap_config_t* config_;
};
