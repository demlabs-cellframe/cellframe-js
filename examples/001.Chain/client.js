var dap = require("../../packages/cellframe-sdk-nodejs");
var common = require("./common.js");

// -----------------------------------------

function prepareContext(init_context) {
    init_context.test_dir = './' + process.argv[2] + '/';
    init_context.app_name = 'server';
    init_context.log_path = init_context.test_dir + 'client.log';
}

function setLoggerLevelToDebug(init_context) {
    dap.logger.setLevel("error");
}

function readConfig(init_context) {
    var cfg = new dap.config.Config(init_context.app_name);
    cfg.makeItGlobal();

    init_context.cfg = cfg;
}

function closeConfig(init_context) {
    init_context.cfg.close();
}

var [deinit, vars] = common.init(dap,
    [prepareContext],
    ["common", ["test_dir", "log_path"]],
    [setLoggerLevelToDebug],
    ["config", ["test_dir"]],
    [readConfig, closeConfig],
);

// -----------------------------------------

var socket_path = vars.cfg.getString("conserver", "listen_unix_socket_path");

var connection = new dap.app.cli.Connection(socket_path);
if (connection.isOK()) {
    var argv = process.argv.slice(3);
    if (argv.length == 0) {
        var retCode = connection.shellReaderLoop();
        dap.logger.debug("ShellReaderLoop returned:", retCode);
    } else {
        var retCode = connection.postCommand(argv);
        dap.logger.debug("PostCommand returned:", retCode);
    }

    retCode = connection.disconnect();
} else {
    dap.logger.error("Can't connect to server via socket:", socket_path);
    retCode = -1;
}

// -----------------------------------------

deinit();

// -----------------------------------------

process.exit(retCode);

// -----------------------------------------
