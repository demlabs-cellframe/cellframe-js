#include <assert.h>
#include <node_api.h>

extern "C" {
#include <dap_http.h>
#include <dap_enc_http.h>
}

#include "utils.h"
#include "server.h"




/*
    HTTP
*/


napi_value js_dap_http_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_http_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_http_deinit(napi_env env, napi_callback_info info)
{
    dap_http_deinit();

    return nullptr;
}

napi_value js_dap_http_new(napi_env env, napi_callback_info info)
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

    int result = dap_http_new(server->server_, buffer);

    delete[] buffer;

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}


/*
    ENC HTTP
*/


napi_value js_enc_http_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = enc_http_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_enc_http_deinit(napi_env env, napi_callback_info info)
{
    enc_http_deinit();

    return nullptr;
}

napi_value js_enc_http_add_proc(napi_env env, napi_callback_info info)
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

    enc_http_add_proc(DAP_HTTP(server->server_), buffer);

    delete[] buffer;

    return js_result;
}


/*
*/


napi_value ServerInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_http_init", js_dap_http_init),
        DECLARE_NAPI_METHOD("dap_http_deinit", js_dap_http_deinit),
        DECLARE_NAPI_METHOD("dap_http_new", js_dap_http_new),
        DECLARE_NAPI_METHOD("enc_http_init", js_enc_http_init),
        DECLARE_NAPI_METHOD("enc_http_deinit", js_enc_http_deinit),
        DECLARE_NAPI_METHOD("enc_http_add_proc", js_enc_http_add_proc),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
