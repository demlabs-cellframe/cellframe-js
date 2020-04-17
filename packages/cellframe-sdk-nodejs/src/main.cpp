#include <node_api.h>


napi_value CoreInit(napi_env env, napi_value exports);
napi_value CryptoInit(napi_env env, napi_value exports);
napi_value ServerCoreInit(napi_env env, napi_value exports);
napi_value StreamInit(napi_env env, napi_value exports);
napi_value ServerInit(napi_env env, napi_value exports);
napi_value ChainNetInit(napi_env env, napi_value exports);


static napi_value Init(napi_env env, napi_value exports)
{
    exports = CoreInit(env, exports);
    exports = CryptoInit(env, exports);
    exports = ServerCoreInit(env, exports);
    exports = StreamInit(env, exports);
    exports = ServerInit(env, exports);
    exports = ChainNetInit(env, exports);
    return exports;
}


NAPI_MODULE(cellframe_sdk_nodejs, Init)
