#include <cmath>
#include <node_api.h>
#include <dap_enc_key.h>
#include <dap_enc_base64.h>
#include <dap_enc_base58.h>
extern "C" {
#include <dap_hash.h>
}
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
    BASE64
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
    BASE58
*/


#define BASE58_ENCODE_SIZE(a_in_size) ((size_t) (138 * (a_in_size) / 100 + 1))
#define BASE58_DECODE_SIZE(a_in_size) ((size_t) ((a_in_size) * 733 / 1000 + 1))

napi_value js_dap_enc_base58_decode(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_string)

    size_t buffer_size;
    char* str_buffer = extract_str(env, args[0], &buffer_size);

    void* result_buffer = nullptr;
    //size_t result_buffer_size = BASE58_DECODE_SIZE(buffer_size);
    size_t result_buffer_size = DAP_ENC_BASE58_DECODE_SIZE(buffer_size);
    CHECK(napi_create_arraybuffer(env, result_buffer_size, &result_buffer, &js_result));

    size_t result_size = dap_enc_base58_decode(str_buffer, result_buffer);

    delete[] str_buffer;

    return js_result;
}

napi_value js_dap_enc_base58_encode(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, args[0], &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "ArrayBuffer expected");
        return nullptr;
    }

    size_t buffer_size;
    void* buffer = nullptr;
    CHECK(napi_get_arraybuffer_info(env, args[0], &buffer, &buffer_size));

    //size_t result_buffer_size = BASE58_ENCODE_SIZE(buffer_size);
    size_t result_buffer_size = DAP_ENC_BASE58_ENCODE_SIZE(buffer_size);
    char* result_buffer = new char[result_buffer_size];

    size_t result_size = dap_enc_base58_encode(buffer, buffer_size, result_buffer);

    CHECK(napi_create_string_utf8(env, result_buffer, result_size, &js_result));

    delete[] result_buffer;

    return js_result;
}


/*
    HASH
*/


void create_fast_hash_typed_array(napi_env env, void**hash_buffer, napi_value* hash_typed_array)
{
    napi_status status;
    napi_value hash_array_buffer;
    CHECK(napi_create_arraybuffer(env, DAP_CHAIN_HASH_FAST_SIZE, hash_buffer, &hash_array_buffer));
    CHECK(napi_create_typedarray(env, napi_uint8_array, DAP_CHAIN_HASH_FAST_SIZE, hash_array_buffer, 0, hash_typed_array));
}


napi_value js_dap_chain_str_to_hash_fast(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_string)

    void* hash_buffer;
    napi_value hash_typed_array;
    create_fast_hash_typed_array(env, &hash_buffer, &hash_typed_array);

    size_t str_size;
    char* str_buffer = extract_str(env, args[0], &str_size);

    int result = dap_chain_str_to_hash_fast(str_buffer, (dap_chain_hash_fast_t*)hash_buffer);

    delete[] str_buffer;

    if (result == -1)
    {
        napi_throw_error(env, nullptr, "Wrong string size");
        return nullptr;
    }
    else if (result < 0)
    {
        char error_msg_buffer[256] = {0};
        sprintf(error_msg_buffer, "Wrong char at %d", (result / -10));
        napi_throw_error(env, nullptr, error_msg_buffer);
        return nullptr;
    }

    return hash_typed_array;
}


napi_value js_dap_hash_fast(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    // Argument #0
    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, args[0], &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "ArrayBuffer expected");
        return nullptr;
    }

    size_t buffer_size;
    void* buffer = nullptr;
    CHECK(napi_get_arraybuffer_info(env, args[0], &buffer, &buffer_size));

    // Result
    void* hash_buffer;
    napi_value hash_typed_array;
    create_fast_hash_typed_array(env, &hash_buffer, &hash_typed_array);

    dap_hash_fast(buffer, buffer_size, (dap_chain_hash_fast_t*)hash_buffer);

    return hash_typed_array;
}

napi_value js_dap_chain_hash_fast_to_str(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    // Argument #0
    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, args[0], &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "ArrayBuffer expected");
        return nullptr;
    }

    size_t buffer_size;
    void* buffer = nullptr;
    CHECK(napi_get_arraybuffer_info(env, args[0], &buffer, &buffer_size));

    // Result
    const size_t result_size = DAP_CHAIN_HASH_FAST_SIZE * 2 + 2;
    char result_buffer[result_size];

    size_t actual_size = dap_chain_hash_fast_to_str((dap_chain_hash_fast_t*)buffer, result_buffer, result_size);

    CHECK(napi_create_string_utf8(env, result_buffer, result_size, &js_result));

    return js_result;
}

napi_value js_dap_hash_fast_compare(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_object)
    ARG_TYPE_CHECK(1, napi_object)

    // Argument #0
    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, args[0], &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "Argument #0: ArrayBuffer expected");
        return nullptr;
    }

    size_t buffer_size1;
    void* buffer1 = nullptr;
    CHECK(napi_get_arraybuffer_info(env, args[0], &buffer1, &buffer_size1));

    // Argument #1
    CHECK(napi_is_arraybuffer(env, args[1], &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "Argument #1: ArrayBuffer expected");
        return nullptr;
    }

    size_t buffer_size2;
    void* buffer2 = nullptr;
    CHECK(napi_get_arraybuffer_info(env, args[1], &buffer2, &buffer_size2));

    // Result
    bool result = buffer_size1 == buffer_size2 && !memcmp(buffer1, buffer2, buffer_size1);

    CHECK(napi_get_boolean(env, result, &js_result));

    return js_result;
}

napi_value js_dap_hash_fast_is_blank(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    // Argument #0
    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, args[0], &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "ArrayBuffer expected");
        return nullptr;
    }

    size_t buffer_size;
    void* buffer = nullptr;
    CHECK(napi_get_arraybuffer_info(env, args[0], &buffer, &buffer_size));

    if (buffer_size != DAP_CHAIN_HASH_FAST_SIZE)
    {
        napi_throw_type_error(env, nullptr, "Wrong ArrayBuffer size.");
        return nullptr;
    }

    // Result
    bool result = dap_hash_fast_is_blank((dap_chain_hash_fast_t*)buffer);

    CHECK(napi_get_boolean(env, result, &js_result));

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
        DECLARE_NAPI_METHOD("dap_enc_base58_decode", js_dap_enc_base58_decode),
        DECLARE_NAPI_METHOD("dap_enc_base58_encode", js_dap_enc_base58_encode),
        DECLARE_NAPI_METHOD("dap_chain_str_to_hash_fast", js_dap_chain_str_to_hash_fast),
        DECLARE_NAPI_METHOD("dap_hash_fast", js_dap_hash_fast),
        DECLARE_NAPI_METHOD("dap_chain_hash_fast_to_str", js_dap_chain_hash_fast_to_str),
        DECLARE_NAPI_METHOD("dap_hash_fast_compare", js_dap_hash_fast_compare),
        DECLARE_NAPI_METHOD("dap_hash_fast_is_blank", js_dap_hash_fast_is_blank),
    };
    status = napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc);
    assert(status == napi_ok);

    return exports;
}


NAPI_MODULE(libdap_crypto_nodejs, Init)
