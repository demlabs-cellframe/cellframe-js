#include <assert.h>
#include <node_api.h>
#include <uv.h>

extern "C" {
#include <dap_chain_net.h>
#include <dap_chain_node_cli.h>
}

#include "utils.h"
#include "config.h"


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


struct CommandContext {
    napi_threadsafe_function func;
    napi_ref js_context_ref;
};

struct CommandData {
    uv_mutex_t mutex;
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

void js_call_finalize(napi_env env, void* finalize_data, void* finalize_hint)
{
    log_it(L_DEBUG, "JS Finalizer is called");
    CommandContext* cmd_context = (CommandContext*)finalize_data;
    if (cmd_context->js_context_ref)
    {
        napi_delete_reference(env, cmd_context->js_context_ref);
    }
    delete cmd_context;
}

/*
    This is a common entry point for every command callback from JS.

    It will call a threadsafe wrapper for a callback and wait until it's finished using a mutex.
*/
int cmd_function(int argc, char **argv, void* context, char **str_reply)
{
    napi_status status;
    CommandContext* cmd_context = (CommandContext*)context;
    CommandData* cmd_data = new CommandData();
    cmd_data->argc = argc;
    cmd_data->argv = argv;
    cmd_data->str_reply = str_reply;

    uv_mutex_init(&cmd_data->mutex);
    uv_mutex_lock(&cmd_data->mutex);

    CHECK(napi_call_threadsafe_function(cmd_context->func, cmd_data, napi_tsfn_nonblocking));

    // CallJS function will unlock it
    uv_mutex_lock(&cmd_data->mutex);
    uv_mutex_unlock(&cmd_data->mutex);
    uv_mutex_destroy(&cmd_data->mutex);

    int result = cmd_data->result;
    delete cmd_data;

    return result;
}

/*
    This function will be called on a main thread of NodeJS
    with a purpose to convert incoming data and result
    and call a callback provided by JS code.
*/
void CallJS(napi_env env, napi_value js_callback, void* context, void* data)
{
    CommandContext* cmd_context = (CommandContext*)context;
    CommandData* cmd_data = (CommandData*)data;

    int argc = cmd_data->argc;
    char** argv = cmd_data->argv;

    int cmd_argc = argc + 1;
    napi_value *cmd_argv = new napi_value[cmd_argc];
    napi_value js_result = nullptr;
    napi_value undefined;
    napi_value js_context = nullptr;
    napi_valuetype arg_type;
    napi_status status;
    int result = 0;

    CHECK(napi_get_undefined(env, &undefined));

    if (cmd_context->js_context_ref)
    {
        CHECK(napi_get_reference_value(env, cmd_context->js_context_ref, &js_context));
    }
    else
    {
        js_context = undefined;
    }

    cmd_argv[0] = js_context;

    for (int i = 0; i < argc; ++i)
    {
        CHECK(napi_create_string_utf8(env, argv[i], NAPI_AUTO_LENGTH, cmd_argv + i + 1));
    }

    status = napi_call_function(env, undefined, js_callback, cmd_argc, cmd_argv, &js_result);
    bool exception_is_pending = false;

    if (status == napi_pending_exception)
    {
        exception_is_pending = true;
    }
    else
    {
        CHECK(status);
        CHECK(napi_is_exception_pending(env, &exception_is_pending));
    }

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

    uv_mutex_unlock(&cmd_data->mutex);
}

/*
    Overall interraction is approximately looks like this:

    Adding a callback:
    JS Code -> js_dap_chain_node_cli_cmd_item_create -> CellFrame Code

    Invoking a callback:
    CellFrame Code -> cmd_function -> Threadsafe Wrapper -> CallJS Function -> JS Code
*/
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

    napi_valuetype context_type;
    CHECK(napi_typeof(env, args[2], &context_type));
    if (context_type != napi_undefined && context_type != napi_null && context_type != napi_object)
    {
        napi_throw_type_error(env, nullptr, "Wrong context type: must be undefined, null or object");
        return nullptr;
    }

    char* cmd_name_buffer = extract_str(env, args[0], nullptr);
    napi_value func = args[1];
    char* cmd_doc_buffer = extract_str(env, args[3], nullptr);
    char* cmd_doc_ex_buffer = extract_str(env, args[4], nullptr);

    CommandContext *cmd_context = new CommandContext();
    napi_value js_context = args[2];

    if (context_type == napi_object)
    {
        CHECK(napi_create_reference(env, js_context, 1, &cmd_context->js_context_ref));
    }

    // TODO: When can I release this reference?!!
    CHECK(napi_create_threadsafe_function(env, func, nullptr, args[0], 0, 1,
                                          cmd_context/*finalizer data*/,
                                          js_call_finalize /*finalizer*/,
                                          cmd_context, CallJS, &cmd_context->func));

    dap_chain_node_cli_cmd_item_create(cmd_name_buffer, cmd_function, cmd_context, cmd_doc_buffer, cmd_doc_ex_buffer);

    delete[] cmd_name_buffer;
    delete[] cmd_doc_buffer;
    delete[] cmd_doc_ex_buffer;

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
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
