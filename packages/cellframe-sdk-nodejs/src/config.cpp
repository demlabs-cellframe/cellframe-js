#include <assert.h>
#include <node_api.h>
#include "config.h"
#include "utils.h"

napi_ref Config::constructor;

Config::Config(const char* app_name)
    : env_(nullptr), wrapper_(nullptr)
{
    config_ = dap_config_open(app_name);
}

Config::~Config()
{
    if (this->config_ != nullptr)
    {
        dap_config_close(this->config_);
        this->config_ = nullptr;
    }
    napi_delete_reference(env_, wrapper_);
}

void Config::Destructor(napi_env env,
                          void* nativeObject,
                          void* /*finalize_hint*/)
{
    reinterpret_cast<Config*>(nativeObject)->~Config();
}

napi_value Config::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        //{"value", 0, 0, GetValue, SetValue, 0, napi_default, 0},
        //DECLARE_NAPI_METHOD("open", Open),
        DECLARE_NAPI_METHOD("close", Close),
        DECLARE_NAPI_METHOD("getString", GetString),
        DECLARE_NAPI_METHOD("getStringArray", GetStringArray),
        DECLARE_NAPI_METHOD("getInt32", GetInt32),
        DECLARE_NAPI_METHOD("getInt64", GetInt64),
        DECLARE_NAPI_METHOD("getUint64", GetUint64),
        DECLARE_NAPI_METHOD("getUint16", GetUint16),
        DECLARE_NAPI_METHOD("getDouble", GetDouble),
        DECLARE_NAPI_METHOD("getBool", GetBool),
        DECLARE_NAPI_METHOD("makeItGlobal", MakeItGlobal),
    };

    napi_value cons;
    status = napi_define_class(env, "Config", NAPI_AUTO_LENGTH, New, nullptr,
                               sizeof(properties)/sizeof(properties[0]),
                               properties, &cons);
    assert(status == napi_ok);

    status = napi_create_reference(env, cons, 1, &constructor);
    assert(status == napi_ok);

    status = napi_set_named_property(env, exports, "Config", cons);
    assert(status == napi_ok);
    return exports;
}

napi_value Config::New(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value target;
    status = napi_get_new_target(env, info, &target);
    assert(status == napi_ok);
    bool is_constructor = target != nullptr;

    if (is_constructor)
    {
        // Invoked as constructor: `new Config(...)`
        size_t argc = 1;
        napi_value args[1];
        napi_value jsthis;
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        napi_valuetype valuetype;
        status = napi_typeof(env, args[0], &valuetype);
        assert(status == napi_ok);

        char* buffer = nullptr;
        if (valuetype == napi_string)
        {
            buffer = extract_str(env, args[0]);
        }

        Config* obj = new Config(buffer);

        delete[] buffer;

        obj->env_ = env;
        status = napi_wrap(env,
                           jsthis,
                           reinterpret_cast<void*>(obj),
                           Config::Destructor,
                           nullptr,  // finalize_hint
                           &obj->wrapper_);
        assert(status == napi_ok);

        return jsthis;
    }
    else
    {
        // Invoked as plain function `Config(...)`, turn into construct call.
        size_t argc_ = 1;
        napi_value args[1];
        status = napi_get_cb_info(env, info, &argc_, args, nullptr, nullptr);
        assert(status == napi_ok);

        const size_t argc = 1;
        napi_value argv[argc] = {args[0]};

        napi_value cons;
        status = napi_get_reference_value(env, constructor, &cons);
        assert(status == napi_ok);

        napi_value instance;
        status = napi_new_instance(env, cons, argc, argv, &instance);
        assert(status == napi_ok);

        return instance;
    }
}

napi_value Config::Close(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ != nullptr)
    {
        dap_config_close(obj->config_);
        obj->config_ = nullptr;
    }

    return nullptr;
}

