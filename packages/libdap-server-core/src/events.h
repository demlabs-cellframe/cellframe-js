#pragma once

#include <node_api.h>
extern "C" {
#include <dap_events.h>
}

class Events
{
public:
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

private:
    explicit Events();
    ~Events();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Wait(napi_env env, napi_callback_info info);
    static napi_value ThreadWakeUp(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

    dap_events_t* events_;
};

napi_value js_dap_events_init(napi_env env, napi_callback_info info);
napi_value js_dap_events_deinit(napi_env env, napi_callback_info info);
