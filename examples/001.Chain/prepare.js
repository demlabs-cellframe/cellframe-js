var fs = require("fs");
var path = require("path");
var assert = require("assert").strict;
var { execSync, spawn } = require('child_process');

/*
var old_execSync = execSync;
execSync = function(str) {
    console.debug("EXEC:", str);
    var _1 = old_execSync(str);
    console.debug("RESULT:", _1.toString());
    return _1;
};
*/

// Config
var nodeCount = 2;
var rootNodesCount = 1;
var basePortNum = 8090;
var localhost = "127.0.0.1";
var walletSignature = "sig_dil";
var certKeyType = "sig_dilithium";
var poaCertName = "poa";
var poaCertPublicName = "poa.pub";
var netName = 'devnet';
var seedNodeIndex = 0;
var tokenName = 'KIM';
var expectedPlasmaEventsCount = 1;
var expectedZerochainEventsCount = 3;
var sharedCertificatesFolder = "./node_shared/ca";

// Checks
assert(nodeCount > 0);
assert(nodeCount < 10000);
assert(rootNodesCount > 0);
assert(rootNodesCount <= nodeCount);
assert(basePortNum > 0);
assert(basePortNum < Math.pow(2, 16) - nodeCount);
assert(seedNodeIndex >= 0);
assert(seedNodeIndex < rootNodesCount);

//
function isRoot(num) {
    return num < rootNodesCount;
}
function makeAddr(num) {
    var lastpart = ("0000" + num).slice(-4);
    return "1234::0000::0000::" + lastpart;
}
function makeAlias(role, num) {
    return `${netName}.${num}.${role}`;
}
function getRole(num) {
    if (isRoot(num)) return "root";
    return "master";
}
function getSeedMode(num) {
    if (num === seedNodeIndex) return "true";
    return "false";
}
function getWalletName(num) {
    return `wallet_${num}`;
}
function makePort(num) {
    return basePortNum + num;
}

function replace(str, dict) {
    for (key in dict) {
        if (dict.hasOwnProperty(key)) {
            str = str.split(key).join(dict[key]);
        }
    }

    return str;
}

var folders = [];

var hostnames = [];
var aliases = [];
var addrs = [];
var ports = [];
for (var nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex) {
    var role = getRole(nodeIndex);
    hostnames.push(localhost);
    aliases.push(makeAlias(role, nodeIndex));
    addrs.push(makeAddr(nodeIndex));
    ports.push(makePort(nodeIndex));
}