napi_value Config::GetString(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value jsthis;
    size_t argc = 3;
    napi_value args[3];

    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);
    if (argc < 2 || argc > 3)
    {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    napi_valuetype optional_arg_type;
    status = napi_typeof(env, args[2], &optional_arg_type);
    assert(status == napi_ok);

    if (optional_arg_type != napi_string && optional_arg_type != napi_undefined)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: string expected");
        return nullptr;
    }

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);
    char* default_value_buffer = nullptr;

    const char* result = nullptr;

    if (optional_arg_type == napi_string)
    {
        default_value_buffer = extract_str(env, args[2]);
        // Returns same pointer to default value if it's used
        result = dap_config_get_item_str_default(obj->config_, section_buffer, item_name_buffer, default_value_buffer);
    }
    else if (optional_arg_type == napi_undefined)
    {
        result = dap_config_get_item_str(obj->config_, section_buffer, item_name_buffer);
    }
    else
    {
        assert(false);
    }

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result = nullptr;

    if (result)
    {
        status = napi_create_string_utf8(env, result, NAPI_AUTO_LENGTH, &js_result);
        assert(status == napi_ok);
    }

    delete[] default_value_buffer;

    return js_result;
}

napi_value Config::GetStringArray(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    ARG_COUNT_CHECK_UNIQUE(2)

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);

    uint16_t result_size;
    char** result = nullptr;

    result = dap_config_get_array_str(obj->config_, section_buffer, item_name_buffer, &result_size);

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result = nullptr;

    if (result)
    {
        CHECK(napi_create_array_with_length(env, result_size, &js_result));
        napi_value tmp_str;

        for (uint16_t i = 0; i < result_size; ++i)
        {
            CHECK(napi_create_string_utf8(env, result[i], NAPI_AUTO_LENGTH, &tmp_str));
            CHECK(napi_set_element(env, js_result, i, tmp_str));
        }
    }

    return js_result;
}

napi_value Config::GetInt32(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value jsthis;
    size_t argc = 3;
    napi_value args[3];

    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);
    if (argc < 2 || argc > 3)
    {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    napi_valuetype optional_arg_type;
    status = napi_typeof(env, args[2], &optional_arg_type);
    assert(status == napi_ok);

    if (optional_arg_type != napi_number && optional_arg_type != napi_undefined)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: number expected");
        return nullptr;
    }

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);

    int32_t result;

    if (optional_arg_type == napi_number)
    {
        int32_t default_value;
        status = napi_get_value_int32(env, args[2], &default_value);
        assert(status == napi_ok);

        result = dap_config_get_item_int32_default(obj->config_, section_buffer, item_name_buffer, default_value);
    }
    else if (optional_arg_type == napi_undefined)
    {
        result = dap_config_get_item_int32(obj->config_, section_buffer, item_name_buffer);
    }
    else
    {
        assert(false);
    }

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result;
    status = napi_create_int32(env, result, &js_result);
    assert(status == napi_ok);

    return js_result;
}

napi_value Config::GetInt64(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value jsthis;
    size_t argc = 3;
    napi_value args[3];

    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);
    if (argc < 2 || argc > 3)
    {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    napi_valuetype optional_arg_type;
    status = napi_typeof(env, args[2], &optional_arg_type);
    assert(status == napi_ok);

    if (optional_arg_type != napi_number && optional_arg_type != napi_undefined)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: number expected");
        return nullptr;
    }

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);

    int64_t result;

    if (optional_arg_type == napi_number)
    {
        int64_t default_value;
        status = napi_get_value_int64(env, args[2], &default_value);
        assert(status == napi_ok);

        result = dap_config_get_item_int64_default(obj->config_, section_buffer, item_name_buffer, default_value);
    }
    else if (optional_arg_type == napi_undefined)
    {
        result = dap_config_get_item_int64(obj->config_, section_buffer, item_name_buffer);
    }
    else
    {
        assert(false);
    }

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result;
    status = napi_create_int64(env, result, &js_result);
    assert(status == napi_ok);

    return js_result;
}

napi_value Config::GetUint64(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value jsthis;
    size_t argc = 3;
    napi_value args[3];

    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);
    if (argc < 2 || argc > 3)
    {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    napi_valuetype optional_arg_type;
    status = napi_typeof(env, args[2], &optional_arg_type);
    assert(status == napi_ok);

    if (optional_arg_type != napi_number && optional_arg_type != napi_undefined)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: number expected");
        return nullptr;
    }

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);

    uint64_t result;

    if (optional_arg_type == napi_number)
    {
        int64_t default_value;
        status = napi_get_value_int64(env, args[2], &default_value);
        assert(status == napi_ok);

        result = (uint64_t)dap_config_get_item_uint64_default(obj->config_, section_buffer, item_name_buffer, (uint64_t)default_value);
    }
    else if (optional_arg_type == napi_undefined)
    {
        result = (uint64_t)dap_config_get_item_uint64(obj->config_, section_buffer, item_name_buffer);
    }
    else
    {
        assert(false);
    }

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result;
    status = napi_create_int64(env, result, &js_result);
    assert(status == napi_ok);

    return js_result;
}

