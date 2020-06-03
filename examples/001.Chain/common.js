function getChildWithPath(parent, path_str) {
    var parts = path_str.split('.');
    var child = parent;
    for (var i = 0, l = parts.length; i < l; ++i) {
        if (parts[i] in child) {
            child = child[parts[i]];
        } else {
            throw new Error("Can't find part: " + parts[i] + "  in path: " + path_str);
        }
    }
    return child;
}

function init(root) {
    var stack = [];
    var context = {};

    for (var i = 1, l = arguments.length; i < l; ++i) {
        if (Object.prototype.toString.call(arguments[i]) !== "[object Array]") {
            throw new Error("Arguments must be arrays");
        }

        var array = arguments[i];

        if (array.length === 0) {
            throw new Error("One of arrays is empty");
        }
    }

    for (var i = 1, l = arguments.length; i < l; ++i) {
        var array = arguments[i];
        var first = array[0];

        if (Object.prototype.toString.call(first) === "[object String]") {
            var obj = getChildWithPath(root, first);

            stack.push(first);

            if (array.length > 1) {
                var arg_names = array[1];
                var args = arg_names.map(x => context[x]);

                try {
                    //console.log("[DEBUG] Calling init of", first, "with args", args);
                    var result = obj.init.apply(obj, args);

                    if (Object.prototype.toString.call(result) === "[object Number]") {
                        if (result !== 0) {
                            throw new Error("Init function returned code = " + result);
                        }
                    } else {
                        //console.log("[DEBUG] Unexpected result:", result);
                    }
                } catch (e) {
                    throw new Error("Can't init module: " + first + "\nArgument names: " + arg_names + "\nArgument values: " + args + "\nError message: " + e.message);
                }
            } else {
                //console.log("[DEBUG] Calling init of", first);
                obj.init();
            }
        } else if (Object.prototype.toString.call(first) === "[object Function]") {
            //console.log("[DEBUG] Calling", first.name, "function");
            first(context);

            if (array.length > 1) {
                var second = array[1];

                if (Object.prototype.toString.call(second) === "[object Function]") {
                    stack.push(second);
                } else {
                    throw new Error("Unknown pair element for function " + first.name);
                }
            }
        } else {
            throw new Error("Unknown init element: " + first);
        }
    }

    return [function deinit() {
        for (var i = 0, l = stack.length; i < l; ++i) {
            var elem = stack.pop();

            if (Object.prototype.toString.call(elem) === "[object String]") {
                //process.stdout.write("[DEBUG] Calling deinit of " + elem + " ... ");
                var obj = getChildWithPath(root, elem);
                obj.deinit();
                //console.log("Done");
            } else if (Object.prototype.toString.call(elem) === "[object Function]") {
                //process.stdout.write("[DEBUG] Calling deinit function " + elem.name + " ... ");
                elem(context);
                //console.log("Done");
            } else {
                throw new Error("Unknown deinit element: " + elem);
            }
        }
    }, context];
}

exports.getChildWithPath = getChildWithPath;
exports.init = init;
