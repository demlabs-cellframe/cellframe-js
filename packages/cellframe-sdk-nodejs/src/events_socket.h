#pragma once

#include <node_api.h>
extern "C" {
#include <dap_events.h>
#include <dap_events_socket.h>
}

class EventsSocket
{
public:
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

private:
    explicit EventsSocket(int sock, struct dap_events * sh);
    ~EventsSocket();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value IsReadyToRead(napi_env env, napi_callback_info info);
    static napi_value IsReadyToWrite(napi_env env, napi_callback_info info);
    static napi_value SetReadable(napi_env env, napi_callback_info info);
    static napi_value SetWritable(napi_env env, napi_callback_info info);
    static napi_value Write(napi_env env, napi_callback_info info);
    static napi_value Read(napi_env env, napi_callback_info info);
    static napi_value Remove(napi_env env, napi_callback_info info);
    static napi_value Kill(napi_env env, napi_callback_info info);
    static napi_value ShrinkBufIn(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

    dap_events_socket_t* events_socket_;
};

napi_value js_dap_events_socket_init(napi_env env, napi_callback_info info);
napi_value js_dap_events_socket_deinit(napi_env env, napi_callback_info info);
