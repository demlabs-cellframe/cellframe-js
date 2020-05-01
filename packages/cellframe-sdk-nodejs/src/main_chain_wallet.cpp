#include <assert.h>
#include <node_api.h>
#include <uv.h>

extern "C" {
#include <dap_sign.h>
#include <dap_chain_wallet.h>
}

#include "utils.h"


#define LOG_TAG "main_chain_wallet_nodejs"


/*
    Chain Node Wallet
*/


napi_value js_dap_chain_wallet_init(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(0)

    int result = dap_chain_wallet_init();

    CHECK(napi_create_int(env, result, &js_result));

    return js_result;
}

napi_value js_dap_chain_wallet_deinit(napi_env env, napi_callback_info info)
{
    dap_chain_wallet_deinit();

    return nullptr;
}

napi_value js_dap_chain_wallet_create(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_valuetype arg_type;
    napi_value js_result = nullptr;

    ARG_COUNT_CHECK_UNIQUE(3)

    ARG_TYPE_CHECK(0, napi_string)
    ARG_TYPE_CHECK(1, napi_string)
    ARG_TYPE_CHECK(2, napi_string)

    char* wallet_name_buffer = extract_str(env, args[0]);
    char* wallets_path_buffer = extract_str(env, args[1]);
    char* signature_type_buffer = extract_str(env, args[2]);

    dap_sign_type_t sig_type = dap_sign_type_from_str(signature_type_buffer);

    if (sig_type.type == SIG_TYPE_NULL)
    {
        napi_throw_type_error(env, nullptr, "Wrong 3rd argument: name of signature type expected");
    }
    else
    {
        // TODO: create wallet class instead?
        /*dap_chain_wallet_t* wallet = */dap_chain_wallet_create(wallet_name_buffer, wallets_path_buffer, sig_type);
    }

    delete[] wallet_name_buffer;
    delete[] wallets_path_buffer;
    delete[] signature_type_buffer;

    return js_result;
}


/*
*/


napi_value ChainWalletInit(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("dap_chain_wallet_init", js_dap_chain_wallet_init),
        DECLARE_NAPI_METHOD("dap_chain_wallet_deinit", js_dap_chain_wallet_deinit),
        DECLARE_NAPI_METHOD("dap_chain_wallet_create", js_dap_chain_wallet_create),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    return exports;
}
