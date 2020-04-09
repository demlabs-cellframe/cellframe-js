#include <assert.h>
#include "utils.h"
#include "events.h"


napi_ref Events::constructor;

Events::Events()
    : env_(nullptr), wrapper_(nullptr)
{
    events_ = dap_events_new();
}

Events::~Events()
{
    if (this->events_ != nullptr)
    {
        dap_events_delete(this->events_);
        this->events_ = nullptr;
    }
    napi_delete_reference(env_, wrapper_);
}

void Events::Destructor(napi_env env, void* nativeObject, void*)
{
    reinterpret_cast<Events*>(nativeObject)->~Events();
}

napi_value Events::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("start", Start),
        DECLARE_NAPI_METHOD("wait", Wait),
        DECLARE_NAPI_METHOD("threadWakeUp", ThreadWakeUp),
    };

    napi_property_descriptor export_descriptors[] = {
        DECLARE_NAPI_METHOD("dap_events_init", js_dap_events_init),
        DECLARE_NAPI_METHOD("dap_events_deinit", js_dap_events_deinit),
    };
    CHECK(napi_define_properties(env, exports,
                                 sizeof(export_descriptors)/sizeof(export_descriptors[0]),
                                 export_descriptors));

    napi_value cons;
    CHECK(napi_define_class(env, "Events", NAPI_AUTO_LENGTH, New, nullptr,
                            sizeof(properties)/sizeof(properties[0]),
                            properties, &cons));
    CHECK(napi_create_reference(env, cons, 1, &constructor));
    CHECK(napi_set_named_property(env, exports, "Events", cons));

    return exports;
}

bool Events::InstanceOf(napi_env env, napi_value object)
{
    napi_status status;
    napi_value cons;
    CHECK(napi_get_reference_value(env, constructor, &cons));

    bool result = false;
    CHECK(napi_instanceof(env, object, cons, &result));

    return result;
}

napi_value Events::New(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    napi_value target;
    CHECK(napi_get_new_target(env, info, &target));
    bool is_constructor = target != nullptr;

    if (is_constructor)
    {
        // Invoked as constructor: `new Events(...)`
        ARG_COUNT_CHECK_UNIQUE(0)

        Events* obj = new Events();

        if (obj != nullptr)
        {
            obj->env_ = env;
            CHECK(napi_wrap(env,
                            jsthis,
                            reinterpret_cast<void*>(obj),
                            Events::Destructor,
                            nullptr,  // finalize_hint
                            &obj->wrapper_));
        }

        return jsthis;
    }
    else
    {
        // Invoked as plain function `Events(...)`, turn into construct call.
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

napi_value Events::Start(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    Events* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    int32_t result = dap_events_start(obj->events_);

    CHECK(napi_create_int32(env, result, &js_result));

    return js_result;
}

napi_value Events::Wait(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    Events* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    int32_t result = dap_events_wait(obj->events_);

    CHECK(napi_create_int32(env, result, &js_result));

    return js_result;
}

napi_value Events::ThreadWakeUp(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    Events* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_events_thread_wake_up(&obj->events_->proc_thread);

    return js_result;
}


napi_value js_dap_events_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(2)

    ARG_TYPE_CHECK(0, napi_number)
    ARG_TYPE_CHECK(1, napi_number)

    uint32_t number_of_threads;
    CHECK(napi_get_value_uint32(env, args[0], &number_of_threads));

    size_t connection_timeout;
    CHECK(napi_get_value_size(env, args[1], &connection_timeout));

    int32_t result = dap_events_init(number_of_threads, connection_timeout);

    CHECK(napi_create_int32(env, result, &js_result));

    return js_result;
}

napi_value js_dap_events_deinit(napi_env env, napi_callback_info info)
{
    dap_events_deinit();

    return nullptr;
}
