var addon = require('bindings')('cellframe-sdk-nodejs');

if (exports.chain == null)
{
    exports.chain = {};
}

if (exports.chain.net == null)
{
    exports.chain.net = {};
}

exports.chain.net.cli = {
    init: function chain_net_init(config) {
        return addon.dap_chain_node_cli_init(config);
    },
    deinit: function chain_net_deinit() {
        return addon.dap_chain_node_cli_delete();
    },
    addCmd: function chain_net_add_cmd(name, func, context, doc, doc_ex) {
        return addon.dap_chain_node_cli_cmd_item_create(name, func, context, doc, doc_ex);
    },
};
