#include <assert.h>
#include <node_api.h>

extern "C" {
#include <dap_enc_key.h>
#include <dap_http.h>
#include <dap_stream.h>
#include <dap_stream_ctl.h>
}

#include "utils.h"
#include "server.h"




/*
    Stream
*/


napi_value js_dap_stream_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_boolean)

    bool a_dump_packet_headers;
    CHECK(napi_get_value_bool(env, args[0], &a_dump_packet_headers));

    int result = dap_stream_init(a_dump_packet_headers);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_stream_deinit(napi_env env, napi_callback_info info)
{
    dap_stream_deinit();

    return nullptr;
}

napi_value js_dap_stream_add_proc_http(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_object)
    ARG_TYPE_CHECK(1, napi_string)

    if (InstanceOf<Server>(env, args[0]) == false)
    {
        napi_throw_type_error(env, nullptr, "Server instance expected");
        return nullptr;
    }

    Server* server;
    CHECK(napi_unwrap(env, args[0], reinterpret_cast<void**>(&server)));

    size_t size;
    char* buffer = extract_str(env, args[1], &size);

    dap_stream_add_proc_http(DAP_HTTP(server->server_), buffer);

    delete[] buffer;

    return js_result;
}


/*
*/


napi_value js_dap_stream_ctl_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_number)
    ARG_TYPE_CHECK(1, napi_number)

    int32_t key_type;
    CHECK(napi_get_value_int32(env, args[0], &key_type));
    // WARNING: keep in sync with dap_enc_key.h
    if (key_type < DAP_ENC_KEY_TYPE_IAES || key_type > DAP_ENC_KEY_TYPE_SIG_DILITHIUM)
    {
        napi_throw_type_error(env, nullptr, "Wrong key type: value out of range");
        return nullptr;
    }

    size_t size;
    CHECK(napi_get_value_size(env, args[1], &size));

    int result = dap_stream_ctl_init((dap_enc_key_type_t)key_type, size);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_stream_ctl_deinit(napi_env env, napi_callback_info info)
{
    dap_stream_ctl_deinit();

    return nullptr;
}

napi_value js_dap_stream_ctl_add_proc(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_object)
    ARG_TYPE_CHECK(1, napi_string)

    if (InstanceOf<Server>(env, args[0]) == false)
    {
        napi_throw_type_error(env, nullptr, "Server instance expected");
        return nullptr;
    }

    Server* server;
    CHECK(napi_unwrap(env, args[0], reinterpret_cast<void**>(&server)));

    size_t size;
    char* buffer = extract_str(env, args[1], &size);

    dap_stream_ctl_add_proc(DAP_HTTP(server->server_), buffer);

    delete[] buffer;

    return js_result;
}


/*
*/


napi_value StreamInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_stream_init", js_dap_stream_init),
        DECLARE_NAPI_METHOD("dap_stream_deinit", js_dap_stream_deinit),
        DECLARE_NAPI_METHOD("dap_stream_add_proc_http", js_dap_stream_add_proc_http),
        DECLARE_NAPI_METHOD("dap_stream_ctl_init", js_dap_stream_ctl_init),
        DECLARE_NAPI_METHOD("dap_stream_ctl_deinit", js_dap_stream_ctl_deinit),
        DECLARE_NAPI_METHOD("dap_stream_ctl_add_proc", js_dap_stream_ctl_add_proc),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
