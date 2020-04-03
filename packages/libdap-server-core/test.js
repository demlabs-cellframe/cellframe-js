var dapServerCore = require("./index.js");


console.log("Server inititialization");
dapServerCore.server.init(4);
console.log("Server creation");
var server = dapServerCore.server.listen("0.0.0.0", 8182, "tcp");
console.log("Server deinititialization");
dapServerCore.server.deinit();
server = null;
console.log();


console.log("Events inititialization");
dapServerCore.events.init(4, 6000);
console.log("Events creation");
var events = dapServerCore.events.create();
console.log("Events start");
events.start();
events = null;
console.log("Events deinititialization");
dapServerCore.events.deinit();
