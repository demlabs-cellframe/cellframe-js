#include <assert.h>
#include <node_api.h>

#include <dap_common.h>
#include <dap_config.h>

#include "utils.h"
#include "config.h"


bool is_enum_value_valid(int32_t value)
{
    return value >= dap_log_level_t::L_DEBUG &&
           value <  dap_log_level_t::L_TOTAL;
}


#define LOG_TAG "nodejs"



/*
    DAP COMMON
*/


napi_value js_dap_itoa(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_number)

    int64_t arg_value;
    status = napi_get_value_int64(env, args[0], &arg_value);
    assert(status == napi_ok);

    char* result = dap_itoa(arg_value);

    napi_value js_result;
    status = napi_create_string_utf8(env, result, NAPI_AUTO_LENGTH, &js_result);
    assert(status == napi_ok);

    return js_result;
}


napi_value js_dap_common_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    char* arg_buffer1 = extract_str(env, args[0]);
    char* arg_buffer2 = extract_str(env, args[1]);

    int result = dap_common_init(arg_buffer1, arg_buffer2);
    napi_value js_result;

    delete[] arg_buffer1;
    delete[] arg_buffer2;

    status = napi_create_int(env, result, &js_result);
    assert(status == napi_ok);

    return js_result;
}


napi_value js_dap_common_deinit(napi_env env, napi_callback_info info)
{
    dap_common_deinit();

    return nullptr;
}


napi_value js_dap_log_level_set(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_number)

    int32_t arg_value;
    status = napi_get_value_int32(env, args[0], &arg_value);
    assert(status == napi_ok);

    if (is_enum_value_valid(arg_value) == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong argument value");
        return nullptr;
    }

    dap_log_level_set((dap_log_level_t)arg_value);

    return nullptr;
}


napi_value js_dap_log_it(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_number)
    ARG_TYPE_CHECK(1, napi_string)

    int32_t arg_value;
    status = napi_get_value_int32(env, args[0], &arg_value);
    assert(status == napi_ok);

    if (is_enum_value_valid(arg_value) == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong first argument value");
        return nullptr;
    }

    char* arg_buffer1 = extract_str(env, args[1]);

    log_it((dap_log_level_t)arg_value, arg_buffer1);

    delete[] arg_buffer1;

    return nullptr;
}


/*
    DAP CONFIG
*/


napi_value js_dap_config_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_string)

    char* buffer = extract_str(env, args[0]);

    int result = dap_config_init(buffer);

    delete[] buffer;

    status = napi_create_int(env, result, &js_result);
    assert(status == napi_ok);

    return js_result;
}

napi_value js_dap_config_deinit(napi_env env, napi_callback_info info)
{
    dap_config_deinit();

    return nullptr;
}

napi_value js_dap_config_open(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_string)

    char* buffer = extract_str(env, args[0]);

    void* config = dap_config_open(buffer);

    delete[] buffer;

    return nullptr;
}


static napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_itoa", js_dap_itoa),
        DECLARE_NAPI_METHOD("dap_log_level_set", js_dap_log_level_set),
        DECLARE_NAPI_METHOD("dap_log_it", js_dap_log_it),
        DECLARE_NAPI_METHOD("dap_config_init", js_dap_config_init),
        DECLARE_NAPI_METHOD("dap_config_deinit", js_dap_config_deinit),
        DECLARE_NAPI_METHOD("dap_common_init", js_dap_common_init),
        DECLARE_NAPI_METHOD("dap_common_deinit", js_dap_common_deinit)
    };
    status = napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc);
    assert(status == napi_ok);

    Config::Init(env, exports);

    return exports;
}


NAPI_MODULE(hello, Init)

