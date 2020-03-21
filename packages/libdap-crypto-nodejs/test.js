var dapCrypto = require("./index.js");

var textEncoder = new TextEncoder();
var textDecoder = new TextDecoder("utf-8");

var test_strings = ["", "a", "aa", "aaa", "aaaa", "aaaaa"];
for (var i = 0, l = test_strings.length; i < l; ++i)
{
    var str = test_strings[i];
    var u8array = textEncoder.encode(str);
    var encoded = dapCrypto.base64.encode(u8array.buffer, "b64");
    var arrayBuffer = dapCrypto.base64.decode(encoded, "b64");
    var decoded = textDecoder.decode(arrayBuffer);

    console.log("Test 'Encode + Decode' #" + i);
    console.log("Data:   ", str, " --> ", encoded);
    console.log("Decoded:", decoded);
    console.log("Array Before:", u8array.buffer);
    console.log("Array After :", arrayBuffer);
    console.log();
}
