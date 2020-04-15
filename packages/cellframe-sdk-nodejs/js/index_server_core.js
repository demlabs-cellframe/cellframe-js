var addon = require('bindings')('cellframe-sdk-nodejs');

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

exports.socket = {
    EventsSocket: addon.EventsSocket,
    find: function socket_find(sock, events_instance) {
        return new addon.EventsSocket(sock, events_instance);
    },
    init : function socket_init() {
        return addon.dap_events_socket_init();
    },
    deinit : function socket_deinit() {
        return addon.dap_events_socket_deinit();
    },
};
