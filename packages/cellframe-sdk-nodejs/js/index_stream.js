var addon = require('bindings')('cellframe-sdk-nodejs');

exports.stream = {
    init: function stream_init(dumpPacketHeaders) {
        return addon.dap_stream_init(dumpPacketHeaders);
    },
    deinit: function stream_deinit() {
        return addon.dap_stream_deinit();
    },
    addProc: function stream_add_proc(server, urlComponent) {
        return addon.dap_stream_add_proc_http(server, urlComponent);
    },
};

exports.streamCtl = {
    init: function stream_ctl_init(keyType, size) {
        return addon.dap_stream_ctl_init(keyType, size);
    },
    deinit: function stream_ctl_deinit() {
        return addon.dap_stream_ctl_deinit();
    },
    addProc: function stream_ctl_add_proc(server, urlComponent) {
        return addon.dap_stream_ctl_add_proc(server, urlComponent);
    },
};
