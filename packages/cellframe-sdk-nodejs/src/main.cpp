#include "utils.h"

napi_value CoreInit(napi_env env, napi_value exports);
napi_value CryptoInit(napi_env env, napi_value exports);
napi_value ServerCoreInit(napi_env env, napi_value exports);
napi_value StreamInit(napi_env env, napi_value exports);
napi_value ServerInit(napi_env env, napi_value exports);
napi_value ChainInit(napi_env env, napi_value exports);
napi_value ChainWalletInit(napi_env env, napi_value exports);
napi_value ChainGDBInit(napi_env env, napi_value exports);
napi_value ChainCSDAGInit(napi_env env, napi_value exports);
napi_value ChainNetInit(napi_env env, napi_value exports);
napi_value ChainNetSrvInit(napi_env env, napi_value exports);
napi_value ChainGlobalDBInit(napi_env env, napi_value exports);
napi_value ApplicationCLIInit(napi_env env, napi_value exports);


static napi_value Init(napi_env env, napi_value exports)
{
    utils_init();

    exports = CoreInit(env, exports);
    exports = CryptoInit(env, exports);
    exports = ServerCoreInit(env, exports);
    exports = StreamInit(env, exports);
    exports = ServerInit(env, exports);
    exports = ChainInit(env, exports);
    exports = ChainWalletInit(env, exports);
    exports = ChainGDBInit(env, exports);
    exports = ChainCSDAGInit(env, exports);
    exports = ChainNetInit(env, exports);
    exports = ChainNetSrvInit(env, exports);
    exports = ChainGlobalDBInit(env, exports);
    exports = ApplicationCLIInit(env, exports);
    return exports;
}


NAPI_MODULE(cellframe_sdk_nodejs, Init)
