var addon = require('bindings')('cellframe-sdk-nodejs');

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

exports.crypto = {
    init: function enc_init() {
        return addon.dap_enc_init();
    },
    deinit: function enc_deinit() {
        return addon.dap_enc_deinit();
    },
};

exports.crypto.cert = {
    init: function cert_init() {
        return addon.dap_cert_init();
    },
    deinit: function cert_deinit() {
        return addon.dap_cert_deinit();
    },
    create: function dap_cert_create(name, filepath, keyType) {
        keyType = tryToConvertKeyType(keyType);
        return addon.dap_cert_generate(name, filepath, keyType);
    },
    extractPublicKeyAsCert: function extract_pkey_as_cert(cert_name, new_cert_name, ca_folder) {
        return addon.extract_pkey_as_cert(cert_name, new_cert_name, ca_folder);
    },
};

exports.base64 = {
    DataTypes: dataTypes,
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
    },
};

// TODO: find a better solution later
var origin_genKeyPublic = addon.Key.prototype.genKeyPublic;
addon.Key.prototype.genKeyPublic = function genKeyPublic(input) {
    return origin_genKeyPublic.call(this, tryToGetArrayBuffer(input));
};
var origin_deserializePublicKey = addon.Key.prototype.deserializePublicKey;
addon.Key.prototype.deserializePublicKey = function deserializePublicKey(input) {
    return origin_deserializePublicKey.call(this, tryToGetArrayBuffer(input));
};
var origin_deserializePrivateKey = addon.Key.prototype.deserializePrivateKey;
addon.Key.prototype.deserializePrivateKey = function deserializePrivateKey(input) {
    return origin_deserializePrivateKey.call(this, tryToGetArrayBuffer(input));
};

function tryToConvertKeyType(keyType) {
    if (Object.prototype.toString.call(keyType) === "[object String]") {
        keyType = keyType.toLowerCase();
        if (addon.KeyTypes.hasOwnProperty(keyType) == false) {
            throw new Error("Unknown key type");
        }
        return addon.KeyTypes[keyType];
    }
    return keyType;
}

exports.key = {
    KeyTypes: addon.KeyTypes,
    Key: addon.Key,
    tryToConvertKeyType: tryToConvertKeyType,
    create: function(keyType, kaxBuffer, seedBuffer, keySize) {
        keyType = tryToConvertKeyType(keyType);
        if (arguments.length == 1) {
            return new addon.Key(keyType);
        } else if (arguments.length == 4) {
            return new addon.Key(keyType, tryToGetArrayBuffer(kaxBuffer), tryToGetArrayBuffer(seedBuffer), keySize);
        } else {
            throw new Error("1 or 4 arguments are expected");
        }
    },
    init: function key_init() {
        return addon.dap_enc_key_init();
    },
    deinit: function key_deinit() {
        return addon.dap_enc_key_deinit();
    },
};