for (var nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex) {
    var nodeFolder = `node_${nodeIndex}`;
    var certificatesFolder = path.join(nodeFolder, "ca");
    var walletsFolder = path.join(nodeFolder, "wallets");
    var networkPath = path.join(nodeFolder, "network")
    var devnetPath = path.join(networkPath, netName)
    fs.mkdirSync(devnetPath, {recursive:true});
    fs.mkdirSync(certificatesFolder);
    fs.mkdirSync(walletsFolder);
    fs.mkdirSync(sharedCertificatesFolder, {recursive:true});

    folders.push(nodeFolder);

    var walletName = getWalletName(nodeIndex);

    var replacements = {
        "$node_num": `${nodeIndex}`,
        "$node_role": getRole(nodeIndex),
        "$node_port": String(makePort(nodeIndex)),
        "$node_wallet": walletName,
        "$node_addr": makeAddr(nodeIndex),
        "$node_cert_poa": `${poaCertName}.${nodeIndex}`,
        "$node_cert_prefix_poa": poaCertPublicName,
        "$hostnames": replace(JSON.stringify(hostnames), {'"':''}),
        "$aliases": replace(JSON.stringify(aliases), {'"':''}),
        "$addrs": replace(JSON.stringify(addrs), {'"':''}),
        "$ports": replace(JSON.stringify(ports), {'"':''}),
        "$root_nodes_count": String(rootNodesCount),
        "$seed_mode": getSeedMode(nodeIndex),
        "$tokens_hold": `[${tokenName}]`,
        "$tokens__hold_value": "[1000000000]",
        "$listen_unix_socket_path": `/tmp/node_cli_${nodeIndex}`,
        "$pid_path": `${nodeFolder}/node_${nodeIndex}.pid`,
        "$log_file": `${nodeFolder}/node_${nodeIndex}.log`,
        "$wallets_path" : walletsFolder,
        "$ca_folders": `[${certificatesFolder},${sharedCertificatesFolder}]`,
        "$dap_global_db_path": `${nodeFolder}/global_db`,
        "$net_name": netName,
        "$zerochain_storage_dir": `${nodeFolder}/network/${netName}/zerochain`,
        "$auth_certs_dir": sharedCertificatesFolder,
        "$plasma_storage_dir": `${nodeFolder}/network/${netName}/plasma`,
    };

    var tempaltesAndFolders = [
        ["server.cfg", nodeFolder],
        [`${netName}.cfg`, networkPath],
        ["chain-0.cfg", devnetPath],
        ["chain-plasma.cfg", devnetPath],
    ];

    for (var j = 0; j < tempaltesAndFolders.length; ++j) {
        var [filename, folder] = tempaltesAndFolders[j];
        var tmp = fs.readFileSync(path.join("cfg_templates", `${filename}.in`), 'utf8');
        var result = replace(tmp, replacements);
        var resultPath = path.join(folder, filename);
        fs.writeFileSync(resultPath, result, 'utf8');
    }

    execSync(`node tool.js "${nodeFolder}" wallet create "${walletName}" "${walletSignature}"`);
    execSync(`node tool.js "${nodeFolder}" cert create "${poaCertName}.${nodeIndex}" "${certKeyType}"`);
    if (isRoot(nodeIndex)) {
        execSync(`node tool.js "${nodeFolder}" cert extract_pkey "${poaCertName}.${nodeIndex}" "${poaCertPublicName}.${nodeIndex}"`);
    }
}

// Copy public certificates to shared folder
for (var root_node_index = 0; root_node_index < rootNodesCount; ++root_node_index) {
    var public_key_path = path.join(folders[root_node_index], "ca", `${poaCertPublicName}.${root_node_index}.dcert`);
    var target_path = path.join(sharedCertificatesFolder, `${poaCertPublicName}.${root_node_index}.dcert`);
    fs.copyFileSync(public_key_path, target_path);
}

function parseOutput(output, firstStr, separator, valueIndex) {
    var lines = output.toString().split('\n').filter(x => x.startsWith(firstStr));
    var values = lines.map(x => x.split(separator)[valueIndex]);
    return values;
}

function parseFirstIdFromOutput(output, netName) {
    var parts = output.toString().split('\n');
    var datumIndex = -1;
    for (var i = 0; i < parts.length; ++i) {
        if (parts[i].startsWith(netName)) {
            datumIndex = i + 1;
            break;
        }
    }
    var datumId = null;
    if (datumIndex > 0 && parts[datumIndex].indexOf(':') > 0) {
        datumId = parts[datumIndex].split(':')[0].trim();
        assert(datumId.length === 66);
    } else {
        throw new Error("Can't parse id.\nOUTPUT>>>\n" + output.toString() + "\n<<<OUTPUT");
    }

    return datumId;
}

function Wait(timeout) {
    return new Promise(function(resolve, reject) {
        setTimeout(resolve, timeout);
    });
}

async function Try(times, cmd) {
    var lastError = null;
    while (times --> 0) {
        try {
            return execSync(cmd);
        } catch (e) {
            lastError = e;
            await Wait(200)
        }
    }

    throw lastError;
}

function checkEvents(nodeFolder, netName) {
    var output10 = execSync(`node ./client.js "${nodeFolder}" dag -net "${netName}" -chain plasma event list -from events`);
    var plasmaEventsCount = parseInt(parseOutput(output10, netName, " ", 2)[0], 10);
    assert.equal(plasmaEventsCount, expectedPlasmaEventsCount);
    var plasmaEventId = parseFirstIdFromOutput(output10, netName);

    var output11 = execSync(`node ./client.js "${nodeFolder}" dag -net "${netName}" -chain zerochain event list -from events`);
    var zerochainEventsCount = parseInt(parseOutput(output11, netName, " ", 2)[0], 10);
    assert.equal(zerochainEventsCount, expectedZerochainEventsCount);
    var zerochainEventId = parseFirstIdFromOutput(output11, netName);

    return {zerochainEventId,plasmaEventId};
}

