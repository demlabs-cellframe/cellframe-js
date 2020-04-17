var fs = require("fs");

var files = fs.readdirSync(__dirname + "/js/");
var objects = files
    .filter(RegExp.prototype.test.bind(/^index_.+\.js$/))
    .map(x => "./js/" + x)
    .map(require);

objects.splice(0, 0, exports);

exports = Object.assign.apply(null, objects);
