var addon = require('bindings')('libdap-crypto-nodejs');

// dap_enc_data_type_t from dap_enc_key.h
var dataTypes = {
    "raw"         : 0,
    "b64"         : 1,
    "urlsafe"     : 2,
    "b64_urlsafe" : 2,
};

function toDataTypeEnum(dataType) {
    if (Object.prototype.toString.call(dataType) !== "[object Number]") {
        dataType = dataType.toLowerCase();
        if (dataTypes.hasOwnProperty(dataType) == false) {
            throw new Error("Unknown data type");
        }
        return dataTypes[dataType];
    }
    return dataType;
}

function tryToGetArrayBuffer(input) {
    if (input != null /*yes "!=", ignore linter*/ && input.buffer instanceof ArrayBuffer) {
        return input.buffer;
    }

    return input;
}

exports.base64 = {
    encode: function base64_encode(input, dataType) {
        return addon.dap_enc_base64_encode(tryToGetArrayBuffer(input), toDataTypeEnum(dataType));
    },
    decode: function base64_decode(inputString, dataType) {
        return addon.dap_enc_base64_decode(inputString, toDataTypeEnum(dataType));
    },
};
exports.base58 = {
    encode: function base58_encode(input) {
        return addon.dap_enc_base58_encode(tryToGetArrayBuffer(input));
    },
    decode: function base58_decode(inputString) {
        return addon.dap_enc_base58_decode(inputString);
    },
};
exports.hash = {
    chainStringToHashFast: function(inputString) {
        return addon.dap_chain_str_to_hash_fast(inputString);
    },
    fast: function(input) {
        return addon.dap_hash_fast(tryToGetArrayBuffer(input));
    },
    chainHashToStringFast: function(input) {
        return addon.dap_chain_hash_fast_to_str(tryToGetArrayBuffer(input));
    },
    isEqualFast: function(hash_a, hash_b) {
        return addon.dap_hash_fast_compare(tryToGetArrayBuffer(hash_a), tryToGetArrayBuffer(hash_b));
    },
    isBlankHashFast: function(input) {
        return addon.dap_hash_fast_is_blank(tryToGetArrayBuffer(input));
    }
};