async function PrepareNodes() {
    var nodeFolder = folders[seedNodeIndex];
    var tmpWalletName = 'wallet_tmp';
    var certName = `${poaCertName}.${seedNodeIndex}`;
    var walletName = getWalletName(seedNodeIndex);

    // Step #1: Declare token
    process.stdout.write('.');
    var output = await Try(50, `node ./client.js "${nodeFolder}" token_decl -net "${netName}" -chain zerochain -token "${tokenName}" -total_supply 1000000000000000000 -signs_total 1 -signs_emission 1 -certs "${certName}" -type private -flags ALL_ALLOWED`);
    var datumId = parseOutput(output, "datum", " ", 1)[0];
    var output2 = execSync(`node ./client.js "${nodeFolder}" mempool_proc -net "${netName}" -chain zerochain -datum ${datumId}`);

    // Step #2: Get wallet address
    process.stdout.write('.');
    var output3 = execSync(`node ./client.js "${nodeFolder}" wallet info -w "${walletName}" -net "${netName}"`);
    var walletAddress = parseOutput(output3, "addr:", " ", 1)[0];
    assert(walletAddress != null);

    // Step #3: Emission
    process.stdout.write('.');
    var output4 = execSync(`node ./client.js "${nodeFolder}" token_emit -net "${netName}" -chain_emission zerochain -chain_base_tx zerochain -addr ${walletAddress} -token "${tokenName}" -certs "${certName}" -emission_value 1000000000000000`);
    var datumIdList = parseOutput(output4, "datum", " ", 2);

    for (var i = 0; i < datumIdList.length; ++i) {
        execSync(`node ./client.js "${nodeFolder}" mempool_proc -net "${netName}" -chain zerochain -datum ${datumIdList[i]}`);
    }

    // Step #4: Create new wallet
    process.stdout.write('.');
    var output5 = execSync(`node tool.js "${nodeFolder}" wallet create "${tmpWalletName}" "${walletSignature}"`);

    var output6 = execSync(`node ./client.js "${nodeFolder}" wallet info -w "${tmpWalletName}" -net "${netName}"`);
    var tmpWalletAddress = parseOutput(output6, "addr:", " ", 1)[0];

    // Step #5: Move some coins to the new wallet
    process.stdout.write('.');
    var output7 = execSync(`node ./client.js "${nodeFolder}" tx_create -net "${netName}" -chain plasma -from_wallet "${walletName}" -to_addr "${tmpWalletAddress}" -token "${tokenName}" -value 1000000000000`);

    var output8 = execSync(`node ./client.js "${nodeFolder}" mempool_list -net "${netName}" -chain plasma`);
    var transactionDatumId = parseFirstIdFromOutput(output8, netName);

    var output9 = execSync(`node ./client.js "${nodeFolder}" mempool_proc -net "${netName}" -chain plasma -datum ${transactionDatumId}`);

    // Step #6: Check events count
    process.stdout.write('.');
    return checkEvents(nodeFolder, netName);
}

function UpdateConfigs(zerochainEventId, plasmaEventId) {
    process.stdout.write('.');

    // Set genesis events, disable seed mode, disable transactions for zerochain.
    var replacements = {
        "$is_static_genesis_event": "true",
        "$static_genesis_event": zerochainEventId,
        "seed_mode=true": "seed_mode=false",
        "$plasma_is_static_genesis_event": "true",
        "$plasma_static_genesis_event": plasmaEventId,
        "#is_static_genesis_event": "is_static_genesis_event",
        "#static_genesis_event": "static_genesis_event",
        "datum_types=[token,emission,transaction,shard,ca]": "datum_types=[token,emission,shard,ca]",
    };

    for (var nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex) {
        var nodeFolder = `node_${nodeIndex}`;
        var chains_folder = path.join(nodeFolder, "network", netName);

        var tempaltesAndFolders = [
            ["server.cfg", nodeFolder],
            ["chain-0.cfg", chains_folder],
            ["chain-plasma.cfg", chains_folder],
        ];

        for (var j = 0; j < tempaltesAndFolders.length; ++j) {
            var [filename, folder] = tempaltesAndFolders[j];
            var tmp = fs.readFileSync(path.join(folder, filename), 'utf8');
            var result = replace(tmp, replacements);
            var resultPath = path.join(folder, filename);
            fs.writeFileSync(resultPath, result, 'utf8');
        }
    }
}

