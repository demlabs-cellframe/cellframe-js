#include <cmath>
#include <assert.h>
#include <node_api.h>



#include "utils.h"
#include "server.h"
#include "events.h"
#include "events_socket.h"



/*
*/


static napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor desc[] = {
        //DECLARE_NAPI_METHOD("dap_server_init", js_dap_server_init),
    };
    CHECK(napi_define_properties(env, exports, sizeof(desc)/sizeof(desc[0]), desc));

    Server::Init(env, exports);
    Events::Init(env, exports);
    EventsSocket::Init(env, exports);

    return exports;
}


NAPI_MODULE(libdap_server_core_nodejs, Init)
