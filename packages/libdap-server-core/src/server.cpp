#include <assert.h>
#include "utils.h"
#include "server.h"


napi_ref Server::constructor;

Server::Server(const char *addr, uint16_t port, dap_server_type_t type)
    : env_(nullptr), wrapper_(nullptr)
{
    server_ = dap_server_listen(addr, port, type);
}

Server::~Server()
{
    if (this->server_ != nullptr)
    {
        // TODO: cleanup somehow???
    }
    napi_delete_reference(env_, wrapper_);
}

void Server::Destructor(napi_env env, void* nativeObject, void*)
{
    reinterpret_cast<Server*>(nativeObject)->~Server();
}

napi_value Server::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("loop", Loop),
    };

    napi_property_descriptor export_descriptors[] = {
        DECLARE_NAPI_METHOD("dap_server_init", js_dap_server_init),
        DECLARE_NAPI_METHOD("dap_server_deinit", js_dap_server_deinit),
    };
    CHECK(napi_define_properties(env, exports,
                                 sizeof(export_descriptors)/sizeof(export_descriptors[0]),
                                 export_descriptors));

    napi_value cons;
    CHECK(napi_define_class(env, "Server", NAPI_AUTO_LENGTH, New, nullptr,
                            sizeof(properties)/sizeof(properties[0]),
                            properties, &cons));
    CHECK(napi_create_reference(env, cons, 1, &constructor));
    CHECK(napi_set_named_property(env, exports, "Server", cons));

    return exports;
}

bool Server::InstanceOf(napi_env env, napi_value object)
{
    napi_status status;
    napi_value cons;
    CHECK(napi_get_reference_value(env, constructor, &cons));

    bool result = false;
    CHECK(napi_instanceof(env, object, cons, &result));

    return result;
}

napi_value Server::New(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    napi_value target;
    CHECK(napi_get_new_target(env, info, &target));
    bool is_constructor = target != nullptr;

    if (is_constructor)
    {
        // Invoked as constructor: `new Server(...)`
        ARG_COUNT_CHECK_UNIQUE(3)

        ARG_TYPE_CHECK(0, napi_string)
        ARG_TYPE_CHECK(1, napi_number)
        ARG_TYPE_CHECK(2, napi_number)

        size_t address_size;
        char* address_str = extract_str(env, args[0], &address_size);

        int32_t port_tmp;
        CHECK(napi_get_value_int32(env, args[1], &port_tmp));
        int16_t port = (int16_t)port_tmp;

        int32_t server_type;
        CHECK(napi_get_value_int32(env, args[2], &server_type));
        // WARNING: keep in sync with dap_server.h
        if (server_type < DAP_SERVER_TCP || server_type > DAP_SERVER_TCP)
        {
            napi_throw_type_error(env, nullptr, "Wrong server type: value out of range");
            return nullptr;
        }

        Server* obj = new Server(address_str, port, (dap_server_type_t)server_type);

        if (obj && obj->server_ != nullptr)
        {
            obj->env_ = env;
            CHECK(napi_wrap(env,
                            jsthis,
                            reinterpret_cast<void*>(obj),
                            Server::Destructor,
                            nullptr,  // finalize_hint
                            &obj->wrapper_));
        }
        else
        {
            delete obj;
            jsthis = nullptr;
        }

        return jsthis;
    }
    else
    {
        // Invoked as plain function `Server(...)`, turn into construct call.
        size_t argc = 3;
        napi_value args[3];
        CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

        napi_value cons;
        CHECK(napi_get_reference_value(env, constructor, &cons));

        napi_value instance;
        CHECK(napi_new_instance(env, cons, argc, args, &instance));

        return instance;
    }
}

napi_value Server::Loop(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    Server* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    int32_t result = dap_server_loop(obj->server_);

    CHECK(napi_create_int32(env, result, &js_result));

    return js_result;
}


napi_value js_dap_server_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_number)

    uint32_t number_of_threads;
    CHECK(napi_get_value_uint32(env, args[0], &number_of_threads));

    int32_t result = dap_server_init(number_of_threads);

    CHECK(napi_create_int32(env, result, &js_result));

    return js_result;
}

napi_value js_dap_server_deinit(napi_env env, napi_callback_info info)
{
    dap_server_deinit();

    return nullptr;
}
