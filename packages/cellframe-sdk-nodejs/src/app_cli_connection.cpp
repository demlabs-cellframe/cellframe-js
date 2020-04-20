#include <assert.h>

#include <dap_app_cli.h>

#include "utils.h"
#include "app_cli_connection.h"

extern "C" {
int shell_reader_loop(dap_app_cli_connect_param_t *cparam);
}

napi_ref AppCLIConnection::constructor;

AppCLIConnection::AppCLIConnection(char* socket_path)
    : env_(nullptr), wrapper_(nullptr)
{
    connection_ = dap_app_cli_connect(socket_path);
}

AppCLIConnection::~AppCLIConnection()
{
    if (this->connection_ != nullptr)
    {
        dap_app_cli_disconnect(this->connection_);
        this->connection_ = nullptr;
    }
    napi_delete_reference(env_, wrapper_);
}

void AppCLIConnection::Destructor(napi_env env, void* nativeObject, void*)
{
    reinterpret_cast<AppCLIConnection*>(nativeObject)->~AppCLIConnection();
}

napi_value AppCLIConnection::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("postCommand", PostCommand),
        DECLARE_NAPI_METHOD("disconnect", Disconnect),
        DECLARE_NAPI_METHOD("shellReaderLoop", ShellReaderLoop),
        DECLARE_NAPI_METHOD("isOK", IsOK),
    };

    napi_value cons;
    CHECK(napi_define_class(env, "AppCLIConnection", NAPI_AUTO_LENGTH, New, nullptr,
                            sizeof(properties)/sizeof(properties[0]),
                            properties, &cons));
    CHECK(napi_create_reference(env, cons, 1, &constructor));
    CHECK(napi_set_named_property(env, exports, "AppCLIConnection", cons));

    return exports;
}

napi_value AppCLIConnection::New(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    napi_value target;
    CHECK(napi_get_new_target(env, info, &target));
    bool is_constructor = target != nullptr;

    if (is_constructor)
    {
        // Invoked as constructor: `new AppCLIConnection(...)`
        ARG_COUNT_CHECK_UNIQUE(1)

        ARG_TYPE_CHECK(0, napi_string)

        char* socket_path_buffer = extract_str(env, args[0], nullptr);

        AppCLIConnection* obj = new AppCLIConnection(socket_path_buffer);

        delete[] socket_path_buffer;

        if (obj != nullptr)
        {
            obj->env_ = env;
            CHECK(napi_wrap(env, jsthis, reinterpret_cast<void*>(obj),
                            AppCLIConnection::Destructor, nullptr,&obj->wrapper_));
        }

        return jsthis;
    }
    else
    {
        // Invoked as plain function `AppCLIConnection(...)`, turn into construct call.
        size_t argc = 1;
        napi_value args[1];
        CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

        napi_value cons;
        CHECK(napi_get_reference_value(env, constructor, &cons));

        napi_value instance;
        CHECK(napi_new_instance(env, cons, argc, args, &instance));

        return instance;
    }
}

napi_value AppCLIConnection::PostCommand(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    bool is_array = false;
    CHECK(napi_is_array(env, args[0], &is_array));
    if (is_array == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong 1st argument type: array expected");
        return nullptr;
    }

    uint32_t app_cli_argv_size = 0;
    CHECK(napi_get_array_length(env, args[0], &app_cli_argv_size));

    if (app_cli_argv_size == 0)
    {
        napi_throw_error(env, nullptr, "Wrong 1st argument: array must not be empty");
        return nullptr;
    }

    char** app_cli_argv = new char*[app_cli_argv_size]();
    napi_value tmp_element;
    bool was_error = false;

    for (uint32_t i = 0; i < app_cli_argv_size; ++i)
    {
        CHECK(napi_get_element(env, args[0], i, &tmp_element));
        CHECK(napi_typeof(env, tmp_element, &arg_type));
        if (arg_type != napi_string)
        {
            napi_throw_type_error(env, nullptr, "Wrong 3rd argument: array of strings expected");
            was_error = true;
            break;
        }

        app_cli_argv[i] = extract_str(env, tmp_element, nullptr);
    }


    if (was_error == false)
    {
        int app_cli_argc = (int)app_cli_argv_size;
        dap_app_cli_cmd_state_t cmd;
        memset(&cmd, 0, sizeof(dap_app_cli_cmd_state_t));
        cmd.cmd_name = app_cli_argv[0];
        cmd.cmd_param_count = app_cli_argc - 1;
        if (cmd.cmd_param_count > 0)
        {
            cmd.cmd_param = (char**) (app_cli_argv + 2);
        }

        AppCLIConnection* obj;
        CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

        int result = dap_app_cli_post_command(obj->connection_, &cmd);
        CHECK(napi_create_int(env, result, &js_result));
    }


    // Cleanup
    if (app_cli_argv != nullptr)
    {
        for (uint32_t i = 0; i < app_cli_argv_size; ++i)
        {
            if (app_cli_argv[i])
            {
                delete[] app_cli_argv[i];
            }
        }

        delete[] app_cli_argv;
    }


    return js_result;
}

napi_value AppCLIConnection::Disconnect(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    AppCLIConnection* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));
    int result = dap_app_cli_disconnect(obj->connection_);
    obj->connection_ = nullptr;

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value AppCLIConnection::ShellReaderLoop(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    AppCLIConnection* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));
    int result = shell_reader_loop(obj->connection_);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value AppCLIConnection::IsOK(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    AppCLIConnection* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    CHECK(napi_get_boolean(env, obj->connection_ != nullptr, &js_result));

    return js_result;
}
