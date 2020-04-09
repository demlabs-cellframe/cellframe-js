var dapServer = require("./index.js");

console.log("Testing HTTP...")
console.log("HTTP Init");
dapServer.http.init()
console.log("HTTP ENC Init");
dapServer.httpEnc.init()

try {
    console.log("HTTP NEW");
    dapServer.http.new(null, 1);
    console.log("HTTP NEW didn't fail as expected");
    process.exit(-1);
} catch(e) {
    console.log("HTTP NEW failed as expected");
} try {
    console.log("HTTP ENC ADD PROC");
    dapServer.httpEnc.addProc(null, 1);
    console.log("HTTP ENC ADD PROC didn't fail as expected");
    process.exit(-2);
}catch(e){
    console.log("HTTP ENC ADD PROC failed as expected");
}

console.log("HTTP ENC Deinit");
dapServer.httpEnc.deinit()
console.log("HTTP Deinit");
dapServer.http.deinit()
console.log();
