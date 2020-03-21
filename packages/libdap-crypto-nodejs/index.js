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

exports.base64 = {
    encode: function base64_encode(input, dataType) {
        return addon.dap_enc_base64_encode(input, toDataTypeEnum(dataType));
    },
    decode: function base64_decode(input, dataType) {
        return addon.dap_enc_base64_decode(input, toDataTypeEnum(dataType));
    },
};
