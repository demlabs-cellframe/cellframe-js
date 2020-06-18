#include <assert.h>
#include <node_api.h>
#include <uv.h>

extern "C" {
#include <dap_chain_global_db.h>
}

#include "utils.h"
#include "config.h"


#define LOG_TAG "main_chain_global_db_nodejs"


/*
    Chain Net Service
*/


napi_value js_dap_chain_global_db_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    bool isConfig = InstanceOf<Config>(env, args[0]);
    if (isConfig == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong argument: Config instance expected");
        return nullptr;
    }

    Config* config;
    CHECK(napi_unwrap(env, args[0], reinterpret_cast<void**>(&config)));

    int result = dap_chain_global_db_init(config->config_);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_global_db_deinit(napi_env env, napi_callback_info info)
{
    dap_chain_global_db_deinit();

    return nullptr;
}


/*
*/


napi_value js_dap_chain_global_db_gr_get(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    char* key_buffer = extract_str(env, args[0], nullptr);
    char* group_buffer = extract_str(env, args[1], nullptr);

    size_t result_size;
    uint8_t* result = dap_chain_global_db_gr_get(key_buffer, &result_size, group_buffer);

    void* buffer;
    CHECK(napi_create_arraybuffer(env, result_size, &buffer, &js_result));

    // TODO: make a second *_db_gr_get function that uses already allocated memory
    memcpy(buffer, result, result_size);
    DAP_DELETE(result);

    delete[] key_buffer;
    delete[] group_buffer;

    return js_result;
}

napi_value js_dap_chain_global_db_gr_set(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(3)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_object)
    ARG_TYPE_CHECK(2, napi_string)

    void* buffer;
    size_t buffer_size;
    if (extract_array_buffer(env, args[1], &buffer, &buffer_size) < 0)
    {
        return nullptr;
    }

    char* key_buffer = extract_str(env, args[0], nullptr);
    char* group_buffer = extract_str(env, args[2], nullptr);

    bool result = dap_chain_global_db_gr_set(key_buffer, buffer, buffer_size, group_buffer);

    CHECK(napi_get_boolean(env, result, &js_result));

    delete[] key_buffer;
    delete[] group_buffer;

    return js_result;
}


/*
*/


struct CallbackData
{
    char a_op_code;
    const char* a_prefix;
    const char* a_group;
    const char* a_key;
    const void* a_value;
    size_t a_value_len;
};

void arguments_converter(napi_env env, napi_value js_context, void* data, int* argc, napi_value** argv)
{
    const int cmd_argc = 6;
    napi_value* cmd_argv = new napi_value[cmd_argc];
    napi_status status;
    CallbackData* cmd_data = (CallbackData*)data;

    void* buffer;

    cmd_argv[0] = js_context;
    CHECK(napi_create_int32(env,       cmd_data->a_op_code,                     cmd_argv + 1));
    CHECK(napi_create_string_utf8(env, cmd_data->a_prefix,    NAPI_AUTO_LENGTH, cmd_argv + 2));
    CHECK(napi_create_string_utf8(env, cmd_data->a_group,     NAPI_AUTO_LENGTH, cmd_argv + 3));
    CHECK(napi_create_string_utf8(env, cmd_data->a_key,       NAPI_AUTO_LENGTH, cmd_argv + 4));
    CHECK(napi_create_arraybuffer(env, cmd_data->a_value_len, &buffer,          cmd_argv + 5));
    memcpy(buffer, cmd_data->a_value, cmd_data->a_value_len);

    *argv = cmd_argv;
    *argc = cmd_argc;
}

void dap_global_db_obj_callback_notify(void * a_arg,
                                       const char a_op_code, const char * a_prefix,
                                       const char * a_group, const char * a_key,
                                       const void * a_value, const size_t a_value_len)
{
    CallbackData cmd_data;
    cmd_data.a_prefix = a_prefix;
    cmd_data.a_group = a_group;
    cmd_data.a_key = a_key;
    cmd_data.a_value = a_value;
    cmd_data.a_value_len = a_value_len;
    cmd_data.a_op_code = a_op_code;

    native_callback(&cmd_data, (CallbackContext*)a_arg);
}

napi_value js_dap_chain_global_db_add_history_group_prefix(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    char* prefix_buffer = extract_str(env, args[0], nullptr);
    char* group_buffer = extract_str(env, args[1], nullptr);

    dap_chain_global_db_add_history_group_prefix(prefix_buffer, group_buffer);

    delete[] prefix_buffer;
    delete[] group_buffer;

    return js_result;
}

napi_value js_dap_chain_global_db_add_history_callback_notify(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(3)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_function)
    //             2  context can be null, undefined or object

    char* prefix_buffer = extract_str(env, args[0], nullptr);

    CallbackContext *cmd_context = nullptr;
    // Will check type of a context
    int res = create_callback_context(env, args[1], args[0], args[2], arguments_converter, nullptr, &cmd_context);
    if (res < 0)
    {
        if (res == -10)
        {
            napi_throw_type_error(env, nullptr, "Wrong context type: must be undefined, null or object");
        }
        else
        {
            napi_throw_type_error(env, nullptr, "Unexpected error");
        }
    }
    else
    {
        dap_chain_global_db_add_history_callback_notify(prefix_buffer, dap_global_db_obj_callback_notify, cmd_context);
    }

    delete[] prefix_buffer;

    return nullptr;
}


/*
*/


napi_value ChainGlobalDBInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_chain_global_db_init", js_dap_chain_global_db_init),
        DECLARE_NAPI_METHOD("dap_chain_global_db_deinit", js_dap_chain_global_db_deinit),
        DECLARE_NAPI_METHOD("dap_chain_global_db_gr_get", js_dap_chain_global_db_gr_get),
        DECLARE_NAPI_METHOD("dap_chain_global_db_gr_set", js_dap_chain_global_db_gr_set),
        DECLARE_NAPI_METHOD("dap_chain_global_db_add_history_group_prefix", js_dap_chain_global_db_add_history_group_prefix),
        DECLARE_NAPI_METHOD("dap_chain_global_db_add_history_callback_notify", js_dap_chain_global_db_add_history_callback_notify),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
