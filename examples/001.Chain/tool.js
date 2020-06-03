var fs = require('fs');
var dap = require("../../packages/cellframe-sdk-nodejs");
var common = require("./common.js");

// -----------------------------------------

function prepareContext(init_context) {
    init_context.test_dir = './' + process.argv[2] + '/';
    init_context.app_name = 'server';
    init_context.log_path = init_context.test_dir + 'tool.log';
}

function setLoggerLevelToDebug(init_context) {
    dap.logger.setLevel("error");
}

function readConfig(init_context) {
    var cfg = new dap.config.Config(init_context.app_name);
    cfg.makeItGlobal();

    init_context.cfg = cfg;

    init_context.wallets_path = cfg.getString("resources", "wallets_path");
    init_context.ca_folders = cfg.getStringArray("resources", "ca_folders");
}

function closeConfig(init_context) {
    init_context.cfg.close();
}


// -----------------------------------------

var [deinit, vars] = common.init(
    dap,
    [prepareContext],
    ["common", ["app_name", "log_path"]],
    [setLoggerLevelToDebug],
    ["config", ["test_dir"]],
    [readConfig, closeConfig],
    ["crypto"],
    ["crypto.cert"],
    ["chain"],
    ["chain.wallet"],
);

// -----------------------------------------

function createWalletCommad(wallet_name, signature_name) {
    dap.chain.wallet.create(wallet_name, vars.wallets_path, signature_name);
    return 0;
}

function createCertificateCommad(name, keyType) {
    var ca_folder = vars.ca_folders[0];
    var filepath = ca_folder + '/' + name + '.dcert';

    if (fs.existsSync(filepath)) {
        dap.logger.critical("File already exists");
        return -1;
    } else {
        dap.crypto.cert.create(name, filepath, keyType);
    }

    return 0;
}

function extractPKeyCommand(certName, newCertName) {
    var ca_folder = vars.ca_folders[0];
    return dap.crypto.cert.extractPublicKeyAsCert(certName, newCertName, ca_folder);
}

function walletCommand(__, command, ...args) {
    if (command === 'create') {
        return createWalletCommad.apply(null, args);
    } else {
        return unknownCommand();
    }
}

function certCommand(__, command, ...args)
{
    if (command === 'create') {
        return createCertificateCommad.apply(null, args);
    } if (command === 'extract_pkey') {
        return extractPKeyCommand.apply(null, args);
    } else {
        return unknownCommand();
    }
}

function unknownCommand() {
    // TODO: print help
    console.log("Unkown command");
    return -1;
}

function main(argv) {
    if (argv[0] === 'wallet') {
        return walletCommand.apply(null, argv);
    } else if (argv[0] === 'cert') {
        return certCommand.apply(null, argv);
    } else {
        return unknownCommand();
    }
}

// -----------------------------------------

process.on('SIGINT', function() {
    dap.logger.debug("SIGINIT received.");
    deinit();
    process.exit(0);
});

/*
    0 = node process path
    1 = current script path
    2 = app folder
*/
var result = main(process.argv.slice(3));

deinit();

process.exit(result);

// -----------------------------------------
