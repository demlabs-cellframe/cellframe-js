#include <assert.h>
#include "utils.h"
#include "events.h"
#include "events_socket.h"


napi_ref EventsSocket::constructor;

EventsSocket::EventsSocket(int sock, struct dap_events *events)
    : env_(nullptr), wrapper_(nullptr)
{
    events_socket_ = dap_events_socket_find(sock, events);
}

EventsSocket::~EventsSocket()
{
    if (this->events_socket_ != nullptr)
    {
        dap_events_socket_delete(this->events_socket_, false);  // TODO: or true?
        this->events_socket_ = nullptr;
    }
    napi_delete_reference(env_, wrapper_);
}

void EventsSocket::Destructor(napi_env env, void* nativeObject, void*)
{
    reinterpret_cast<EventsSocket*>(nativeObject)->~EventsSocket();
}

napi_value EventsSocket::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("readable", IsReadyToRead, SetReadable),
        DECLARE_NAPI_PROPERTY("writable", IsReadyToWrite, SetWritable),
        DECLARE_NAPI_METHOD("read", Read),
        DECLARE_NAPI_METHOD("write", Write),
        DECLARE_NAPI_METHOD("remove", Remove),
        DECLARE_NAPI_METHOD("kill", Kill),
        DECLARE_NAPI_METHOD("shrinkBuffer", ShrinkBufIn),
    };

    napi_property_descriptor export_descriptors[] = {
        DECLARE_NAPI_METHOD("dap_events_socket_init", js_dap_events_socket_init),
        DECLARE_NAPI_METHOD("dap_events_socket_deinit", js_dap_events_socket_deinit),
    };
    CHECK(napi_define_properties(env, exports,
                                 sizeof(export_descriptors)/sizeof(export_descriptors[0]),
                                 export_descriptors));

    napi_value cons;
    CHECK(napi_define_class(env, "EventsSocket", NAPI_AUTO_LENGTH, New, nullptr,
                            sizeof(properties)/sizeof(properties[0]),
                            properties, &cons));
    CHECK(napi_create_reference(env, cons, 1, &constructor));
    CHECK(napi_set_named_property(env, exports, "EventsSocket", cons));

    return exports;
}

napi_value EventsSocket::New(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    napi_value target;
    CHECK(napi_get_new_target(env, info, &target));
    bool is_constructor = target != nullptr;

    if (is_constructor)
    {
        // Invoked as constructor: `new EventsSocket(...)`
        ARG_COUNT_CHECK_UNIQUE(2)

        ARG_TYPE_CHECK(0, napi_number)
        ARG_TYPE_CHECK(1, napi_object)

        int sock;
        CHECK(napi_get_value_int(env, args[0], &sock));

        if (InstanceOf<Events>(env, args[1]) == false)
        {
            napi_throw_type_error(env, nullptr, "Events instance expected");
            return nullptr;
        }
        Events* events;
        CHECK(napi_unwrap(env, args[1], reinterpret_cast<void**>(&events)));

        // TODO: should we keep a reference?
        EventsSocket* obj = new EventsSocket(sock, events->events_);

        if (obj != nullptr)
        {
            obj->env_ = env;
            CHECK(napi_wrap(env, jsthis, reinterpret_cast<void*>(obj),
                            EventsSocket::Destructor, nullptr,&obj->wrapper_));
        }

        return jsthis;
    }
    else
    {
        // Invoked as plain function `EventsSocket(...)`, turn into construct call.
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

napi_value EventsSocket::IsReadyToRead(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    bool result = dap_events_socket_is_ready_to_read(obj->events_socket_);
    CHECK(napi_get_boolean(env, result, &js_result));

    return js_result;
}

napi_value EventsSocket::IsReadyToWrite(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    bool result = dap_events_socket_is_ready_to_write(obj->events_socket_);
    CHECK(napi_get_boolean(env, result, &js_result));

    return js_result;
}

napi_value EventsSocket::SetReadable(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_boolean)

    bool flag;
    CHECK(napi_get_value_bool(env, args[0], &flag));

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_events_socket_set_readable(obj->events_socket_, flag);

    return js_result;
}

napi_value EventsSocket::SetWritable(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_boolean)

    bool flag;
    CHECK(napi_get_value_bool(env, args[0], &flag));

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_events_socket_set_writable(obj->events_socket_, flag);

    return js_result;
}

napi_value EventsSocket::Write(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    void* buffer;
    size_t buffer_size;
    if (extract_array_buffer(env, args[0], &buffer, &buffer_size) < 0)
    {
        return nullptr;
    }

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    size_t result = dap_events_socket_write(obj->events_socket_, buffer, buffer_size);

    CHECK(napi_create_size(env, result, &js_result));

    return js_result;
}

napi_value EventsSocket::Read(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    void* buffer;
    size_t buffer_size;
    if (extract_array_buffer(env, args[0], &buffer, &buffer_size) < 0)
    {
        return nullptr;
    }

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    size_t result = dap_events_socket_read(obj->events_socket_, buffer, buffer_size);

    CHECK(napi_create_size(env, result, &js_result));

    return js_result;
}

napi_value EventsSocket::Remove(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_events_socket_remove(obj->events_socket_);

    return js_result;
}

napi_value EventsSocket::Kill(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_events_socket_kill_socket(obj->events_socket_);

    return js_result;
}

napi_value EventsSocket::ShrinkBufIn(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_number)

    size_t shrink_size;
    CHECK(napi_get_value_size(env, args[0], &shrink_size));

    EventsSocket* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_events_socket_shrink_buf_in(obj->events_socket_, shrink_size);

    return js_result;
}


napi_value js_dap_events_socket_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_events_socket_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_events_socket_deinit(napi_env env, napi_callback_info info)
{
    dap_events_socket_deinit();

    return nullptr;
}
