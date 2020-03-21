var dapCrypto = require("./index.js");

var textEncoder = new TextEncoder();
var textDecoder = new TextDecoder("utf-8");

var test_strings = ["", "a", "aa", "aaa", "aaaa", "aaaaa", "git@gitlab.demlabs.net:cellframe/libdap.git"];

function test_encode_decode(name, array, encode, decode)
{
    for (var i = 0, l = array.length; i < l; ++i)
    {
        var str = array[i];
        var u8array = textEncoder.encode(str);
        var encoded = encode(u8array.buffer);
        var arrayBuffer = decode(encoded);
        var decoded = textDecoder.decode(arrayBuffer);

        console.log("Test '" + name + " Encode + Decode' #" + i);
        console.log("Data:   ", str, " --> ", encoded);
        console.log("Decoded:", decoded);
        console.log("Array Before:", u8array.buffer);
        console.log("Array After :", arrayBuffer);
        console.log();
    }
}

test_encode_decode("B64", test_strings, x => dapCrypto.base64.encode(x, "b64"), x => dapCrypto.base64.decode(x, "b64"));
test_encode_decode("B64_URLSAFE", test_strings, x => dapCrypto.base64.encode(x, "b64_urlsafe"), x => dapCrypto.base64.decode(x, "b64_urlsafe"));
test_encode_decode("B58", test_strings, dapCrypto.base58.encode, dapCrypto.base58.decode);
