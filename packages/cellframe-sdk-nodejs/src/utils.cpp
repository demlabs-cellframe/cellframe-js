#include <assert.h>
#include "utils.h"

#include <dap_common.h>

#define LOG_TAG "utils_nodejs"

static uv_thread_t main_thread;

/*
    WARNING: Type check is omitted!
*/
char* extract_str(napi_env env, napi_value js_value, size_t* size)
{
    size_t str_size;
    // Returns string size without NULL
    napi_status status = napi_get_value_string_utf8(env, js_value, nullptr, 0, &str_size);
    assert(status == napi_ok);

    char* buffer = new char[str_size + 1];
    // Writes string + NULL
    status = napi_get_value_string_utf8(env, js_value, buffer, str_size + 1, &str_size);
    assert(status == napi_ok);

    if (size != nullptr)
    {
        *size = str_size;
    }

    return buffer;
}

/*
    WARNING: Can raise js exception
*/
int extract_array_buffer(napi_env env, napi_value js_value, void** buffer, size_t* size)
{
    napi_status status;
    bool is_array_buffer = false;
    CHECK(napi_is_arraybuffer(env, js_value, &is_array_buffer));
    if (is_array_buffer == false)
    {
        napi_throw_type_error(env, nullptr, "ArrayBuffer expected");
        return -1;
    }

    CHECK(napi_get_arraybuffer_info(env, js_value, buffer, size));

    return 0;
}

napi_status napi_get_value_size(napi_env env, napi_value js_value, size_t* result)
{
#ifdef ENV64
    int64_t _tmp;
    napi_status status = napi_get_value_int64(env, js_value, &_tmp);
    *result = (size_t)_tmp;
    return status;
#else
    return napi_get_value_uint32(env, js_value, result);
#endif
}

napi_status napi_create_size(napi_env env, size_t value, napi_value* result)
{
#ifdef ENV64
    return napi_create_int64(env, (int64_t)value, result);
#else
    return napi_create_uint32(env, value, result);
#endif
}

static void js_call_finalize(napi_env env, void* finalize_data, void* finalize_hint)
{
    log_it(L_DEBUG, "JS Finalizer is called");
    CallbackContext* cmd_context = (CallbackContext*)finalize_data;
    if (cmd_context->js_context_ref)
    {
        napi_delete_reference(env, cmd_context->js_context_ref);
    }
    napi_delete_reference(env, cmd_context->js_func_ref);
    delete cmd_context;
}

struct CallbackData
{
    void* data;
    uv_mutex_t mutex;
    bool locked;
};

/*
    This function will be called on a main thread of NodeJS
    with a purpose to convert incoming data and result
    and call a callback provided by JS code.
*/
static void CallJS(napi_env env, napi_value js_callback, void* context, void* data)
{
    CallbackContext* cmd_context = (CallbackContext*)context;
    CallbackData* cb_data = (CallbackData*)data;

    assert(cmd_context->convert_args != nullptr);

    napi_value js_result = nullptr;
    napi_value undefined;
    napi_value js_context = nullptr;
    napi_status status;

    CHECK(napi_get_undefined(env, &undefined));

    if (cmd_context->js_context_ref)
    {
        CHECK(napi_get_reference_value(env, cmd_context->js_context_ref, &js_context));
    }
    else
    {
        js_context = undefined;
    }

    int cmd_argc = 0;
    napi_value* cmd_argv = nullptr;

    // MAY THROW JS EXCEPTION.
    cmd_context->convert_args(env, js_context, cb_data->data, &cmd_argc, &cmd_argv);

    // MAY THROW JS EXCEPTION.
    status = napi_call_function(env, undefined, js_callback, cmd_argc, cmd_argv, &js_result);

    delete[] cmd_argv;

    if (status != napi_ok && status != napi_pending_exception)
    {
        CHECK(status);
    }

    if (cmd_context->convert_result != nullptr)
    {
        // This function should check for exception that may happen during execution of a callback or arguments converter
        cmd_context->convert_result(env, js_result, cb_data->data);
    }

    if (cb_data->locked)
    {
        uv_mutex_unlock(&cb_data->mutex);
    }
}

/*
    This is a common entry point for every callback to JS interpreter.

    It will call a threadsafe wrapper for a callback and wait until it's finished using a mutex.
*/
void native_callback(void* cmd_data, CallbackContext* cmd_context)
{
    napi_status status;

    bool main_thread = is_main_thread();
    CallbackData cb_data;
    cb_data.data = cmd_data;

    if (main_thread)
    {
        napi_value js_func;
        CHECK(napi_get_reference_value(cmd_context->env, cmd_context->js_func_ref, &js_func));

        cb_data.locked = false;
        CallJS(cmd_context->env, js_func, cmd_context, &cb_data);
    }
    else
    {
        uv_mutex_init(&cb_data.mutex);
        uv_mutex_lock(&cb_data.mutex);

        cb_data.locked = true;
        CHECK(napi_call_threadsafe_function(cmd_context->func, &cb_data, napi_tsfn_nonblocking));

        // Wait until CallJS unlock it
        uv_mutex_lock(&cb_data.mutex);
        uv_mutex_unlock(&cb_data.mutex);
        uv_mutex_destroy(&cb_data.mutex);
    }
}

/*
    Overall interraction is approximately looks like this:
    Native code -> Developer's native callback -> native_callback defined above -> Threadsafe wrapper (optional) -> CallJS defined above -> Userdefined JS callback
*/
int create_callback_context(
    napi_env env, napi_value js_function, napi_value function_name, napi_value js_context,
    callback_arguments_converter_t arguments_converter /*required*/,
    callback_result_converter_t result_converter /*optional*/,
    CallbackContext** result
)
{
    napi_status status;
    assert(arguments_converter != nullptr);

    napi_valuetype context_type;
    CHECK(napi_typeof(env, js_context, &context_type));
    if (context_type != napi_undefined && context_type != napi_null && context_type != napi_object)
    {
        return -10;
    }

    CallbackContext *cmd_context = new CallbackContext();

    if (context_type == napi_object)
    {
        CHECK(napi_create_reference(env, js_context, 1, &cmd_context->js_context_ref));
    }

    CHECK(napi_create_reference(env, js_function, 1, &cmd_context->js_func_ref));
    cmd_context->env = env;

    // TODO: When can I release this reference?!! (SDK does not have deinit function for globalDB callbacks)
    CHECK(napi_create_threadsafe_function(env, js_function, nullptr, function_name, 0, 1,
                                          cmd_context/*finalizer data*/,
                                          js_call_finalize /*finalizer*/,
                                          cmd_context, CallJS, &cmd_context->func));

    // TODO: Since I don't know when to release it, this will allow event loop to exit while reference exists
    CHECK(napi_unref_threadsafe_function(env, cmd_context->func));

    cmd_context->convert_args = arguments_converter;
    cmd_context->convert_result = result_converter;

    *result = cmd_context;

    return 0;
}

void utils_init()
{
    main_thread = uv_thread_self();
}

bool is_main_thread()
{
    uv_thread_t this_thread = uv_thread_self();
    return uv_thread_equal(&main_thread, &this_thread) != 0;
}
