#include <assert.h>
#include "utils.h"
#include "key.h"


// WARNING: keep in sync with dap_enc_key.h
bool is_dap_key_type_valid(int32_t key_type)
{
    if (key_type < DAP_ENC_KEY_TYPE_IAES || key_type > DAP_ENC_KEY_TYPE_SIG_DILITHIUM)
    {
        return false;
    }

    return true;
}


napi_ref Key::constructor;

Key::Key(dap_enc_key_type_t a_key_type)
    : env_(nullptr), wrapper_(nullptr)
{
    key_ = dap_enc_key_new(a_key_type);
}

Key::Key(dap_enc_key_type_t a_key_type, const void *kex_buf,
         size_t kex_size, const void* seed,
         size_t seed_size, size_t key_size)
    : env_(nullptr), wrapper_(nullptr)
{
    key_ = dap_enc_key_new_generate(a_key_type, kex_buf,
                                    kex_size, seed, seed_size,
                                    key_size);
}

Key::~Key()
{
    if (this->key_ != nullptr)
    {
        dap_enc_key_delete(this->key_);
        this->key_ = nullptr;
    }
    napi_delete_reference(env_, wrapper_);
}

void Key::Destructor(napi_env env, void* nativeObject, void*)
{
    reinterpret_cast<Key*>(nativeObject)->~Key();
}

napi_value Key::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("getEncSize", GetEncSize),
        DECLARE_NAPI_METHOD("getDecSize", GetDecSize),
        DECLARE_NAPI_METHOD("serializePrivateKey", SerializePrivateKey),
        DECLARE_NAPI_METHOD("serializePublicKey", SerializePublicKey),
        DECLARE_NAPI_METHOD("deserializePrivateKey", DeserializePrivateKey),
        DECLARE_NAPI_METHOD("deserializePublicKey", DeserializePublicKey),
        DECLARE_NAPI_METHOD("update", Update),
        DECLARE_NAPI_METHOD("genKeyPublicSize", GenKeyPublicSize),
        DECLARE_NAPI_METHOD("genKeyPublic", GenKeyPublic),
    };

    napi_value key_type_object;
    napi_value key_types[18];  // dap_enc_key.h
    CHECK(napi_create_object(env, &key_type_object));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_IAES,          key_types + 0));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_OAES,          key_types + 1));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_RLWE_NEWHOPE,  key_types + 2));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_SIDH_CLN16,    key_types + 3));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_DEFEO,         key_types + 4));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_MSRLN,         key_types + 5));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_RLWE_MSRLN16,  key_types + 6));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_RLWE_BCNS15,   key_types + 7));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_LWE_FRODO,     key_types + 8));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_SIDH_IQC_REF,  key_types + 9));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_CODE_MCBITS,   key_types + 10));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_NTRU,          key_types + 11));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_MLWE_KYBER,    key_types + 12));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_SIG_PICNIC,    key_types + 13));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_SIG_BLISS,     key_types + 14));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_SIG_TESLA,     key_types + 15));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_SIG_DILITHIUM, key_types + 16));
    CHECK(napi_create_int32(env, dap_enc_key_type_t::DAP_ENC_KEY_TYPE_NULL,          key_types + 17));
    napi_property_descriptor key_type_descriptors[] = {
        DECLARE_NAPI_JS_VALUE("iaes",          key_types[0]),
        DECLARE_NAPI_JS_VALUE("oaes",          key_types[1]),
        DECLARE_NAPI_JS_VALUE("rlwe_newhope",  key_types[2]),
        DECLARE_NAPI_JS_VALUE("sidh_cln16",    key_types[3]),
        DECLARE_NAPI_JS_VALUE("defeo",         key_types[4]),
        DECLARE_NAPI_JS_VALUE("msrln",         key_types[5]),
        DECLARE_NAPI_JS_VALUE("rlwe_msrln16",  key_types[6]),
        DECLARE_NAPI_JS_VALUE("rlwe_bcns15",   key_types[7]),
        DECLARE_NAPI_JS_VALUE("lwe_frodo",     key_types[8]),
        DECLARE_NAPI_JS_VALUE("sidh_iqc_ref",  key_types[9]),
        DECLARE_NAPI_JS_VALUE("code_mcbits",   key_types[10]),
        DECLARE_NAPI_JS_VALUE("ntru",          key_types[11]),
        DECLARE_NAPI_JS_VALUE("mlwe_kyber",    key_types[12]),
        DECLARE_NAPI_JS_VALUE("sig_picnic",    key_types[13]),
        DECLARE_NAPI_JS_VALUE("sig_bliss",     key_types[14]),
        DECLARE_NAPI_JS_VALUE("sig_tesla",     key_types[15]),
        DECLARE_NAPI_JS_VALUE("sig_dilithium", key_types[16]),
        DECLARE_NAPI_JS_VALUE("null",          key_types[17]),
    };
    CHECK(napi_define_properties(env, key_type_object,
                                 sizeof(key_type_descriptors)/sizeof(key_type_descriptors[0]),
                                 key_type_descriptors));
    napi_property_descriptor export_descriptors[] = {
        DECLARE_NAPI_JS_VALUE("KeyTypes", key_type_object),
        DECLARE_NAPI_METHOD("dap_enc_key_init", js_dap_enc_key_init),
        DECLARE_NAPI_METHOD("dap_enc_key_deinit", js_dap_enc_key_deinit),
    };
    CHECK(napi_define_properties(env, exports,
                                 sizeof(export_descriptors)/sizeof(export_descriptors[0]),
                                 export_descriptors));

    napi_value cons;
    CHECK(napi_define_class(env, "Key", NAPI_AUTO_LENGTH, New, nullptr,
                            sizeof(properties)/sizeof(properties[0]),
                            properties, &cons));
    CHECK(napi_create_reference(env, cons, 1, &constructor));
    CHECK(napi_set_named_property(env, exports, "Key", cons));

    return exports;
}

