#include <cmath>
#include <node_api.h>
#include <dap_enc_key.h>
#include <dap_enc_base64.h>
#include <assert.h>
#include "utils.h"

#define CHECK(x) status = (x); assert(status == napi_ok);

// WARNING: keep in sync with dap_enc_key.h
bool is_dap_enc_data_type_valid(int32_t value)
{
    return value >= dap_enc_data_type_t::DAP_ENC_DATA_TYPE_RAW &&
           value <= dap_enc_data_type_t::DAP_ENC_DATA_TYPE_B64_URLSAFE;
}

// Copy from dap_enc_base64.c
size_t B64_GetSize(size_t sz, bool isEncode)
{
    size_t n = 0;

    if( isEncode ) {
        n = ceil ( ((double) sz) / 3.0 ) * 4.0;
        switch( sz % 3 ) {
        case 0: break;
        case 1: n += 2; break;
        case 2: n += 3; break;
        }
    }
    else {
        n = ceil ( ((double) sz) / 4.0 ) * 3.0;
        switch( sz % 4 ) {
        case 0: break;
        case 1: break;
        case 2: n += 1; break;
        case 3: n += 2; break;
        }
    }
    return n;
}


/*
*/


napi_value js_dap_enc_base64_decode(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_number)

    int32_t enum_value;
    CHECK(napi_get_value_int32(env, args[1], &enum_value));

    if (is_dap_enc_data_type_valid(enum_value) == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong enum value");
        return nullptr;
    }

    size_t buffer_size;
    char* str_buffer = extract_str(env, args[0], &buffer_size);

    void* result_buffer = nullptr;
    size_t result_buffer_size = B64_GetSize(buffer_size, false);
    CHECK(napi_create_arraybuffer(env, result_buffer_size, &result_buffer, &js_result));

    // TODO: should we shrink buffer if result_size is smaller than buffer_size?
    size_t result_size = dap_enc_base64_decode(str_buffer, buffer_size, result_buffer, (dap_enc_data_type_t)enum_value);

    delete[] str_buffer;

    return js_result;
}

napi_value js_dap_enc_base64_encode(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_object)
    ARG_TYPE_CHECK(1, napi_number)

    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, args[0], &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "ArrayBuffer expected");
        return nullptr;
    }

    int32_t enum_value;
    CHECK(napi_get_value_int32(env, args[1], &enum_value));

    if (is_dap_enc_data_type_valid(enum_value) == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong enum value");
        return nullptr;
    }

    size_t buffer_size;
    void* buffer = nullptr;
    CHECK(napi_get_arraybuffer_info(env, args[0], &buffer, &buffer_size));

    size_t result_buffer_size = B64_GetSize(buffer_size, true);
    char* result_buffer = new char[result_buffer_size];

    size_t result_size = dap_enc_base64_encode(buffer, buffer_size, result_buffer, (dap_enc_data_type_t)enum_value);

    CHECK(napi_create_string_utf8(env, result_buffer, result_size, &js_result));

    delete[] result_buffer;

    return js_result;
}


/*
*/


static napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_enc_base64_decode", js_dap_enc_base64_decode),
        DECLARE_NAPI_METHOD("dap_enc_base64_encode", js_dap_enc_base64_encode),
    };
    status = napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc);
    assert(status == napi_ok);

    return exports;
}


NAPI_MODULE(libdap_crypto_nodejs, Init)
