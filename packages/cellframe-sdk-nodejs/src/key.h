#pragma once

#include <node_api.h>
#include <dap_enc_key.h>

bool is_dap_key_type_valid(int32_t key_type);

class Key
{
public:
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

private:
    explicit Key(dap_enc_key_type_t a_key_type);
    explicit Key(dap_enc_key_type_t a_key_type,
                 const void *kex_buf,
                 size_t kex_size, const void* seed,
                 size_t seed_size, size_t key_size);
    ~Key();

    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value GetEncSize(napi_env env, napi_callback_info info);
    static napi_value GetDecSize(napi_env env, napi_callback_info info);
    static napi_value SerializePrivateKey(napi_env env, napi_callback_info info);
    static napi_value SerializePublicKey(napi_env env, napi_callback_info info);
    static napi_value DeserializePrivateKey(napi_env env, napi_callback_info info);
    static napi_value DeserializePublicKey(napi_env env, napi_callback_info info);
    static napi_value Update(napi_env env, napi_callback_info info);
    static napi_value GenKeyPublicSize(napi_env env, napi_callback_info info);
    static napi_value GenKeyPublic(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

    dap_enc_key_t* key_;
};

napi_value js_dap_enc_key_init(napi_env env, napi_callback_info info);
napi_value js_dap_enc_key_deinit(napi_env env, napi_callback_info info);
