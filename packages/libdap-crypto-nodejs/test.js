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


var numbers = [125,52,56,121,119,94,123,101,83,85,80,53,44,84,66,49,92,54,55,93,89,104,34,116,88,120,63,118,109,36,77,107,51,117,103,113,70,59,50,39,79,38,96,69,65,41,105,126,91,102,98,68,81,86,33,71,67,106,97,122,60,111,124,74,75,87,40,35,46,76,47,42,108,64,90,73,115,114,58,99,48,82,112,100,57,72,37,62];
var typedArray = new Uint8Array(numbers);
var hashTypedArray = dapCrypto.hash.fast(typedArray);
var hashString = dapCrypto.hash.chainHashToStringFast(hashTypedArray);
var hashTypedArray2 = dapCrypto.hash.chainStringToHashFast(hashString);
var equal = dapCrypto.hash.isEqualFast(hashTypedArray, hashTypedArray2);
var isBlank = dapCrypto.hash.isBlankHashFast(hashTypedArray);

console.log("Test hash functions");
console.log("Data:", typedArray);
console.log("Hash string :", hashString);
console.log("Hash data :", hashTypedArray);
console.log("Hash data2:", hashTypedArray2);
console.log("Equality:", equal);
console.log("isBlank:", isBlank);
