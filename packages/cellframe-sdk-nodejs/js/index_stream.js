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

exports.stream.ctl = {
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

exports.stream.channel = {};

exports.stream.channel.chain = {
    init: function stream_ch_chain_init() {
        return addon.dap_stream_ch_chain_init();
    },
    deinit: function stream_ch_chain_deinit() {
        return addon.dap_stream_ch_chain_deinit();
    },
};

exports.stream.channel.chain.net = {
    init: function stream_ch_chain_net_init() {
        return addon.dap_stream_ch_chain_net_init();
    },
    deinit: function stream_ch_chain_net_deinit() {
        return addon.dap_stream_ch_chain_net_deinit();
    },
};

exports.stream.channel.chain.net.srv = {
    init: function stream_ch_chain_net_srv_init() {
        return addon.dap_stream_ch_chain_net_srv_init();
    },
    deinit: function stream_ch_chain_net_srv_deinit() {
        return addon.dap_stream_ch_chain_net_srv_deinit();
    },
};

