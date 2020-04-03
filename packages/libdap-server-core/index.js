var addon = require('bindings')('libdap-server-core-nodejs');

var ServerTypes = {
    tcp: 0,
};

function tryToConvertType(type) {
    if (Object.prototype.toString.call(type) === "[object String]") {
        type = type.toLowerCase();
        if (type in ServerTypes) {
            return ServerTypes[type];
        }
    }

    return type;
}

exports.server = {
    Server: addon.Server,
    ServerTypes: ServerTypes,
    listen: function server_listen(address, port, type) {
        return new addon.Server(address, port, tryToConvertType(type));
    },
    init: function server_init(threads_count) {
        return addon.dap_server_init(threads_count);
    },
    deinit: function server_init() {
        return addon.dap_server_deinit();
    },
};
