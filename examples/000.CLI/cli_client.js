var example = require("./cli_common.js");
var dap = require("../../packages/cellframe-sdk-nodejs");

// -----------------------------------------

var log_file = example.test_dir + 'client.log';

dap.common.init(example.app_name, log_file);

dap.logger.setLevel("debug");

dap.config.init(example.test_dir);


var cfg = new dap.config.Config(example.app_name);
cfg.makeItGlobal();

// -----------------------------------------

var socket_path = cfg.getString("conserver", "listen_unix_socket_path");

var connection = new dap.app.cli.Connection(socket_path);
if (connection.isOK() == false) {
    dap.logger.error("Can't connect to server via socket:", socket_path);
}
var argv = process.argv.slice(2);
if (argv.length == 0) {
    var retCode = connection.shellReaderLoop();
    dap.logger.debug("ShellReaderLoop returned:", retCode);
} else {
    var retCode = connection.postCommand(argv);
    dap.logger.debug("PostCommand returned:", retCode);
}

retCode = connection.disconnect();

// -----------------------------------------

cfg.close();
dap.config.deinit();
dap.common.deinit();

// -----------------------------------------

process.exit(retCode);

// -----------------------------------------
