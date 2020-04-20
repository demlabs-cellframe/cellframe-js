#include <assert.h>
#include <node_api.h>

extern "C" {
#include <dap_common.h>
#include <dap_app_cli.h>
}

#include "utils.h"
#include "app_cli_connection.h"

#define LOG_TAG "main_app_cli_nodejs"


/*
    Application CLI
*/


napi_value js_dap_app_cli_main(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(3)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)
    ARG_TYPE_CHECK(2, napi_object)

    bool is_array = false;
    CHECK(napi_is_array(env, args[2], &is_array));
    if (is_array == false)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument type: array expected");
        return nullptr;
    }

    size_t app_name_size;
    char* app_name_buffer;
    app_name_buffer = extract_str(env, args[0], &app_name_size);

    size_t socket_path_size;
    char* socket_path_buffer;
    socket_path_buffer = extract_str(env, args[1], &socket_path_size);

    uint32_t app_cli_argv_size = 0;
    CHECK(napi_get_array_length(env, args[2], &app_cli_argv_size));
    char** app_cli_argv = new char*[app_cli_argv_size]();
    napi_value tmp_element;
    bool was_error = false;

    for (uint32_t i = 0; i < app_cli_argv_size; ++i)
    {
        CHECK(napi_get_element(env, args[2], i, &tmp_element));
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
        log_it(L_DEBUG, "argv size before: %u", app_cli_argv_size);
        int app_cli_argc = (int)app_cli_argv_size;
        log_it(L_DEBUG, "argv size after:  %i", app_cli_argc);
        int result = dap_app_cli_main(app_name_buffer, socket_path_buffer, app_cli_argc, app_cli_argv);
        CHECK(napi_create_int(env, result, &js_result));
    }

    delete[] app_name_buffer;
    delete[] socket_path_buffer;
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


/*
*/


napi_value ApplicationCLIInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_app_cli_main", js_dap_app_cli_main),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    exports = AppCLIConnection::Init(env, exports);

    return exports;
}
