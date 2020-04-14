var dapStream = require("./index.js");
var assert = require("assert").strict;

console.log("Testing Stream...");
console.log("Stream init");
var _1 = dapStream.stream.init(false);
console.log("Stream init return code:", _1);
assert.equal(_1, 0);
console.log("Stream ctl init");
var _2 = dapStream.streamCtl.init(0, 0);
console.log("Stream ctl init return code:", _2);
assert.equal(_2, 0);

try {
    console.log("Stream add proc");
    dapStream.stream.addProc(null, "");
    console.log("Stream add proc didn't fail as expected");
    process.exit(-1);
} catch(e) {
    console.log("Stream add proc failed as expected");
} try {
    console.log("Stream ctl add proc");
    dapStream.streamCtl.addProc(null, "");
    console.log("Stream ctl add proc didn't fail as expected");
    process.exit(-2);
}catch(e){
    console.log("Stream ctl add proc failed as expected");
}

console.log("Stream ctl deinit");
dapStream.streamCtl.deinit();
console.log("Stream deinit");
dapStream.stream.deinit();
