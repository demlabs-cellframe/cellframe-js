var fs = require('fs');
var dap = require("../../packages/cellframe-sdk-nodejs");
var common = require("./common.js");

// -----------------------------------------

function prepareContext(init_context) {
    init_context.test_dir = './' + process.argv[2] + '/';
    init_context.app_name = 'server';
    init_context.log_path = init_context.test_dir + 'server.log';
}

function setLoggerLevelToDebug(init_context) {
    dap.logger.setLevel("debug");
}

function readConfig(init_context) {
    var cfg = new dap.config.Config(init_context.app_name);
    cfg.makeItGlobal();

    init_context.cfg = cfg;

    init_context.server_enabled = cfg.getBool("server", "enabled", false);
    init_context.server_threads = cfg.getInt32("server", "server_threads");
    init_context.events_threads = cfg.getInt32("server", "events_threads");
    init_context.events_timeout = cfg.getInt32("server", "events_timeout");
    init_context.stream_dump_headers = cfg.getBool("server", "stream_dump_headers");
    init_context.stream_ctl_keytype = cfg.getString("server", "stream_ctl_keytype");
    init_context.stream_ctl_keytype = dap.key.tryToConvertKeyType(init_context.stream_ctl_keytype);
    init_context.stream_ctl_keytype_size = cfg.getInt32("server", "stream_ctl_keytype_size");
    init_context.pid_path = cfg.getString("resources", "pid_path");
}

function closeConfig(init_context) {
    init_context.cfg.close();
}

function writePID(init_context) {
    dap.logger.debug("pid_path:", init_context.pid_path,"  PID:", process.pid);
    fs.writeFileSync(init_context.pid_path, String(process.pid), 'utf8');
}

function initServer(init_context) {
    if (init_context.server_enabled) {
        return dap.server.init(init_context.server_threads);
    }

    return 0;
}

function deinitServer(init_context) {
    if (init_context.server_enabled) {
        dap.server.deinit();
    }
}

function startEvents(init_context) {
    var events = dap.events.create();
    events.start();

    init_context.events = events;
}

function createServer(init_context) {
    if (init_context.server_enabled) {
        var address = init_context.cfg.getString("server", "listen_address");
        var port = init_context.cfg.getUint16("server", "listen_port_tcp");

        dap.logger.debug("Address:", address, "Port:", port);
        var server = dap.server.listen(address, port, "tcp");

        dap.http.new(server, init_context.app_name);
        dap.http.enc.addProc(server, "/enc_init");
        dap.stream.addProc(server, "/stream");
        dap.stream.ctl.addProc(server, "/stream_ctl");

        init_context.server = server;
    } else {
        dap.logger.debug("Server is disabled");
    }
}

// -----------------------------------------

var [deinit, vars] = common.init(
    dap,
    [prepareContext],
    ["common", ["app_name", "log_path"]],
    [setLoggerLevelToDebug],
    ["config", ["test_dir"]],
    [readConfig, closeConfig],
    [writePID],
    //["server", ["server_threads"]], // replaced with custom init/deinit pair
    [initServer, deinitServer],
    ["events", ["events_threads", "events_timeout"]],
    ["http"],
    ["http.enc"],
    ["http.simple"],

    [startEvents],
    [createServer],

    ["stream", ["stream_dump_headers"]],
    ["stream.ctl", ["stream_ctl_keytype", "stream_ctl_keytype_size"]],
    ["chain"],
    ["chain.gdb"],
    ["chain.wallet"],
    ["chain.dag"],
    ["chain.dag.pos"],
    ["chain.dag.poa"],
    ["chain.globalDB", ["cfg"]],
    ["chain.net"],
    ["chain.net.service", ["cfg"]],

    // Also starts thread that waits for messages from CLI client
    ["chain.net.cli", ["cfg"]],

    ["stream.channel.chain"],
    ["stream.channel.chain.net"],
    ["stream.channel.chain.net.srv"],
);


// -----------------------------------------

var active = true;

function finish() {
    if (vars.server_enabled) {
        vars.server.stop();
    }
    vars.events.stop();
    active = false;
}

function test_cmd() {
    return "USER STRING";
}

dap.chain.net.cli.addCmd("user", test_cmd, undefined, "This is a test func", "Test func returns 'USER STRING' and does nothing else");

function exit_cmd() {
    finish();
    return "Done.";
}

dap.chain.net.cli.addCmd("exit", exit_cmd, undefined, "Terminate node", "Workaround for existing exit function that just kills process");

process.on('SIGINT', function() {
    dap.logger.debug("SIGINIT received.");
    finish();
});

if (vars.server_enabled) {
    var returnCode = vars.server.start();
    if (returnCode !== 0) {
        dap.logger.critical("Can't start server. Error code:", returnCode);
        vars.server_enabled = false;
        finish();
    }
}

// To prevent node proccess from exiting
(function wait () {
    if (active) setTimeout(wait, 200);
    else {
        vars.events.wait();
        deinit();
    }
})();

// -----------------------------------------