napi_value Key::New(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;

    napi_value target;
    CHECK(napi_get_new_target(env, info, &target));
    bool is_constructor = target != nullptr;

    if (is_constructor)
    {
        // Invoked as constructor: `new Key(...)`
        size_t argc = 4;
        napi_value args[4];
        napi_value jsthis = nullptr;
        CHECK(napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr));

        napi_valuetype valuetype;
        CHECK(napi_typeof(env, args[0], &valuetype));

        int32_t key_type;
        if (valuetype == napi_number)
        {
            CHECK(napi_get_value_int32(env, args[0], &key_type));

            if (is_dap_key_type_valid(key_type) == false)
            {
                napi_throw_type_error(env, nullptr, "Wrong key type: value out of range");
                return nullptr;
            }
        }
        else
        {
            napi_throw_type_error(env, nullptr, "Wrong argument type: number expected");
            return nullptr;
        }

        Key* obj = nullptr;
        if (argc == 1)
        {
            obj = new Key((dap_enc_key_type_t)key_type);
        }
        else if (argc == 4)
        {
            ARG_TYPE_CHECK(1, napi_object)
            ARG_TYPE_CHECK(2, napi_object)
            ARG_TYPE_CHECK(3, napi_number)

            // Argument #1: kex arraybuffer
            size_t kex_buffer_size;
            void* kex_buffer = nullptr;
            if (extract_array_buffer(env, args[1], &kex_buffer, &kex_buffer_size) < 0)
            {
                return nullptr;
            }

            // Argument #2: seed arraybuffer
            size_t seed_buffer_size;
            void* seed_buffer = nullptr;
            if (extract_array_buffer(env, args[2], &seed_buffer, &seed_buffer_size) < 0)
            {
                return nullptr;
            }

            // Argument #3: key size
            size_t key_size;
            CHECK(napi_get_value_size(env, args[3], &key_size))

            // Result
            obj = new Key((dap_enc_key_type_t)key_type, kex_buffer, kex_buffer_size, seed_buffer, seed_buffer_size, key_size);
        }

        if (obj != nullptr)
        {
            obj->env_ = env;
            CHECK(napi_wrap(env,
                            jsthis,
                            reinterpret_cast<void*>(obj),
                            Key::Destructor,
                            nullptr,  // finalize_hint
                            &obj->wrapper_));
        }

        return jsthis;
    }
    else
    {
        // Invoked as plain function `Key(...)`, turn into construct call.
        size_t argc = 4;
        napi_value args[4];
        CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

        napi_value cons;
        CHECK(napi_get_reference_value(env, constructor, &cons));

        napi_value instance;
        CHECK(napi_new_instance(env, cons, argc, args, &instance));

        return instance;
    }
}

