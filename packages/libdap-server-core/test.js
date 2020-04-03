var dapServerCore = require("./index.js");


console.log("Server inititialization");
dapServerCore.server.init(4);
console.log("Server creation");
var server = dapServerCore.server.listen("0.0.0.0", 8182, "tcp");
console.log("Server deinititialization");
dapServerCore.server.deinit();
console.log();


