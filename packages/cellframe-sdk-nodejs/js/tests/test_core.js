var fs = require('fs');
var dap = require("../index_core.js");

var test_dir = './test.tmp/';
var app_name = 'test_app';
var log_file = test_dir + 'test.log';
var config_file = test_dir + app_name + '.cfg';

if (fs.existsSync(test_dir))
{
    fs.rmdirSync(test_dir, {recursive: true});
}
fs.mkdirSync(test_dir);
fs.writeFileSync(config_file, "[server]\nlisten_address=0.0.0.0\n");


dap.common.init(app_name, log_file);
dap.config.init(test_dir);

dap.logger.setLevel(0);
dap.logger.setLevel("DeBUg");

console.log('itoa(10) = ' + dap.itoa(10)); // '10'

dap.logger.error("OOPS ERROR");
dap.logger.critical("OOPS CRITICAL");
dap.logger.logIt(3, "MSG KIND of a MESSAGE");

var cfg = new dap.config.Config(app_name);
var address = cfg.getString("server", "listen_address")
console.log("address = " + address);
var address_with_default = cfg.getString("server", "listen_address", "8.8.8.8")
console.log("address_with_default = " + address_with_default);

var missing_key = cfg.getString("unknown", "what?");
var missing_key_with_default = cfg.getString("unknown", "what?", "default_value");
console.log("missing_key = " + missing_key);
console.log("missing_key_with_default = " + missing_key_with_default);

dap.logger.debug("log", new String("message"), String("multiple"), 'arguments');