napi_value Config::GetUint16(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value jsthis;
    size_t argc = 3;
    napi_value args[3];

    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);
    if (argc < 2 || argc > 3)
    {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    napi_valuetype optional_arg_type;
    status = napi_typeof(env, args[2], &optional_arg_type);
    assert(status == napi_ok);

    if (optional_arg_type != napi_number && optional_arg_type != napi_undefined)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: number expected");
        return nullptr;
    }

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);

    uint16_t result;

    if (optional_arg_type == napi_number)
    {
        uint32_t default_value;
        status = napi_get_value_uint32(env, args[2], &default_value);
        assert(status == napi_ok);

        result = dap_config_get_item_uint16_default(obj->config_, section_buffer, item_name_buffer, (uint16_t)default_value);
    }
    else if (optional_arg_type == napi_undefined)
    {
        result = dap_config_get_item_uint16(obj->config_, section_buffer, item_name_buffer);
    }
    else
    {
        assert(false);
    }

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result;
    status = napi_create_uint32(env, (uint32_t)result, &js_result);
    assert(status == napi_ok);

    return js_result;
}

napi_value Config::GetDouble(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value jsthis;
    size_t argc = 3;
    napi_value args[3];

    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);
    if (argc < 2 || argc > 3)
    {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    napi_valuetype optional_arg_type;
    status = napi_typeof(env, args[2], &optional_arg_type);
    assert(status == napi_ok);

    if (optional_arg_type != napi_number && optional_arg_type != napi_undefined)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: number expected");
        return nullptr;
    }

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);

    double result;

    if (optional_arg_type == napi_number)
    {
        double default_value;
        status = napi_get_value_double(env, args[2], &default_value);
        assert(status == napi_ok);

        result = dap_config_get_item_double_default(obj->config_, section_buffer, item_name_buffer, default_value);
    }
    else if (optional_arg_type == napi_undefined)
    {
        result = dap_config_get_item_double(obj->config_, section_buffer, item_name_buffer);
    }
    else
    {
        assert(false);
    }

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result;
    status = napi_create_double(env, result, &js_result);
    assert(status == napi_ok);

    return js_result;
}

napi_value Config::GetBool(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value jsthis;
    size_t argc = 3;
    napi_value args[3];

    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);
    if (argc < 2 || argc > 3)
    {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    Config* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    if (obj->config_ == nullptr)
    {
        napi_throw_type_error(env, nullptr, "Config is closed");
        return nullptr;
    }

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)

    napi_valuetype optional_arg_type;
    status = napi_typeof(env, args[2], &optional_arg_type);
    assert(status == napi_ok);

    if (optional_arg_type != napi_boolean && optional_arg_type != napi_undefined)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: number expected");
        return nullptr;
    }

    char* section_buffer = extract_str(env, args[0]);
    char* item_name_buffer = extract_str(env, args[1]);

    bool result;

    if (optional_arg_type == napi_boolean)
    {
        bool default_value;
        status = napi_get_value_bool(env, args[2], &default_value);
        assert(status == napi_ok);

        result = dap_config_get_item_bool_default(obj->config_, section_buffer, item_name_buffer, default_value);
    }
    else if (optional_arg_type == napi_undefined)
    {
        result = dap_config_get_item_int64(obj->config_, section_buffer, item_name_buffer);
    }
    else
    {
        assert(false);
    }

    delete[] section_buffer;
    delete[] item_name_buffer;

    napi_value js_result;
    status = napi_get_boolean(env, result, &js_result);
    assert(status == napi_ok);

    return js_result;
}

napi_value Config::MakeItGlobal(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsthis;
    CHECK(napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

    Config* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    g_config = obj->config_;

    return nullptr;
}
