#include <assert.h>

#include "utils.h"

#include "chain_net.h"


napi_ref ChainNet::constructor;

ChainNet::ChainNet(char* net_name)
    : env_(nullptr), wrapper_(nullptr)
{
    chain_net_ = dap_chain_net_by_name(net_name);
}

ChainNet::~ChainNet()
{
    if (this->chain_net_ != nullptr)
    {
        // We don't own it
        //dap_chain_net_delete(this->chain_net_);
        this->chain_net_ = nullptr;
    }
    napi_delete_reference(env_, wrapper_);
}

void ChainNet::Destructor(napi_env env, void* nativeObject, void*)
{
    delete reinterpret_cast<ChainNet*>(nativeObject);
}

napi_value ChainNet::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("procMempool", ProcMempool),
        DECLARE_NAPI_METHOD("goSync", GoSync),
        DECLARE_NAPI_METHOD("syncGDB", SyncGDB),
    };

    napi_value cons;
    CHECK(napi_define_class(env, "ChainNet", NAPI_AUTO_LENGTH, New, nullptr,
                            sizeof(properties)/sizeof(properties[0]),
                            properties, &cons));
    CHECK(napi_create_reference(env, cons, 1, &constructor));
    CHECK(napi_set_named_property(env, exports, "ChainNet", cons));

    return exports;
}

napi_value ChainNet::New(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    napi_value target;
    CHECK(napi_get_new_target(env, info, &target));
    bool is_constructor = target != nullptr;

    if (is_constructor)
    {
        // Invoked as constructor: `new ChainNet(...)`
        ARG_COUNT_CHECK_UNIQUE(1)

        ARG_TYPE_CHECK(0, napi_string)

        char* net_name = extract_str(env, args[0], nullptr);

        ChainNet* obj = new ChainNet(net_name);

        delete[] net_name;

        if (obj != nullptr)
        {
            obj->env_ = env;
            CHECK(napi_wrap(env, jsthis, reinterpret_cast<void*>(obj),
                            ChainNet::Destructor, nullptr,&obj->wrapper_));
        }

        return jsthis;
    }
    else
    {
        // Invoked as plain function `ChainNet(...)`, turn into construct call.
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

napi_value ChainNet::ProcMempool(napi_env env, napi_callback_info info)
{
    napi_status status;

    ARG_COUNT_CHECK_UNIQUE(0)

    ChainNet* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_chain_net_proc_mempool(obj->chain_net_);

    return nullptr;
}

napi_value ChainNet::GoSync(napi_env env, napi_callback_info info)
{
    napi_status status;

    ARG_COUNT_CHECK_UNIQUE(0)

    ChainNet* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    s_net_set_go_sync(obj->chain_net_);

    return nullptr;
}

napi_value ChainNet::SyncGDB(napi_env env, napi_callback_info info)
{
    napi_status status;

    ARG_COUNT_CHECK_UNIQUE(0)

    ChainNet* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_chain_net_sync_gdb(obj->chain_net_);

    return nullptr;
}