napi_value Key::GetEncSize(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_number)

    size_t size;
    CHECK(napi_get_value_size(env, args[0], &size));

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    size_t result = dap_enc_key_get_enc_size(obj->key_, size);

    CHECK(napi_create_size(env, result, &js_result));

    return js_result;
}

napi_value Key::GetDecSize(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_number)

    size_t size;
    CHECK(napi_get_value_size(env, args[0], &size));

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    size_t result = dap_enc_key_get_dec_size(obj->key_, size);

    CHECK(napi_create_size(env, result, &js_result));

    return js_result;
}

napi_value Key::SerializePrivateKey(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;
    napi_value jsthis = nullptr;

    CHECK(napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    size_t size;
    uint8_t* buffer = dap_enc_key_serealize_priv_key(obj->key_, &size);
    CHECK(napi_create_arraybuffer(env, size, reinterpret_cast<void**>(&buffer), &js_result));

    return js_result;
}

napi_value Key::SerializePublicKey(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;
    napi_value jsthis = nullptr;

    CHECK(napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    size_t size;
    uint8_t* buffer = dap_enc_key_serealize_pub_key(obj->key_, &size);
    CHECK(napi_create_arraybuffer(env, size, reinterpret_cast<void**>(&buffer), &js_result));

    return js_result;
}

napi_value Key::DeserializePrivateKey(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    // Argument #0
    size_t buffer_size;
    void* buffer = nullptr;
    if (extract_array_buffer(env, args[0], &buffer, &buffer_size) < 0)
    {
        return nullptr;
    }

    // Result
    int result = dap_enc_key_deserealize_priv_key(obj->key_, (uint8_t*)buffer, buffer_size);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value Key::DeserializePublicKey(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    // Argument #0
    size_t buffer_size;
    void* buffer = nullptr;
    if (extract_array_buffer(env, args[0], &buffer, &buffer_size) < 0)
    {
        return nullptr;
    }

    // Result
    int result = dap_enc_key_deserealize_pub_key(obj->key_, (uint8_t*)buffer, buffer_size);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value Key::Update(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsthis = nullptr;
    size_t argc = 0;

    CHECK(napi_get_cb_info(env, info, &argc, nullptr, &jsthis, nullptr));

    if (argc != 0)
    {
        napi_throw_error(env, nullptr, "No arguments are expected");
        return nullptr;
    }

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    dap_enc_key_update(obj->key_);

    return nullptr;
}

napi_value Key::GenKeyPublicSize(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsthis;
    napi_value js_result;

    CHECK(napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    size_t result = dap_enc_gen_key_public_size(obj->key_);

    CHECK(napi_create_size(env, result, &js_result));

    return js_result;
}

napi_value Key::GenKeyPublic(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(1)

    ARG_TYPE_CHECK(0, napi_object)

    Key* obj;
    CHECK(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));

    // Argument #0
    void* buffer;
    size_t buffer_size;
    if (extract_array_buffer(env, args[0], &buffer, &buffer_size) < 0)
    {
        return nullptr;
    }

    // Result
    int result = dap_enc_gen_key_public(obj->key_, buffer);

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}


napi_value js_dap_enc_key_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;
    int result = dap_enc_key_init();
    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_enc_key_deinit(napi_env env, napi_callback_info info)
{
    dap_enc_key_deinit();
    return nullptr;
}
