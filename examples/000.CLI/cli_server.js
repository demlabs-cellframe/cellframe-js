var example = require("./cli_common.js");
var dap = require("../../packages/cellframe-sdk-nodejs");

// -----------------------------------------

dap.common.init(example.app_name, example.test_dir + 'cli_server.log');

dap.logger.setLevel("debug");

dap.config.init(example.test_dir);


var cfg = new dap.config.Config(example.app_name);
cfg.makeItGlobal();

// Also starts thread that waits for messages from CLI client
dap.chain.net.cli.init(cfg);

// -----------------------------------------

function test_cmd() {
    return "USER STRING";
}

dap.chain.net.cli.addCmd("user", test_cmd, undefined, "This is a test func", "Test func returns 'USER STRING' and does nothing else");

var active = true;

// To prevent node proccess from exiting
(function wait () {
   if (active) setTimeout(wait, 200);
})();

process.on('SIGINT', function() {
    dap.logger.debug("SIGINIT received.");
    deinit();
    process.exit(0);
});

// -----------------------------------------

function deinit() {
    dap.logger.debug("Deinit function");
    dap.chain.net.cli.deinit()
    cfg.close();
    dap.config.deinit();
    dap.common.deinit();
}

// -----------------------------------------