function spawnNode(nodeIndex) {
    var nodeProcess = spawn('node', ['./server.js', folders[nodeIndex]], {'stdio': ['ignore', 'ignore', 'ignore']});

    function killRunningNode() {
        var nodePromise = new Promise((resolve, reject) => nodeProcess.on('exit', resolve));
        nodeProcess.kill('SIGINT');
        return nodePromise;
    }

    return killRunningNode;
}

// Start Seed Node
var killRunningNode = spawnNode(seedNodeIndex)

var prepared = PrepareNodes();
var nodeIsDone = prepared.finally(killRunningNode);

var preparationsAreSucceeded = prepared.then(function success(data) {
    process.stdout.write('.');
    var {zerochainEventId, plasmaEventId} = data;
    return nodeIsDone.then(_ => UpdateConfigs(zerochainEventId, plasmaEventId));
});

var preparationsAreFinished = Promise.allSettled([nodeIsDone, preparationsAreSucceeded]).finally(_ => console.log());


// Check that newly created net is functioning properly
var attempts = 5; // TODO: move to config
var allNodesAreInSync = false;
function onSyncError() {
    attempts -= 1;
}
function checkEventsOnEveryNode() {
    folders.forEach(nodeFolder => checkEvents(nodeFolder, netName));
    allNodesAreInSync = true;
}

function CheckAllNodes() {
    if (allNodesAreInSync) {
        return Promise.resolve(true);
    } else if (Object.prototype.toString.call(attempts) !== "[object Number]" || attempts < 0) {
        console.error("Invalid attempts value");
        return Promise.reject();
    } else if (attempts === 0) {
        console.error("Nodes are not in sync. Remove node folders and try again.");
        return Promise.reject();
    }

    process.stdout.write('.');

    // Step #1: Launch all nodes
    var nodeProcesses = folders.map(nodeFolder => spawn('node', ['./server.js', nodeFolder], {'stdio': ['ignore', 'pipe', 'ignore']}));
    var killFunctions = nodeProcesses.map(nodeProcess => function killRunningNode() {
        var nodePromise = new Promise((resolve, reject) => nodeProcess.on('exit', resolve));
        nodeProcess.kill('SIGINT');
        return nodePromise;
    });

    // Step #2: Wait until they all are in sync
    var syncLine = '[INF] [chain_net] Synchronization done';
    var syncPromises = nodeProcesses.map(function(subprocess) {
        return new Promise(function(resolve, reject) {
            var prevPart = '';
            subprocess.stdout.on('data', function(chunk) {
                var data = prevPart + chunk;
                if (data.indexOf(syncLine) >= 0) {
                    process.stdout.write('.');
                    resolve(true);
                } else {
                    prevPart = chunk.slice(-1 * syncLine.length);
                }
            });

            subprocess.stdout.on('error', reject);
            subprocess.stdout.on('end', reject);
            subprocess.stdout.on('close', reject);
        });
    });

    // Step #3: Check events on every node. If at least one is not - relaunch all and check again
    return Promise.all(syncPromises).then(checkEventsOnEveryNode).catch(onSyncError).finally(_ => {
        return Promise.allSettled(killFunctions.map(f => f())).then(CheckAllNodes);
    });
}

Promise.all([preparationsAreFinished, preparationsAreSucceeded]).then(CheckAllNodes).catch(console.error.bind(console)).finally(_ => console.log());
