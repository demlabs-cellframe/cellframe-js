var addon = require('bindings')('cellframe-sdk-nodejs');

exports.http = {
    init: function http_init() {
        return addon.dap_http_init();
    },
    deinit: function http_deinit() {
        return addon.dap_http_deinit();
    },
    new: function http_new(server, url) {
        return addon.dap_http_new(server, url);
    },
};

exports.http.enc = {
    init: function enc_http_init() {
        return addon.enc_http_init();
    },
    deinit: function enc_http_deinit() {
        return addon.enc_http_deinit();
    },
    addProc: function add_proc(server, url) {
        return addon.enc_http_add_proc(server, url);
    },
};

exports.http.simple = {
    init: function http_simple_init() {
        return addon.dap_http_simple_module_init();
    },
    deinit: function http_simple_deinit() {
        return addon.dap_http_simple_module_deinit();
    },
};
