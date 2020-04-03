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
    deinit: function server_deinit() {
        return addon.dap_server_deinit();
    },
};

exports.events = {
    Events: addon.Events,
    create: function events_create() {
        return new addon.Events();
    },
    init: function events_init(threads_count, connection_timeout) {
        return addon.dap_events_init(threads_count, connection_timeout);
    },
    deinit: function events_deinit() {
        return addon.dap_events_deinit();
    },
};
