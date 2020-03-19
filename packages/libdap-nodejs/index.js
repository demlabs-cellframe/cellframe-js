var addon = require('bindings')('libdap-nodejs');

var levels = {
    "debug"      : 0,
    "info"       : 1,
    "notice"     : 2,
    "msg"        : 3,
    "message"    : 3,
    "dap"        : 4,
    "warn"       : 5,
    "warning"    : 5,
    "att"        : 6,
    "attention"  : 6,
    "error"      : 7,
    "critical"   : 8,
};

function set_log_level(log_level) {
    var log_level_num = log_level;
    if (Object.prototype.toString.call(log_level) !== "[object Number]") {
        log_level = log_level.toLowerCase();
        if (levels.hasOwnProperty(log_level) == false) {
            throw new Error("Unknown log level");
        }
        log_level_num = levels[log_level];
    }

    return addon.dap_log_level_set(log_level_num);
}

function log_it(log_level) {
    var args = Array.prototype.slice.call(arguments, 1);
    return addon.dap_log_it(log_level, args.join(' '));
}

function bind_log(log_level) {
    return function() {
        var args = Array.prototype.slice.call(arguments);
        return addon.dap_log_it(log_level, args.join(' '));
    };
}

exports.itoa = function itoa(int_value) {
    return addon.dap_itoa(int_value);
};
exports.common = {
    init: function common_init(console_title, log_file_path) {
        return addon.dap_common_init(console_title, log_file_path);
    },
    deinit : function common_deinit() {
        return addon.dap_common_deinit();
    },
};
exports.config = {
    init : function config_init(config_path) {
        return addon.dap_config_init(config_path);
    },
    deinit : function config_deinit() {
        return addon.dap_config_deinit();
    },
    Config: addon.Config,
};

exports.logger = {
    setLevel  : set_log_level,
    logIt     : log_it,
    debug     : bind_log(levels["debug"]),
    info      : bind_log(levels["info"]),
    notice    : bind_log(levels["notice"]),
    msg       : bind_log(levels["msg"]),
    message   : bind_log(levels["message"]),
    dap       : bind_log(levels["dap"]),
    warn      : bind_log(levels["warn"]),
    warning   : bind_log(levels["warning"]),
    att       : bind_log(levels["att"]),
    attention : bind_log(levels["attention"]),
    error     : bind_log(levels["error"]),
    critical  : bind_log(levels["critical"]),
};
