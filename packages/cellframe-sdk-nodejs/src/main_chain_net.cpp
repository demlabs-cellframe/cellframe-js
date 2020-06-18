#include <assert.h>
#include <node_api.h>
#include <uv.h>

extern "C" {
#include <dap_chain_net.h>
#include <dap_chain_node_cli.h>
}

#include "utils.h"
#include "config.h"
#include "chain_net.h"

#define LOG_TAG "main_chain_net_nodejs"


/*
    Chain Net
*/


napi_value js_dap_chain_net_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_chain_net_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_net_deinit(napi_env env, napi_callback_info info)
{
    dap_chain_net_deinit();

    return nullptr;
}


/*
    Chain Node CLI
*/


struct CommandData {
    int argc;
    char **argv;
    char **str_reply;
    int result;
};


napi_value js_dap_chain_node_cli_init(napi_env env, napi_callback_info info)
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

    int result = dap_chain_node_cli_init(config->config_);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_node_cli_delete(napi_env env, napi_callback_info info)
{
    dap_chain_node_cli_delete();

    return nullptr;
}

int cmd_function(int argc, char **argv, void* context, char **str_reply)
{
    CommandData cmd_data;
    cmd_data.argc = argc;
    cmd_data.argv = argv;
    cmd_data.str_reply = str_reply;

    native_callback(&cmd_data, (CallbackContext*)context);

    return cmd_data->result;
}

static void arguments_converter(napi_env env, napi_value js_context, void* data, int* out_argc, napi_value** out_argv)
{
    CommandData* cmd_data = (CommandData*)data;

    int argc = cmd_data->argc;
    char** argv = cmd_data->argv;

    int cmd_argc = argc + 1;
    napi_value *cmd_argv = new napi_value[cmd_argc];
    napi_status status;

    cmd_argv[0] = js_context;

    for (int i = 0; i < argc; ++i)
    {
        CHECK(napi_create_string_utf8(env, argv[i], NAPI_AUTO_LENGTH, cmd_argv + i + 1));
    }

    *out_argc = cmd_argc;
    *out_argv = cmd_argv;
}

static void result_converter(napi_env env, napi_value js_result, void* data)
{
    napi_valuetype arg_type;
    napi_status status;
    int result = 0;
    bool exception_is_pending;

    CommandData* cmd_data = (CommandData*)data;

    CHECK(napi_is_exception_pending(env, &exception_is_pending));
    CHECK(napi_typeof(env, js_result, &arg_type));

    if (exception_is_pending)
    {
        const napi_extended_error_info *error_info;
        CHECK(napi_get_last_error_info(env, &error_info));

        log_it(L_ERROR, error_info->error_message);
        result = error_info->engine_error_code;
    }
    else if (arg_type != napi_string)
    {
        log_it(L_ERROR, "Value returned from JS callback is not a string");
        result = 0xDEADBEAF;
    }
    else if (cmd_data->str_reply)
    {
        *cmd_data->str_reply = extract_str(env, js_result, nullptr);
    }

    cmd_data->result = result;
}

napi_value js_dap_chain_node_cli_cmd_item_create(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(5)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_function)
    //             2  context can be null, undefined or object
    ARG_TYPE_CHECK(3, napi_string)
    ARG_TYPE_CHECK(4, napi_string)

    char* cmd_name_buffer = extract_str(env, args[0], nullptr);
    char* cmd_doc_buffer = extract_str(env, args[3], nullptr);
    char* cmd_doc_ex_buffer = extract_str(env, args[4], nullptr);

    CallbackContext *cmd_context = nullptr;
    // Will check type of a context
    int res = create_callback_context(env, args[1], args[0], args[2], arguments_converter, result_converter, &cmd_context);
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
        dap_chain_node_cli_cmd_item_create(cmd_name_buffer, cmd_function, cmd_context, cmd_doc_buffer, cmd_doc_ex_buffer);
    }

    delete[] cmd_name_buffer;
    delete[] cmd_doc_buffer;
    delete[] cmd_doc_ex_buffer;

    return js_result;
}


/*
*/


napi_value js_dap_chain_net_list(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    uint16_t size;
    dap_chain_net_t** result = dap_chain_net_list(&size);

    CHECK(napi_create_array_with_length(env, size, &js_result));

    napi_value empty_string;
    CHECK(napi_create_string_utf8(env, "", 0, &empty_string));

    ChainNet* obj = nullptr;
    napi_value tmp_value;
    for (uint16_t i = 0; i < size; ++i)
    {
        CreateInstance<ChainNet>(env, 1, &empty_string, &tmp_value);
        CHECK(napi_unwrap(env, tmp_value, reinterpret_cast<void**>(&obj)));
        obj->chain_net_ = result[i];
        CHECK(napi_set_element(env, js_result, i, tmp_value));
    }

    return js_result;
}


/*
*/


napi_value ChainNetInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_chain_net_init", js_dap_chain_net_init),
        DECLARE_NAPI_METHOD("dap_chain_net_deinit", js_dap_chain_net_deinit),
        DECLARE_NAPI_METHOD("dap_chain_node_cli_init", js_dap_chain_node_cli_init),
        DECLARE_NAPI_METHOD("dap_chain_node_cli_delete", js_dap_chain_node_cli_delete),
        DECLARE_NAPI_METHOD("dap_chain_node_cli_cmd_item_create", js_dap_chain_node_cli_cmd_item_create),
        DECLARE_NAPI_METHOD("dap_chain_net_list", js_dap_chain_net_list),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    exports = ChainNet::Init(env, exports);

    return exports;
}
