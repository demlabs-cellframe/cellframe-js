var addon = require('bindings')('cellframe-sdk-nodejs');

exports.chain = {
    init: function chain_init() {
        return addon.dap_chain_init();
    },
    deinit: function chain_deinit() {
        return addon.dap_chain_deinit();
    },
};

exports.chain.gdb = {
    init: function chain_gdb_init() {
        return addon.dap_chain_gdb_init();
    },
    deinit: function chain_gdb_deinit() {
        // There is no deinit function
    },
};

exports.chain.globalDB = {
    init: function chain_global_db_init(config) {
        return addon.dap_chain_global_db_init(config);
    },
    deinit: function chain_global_db_deinit() {
        return addon.dap_chain_global_db_deinit();
    },
};

exports.chain.wallet = {
    init: function chain_wallet_init() {
        return addon.dap_chain_wallet_init();
    },
    deinit: function chain_wallet_deinit() {
        return addon.dap_chain_wallet_deinit();
    },
    create: function chain_wallet_create(name, folder_path, signature_type_name) {
        return addon.dap_chain_wallet_create(name, folder_path, signature_type_name);
    },
};

exports.chain.dag = {
    init: function chain_dag_init() {
        return addon.dap_chain_cs_dag_init();
    },
    deinit: function chain_dag_deinit() {
        return addon.dap_chain_cs_dag_deinit();
    },
};

exports.chain.dag.pos = {
    init: function chain_dag_pos_init() {
        return addon.dap_chain_cs_dag_pos_init();
    },
    deinit: function chain_dag_pos_deinit() {
        return addon.dap_chain_cs_dag_pos_deinit();
    },
};

exports.chain.dag.poa = {
    init: function chain_dag_poa_init() {
        return addon.dap_chain_cs_dag_poa_init();
    },
    deinit: function chain_dag_poa_deinit() {
        return addon.dap_chain_cs_dag_poa_deinit();
    },
};

exports.chain.net = {
    init: function chain_net_init() {
        return addon.dap_chain_net_init();
    },
    deinit: function chain_net_deinit() {
        return addon.dap_chain_net_deinit();
    },
};

exports.chain.net.service = {
    init: function chain_net_srv_init(config) {
        return addon.dap_chain_net_srv_init(config);
    },
    deinit: function chain_net_srv_deinit() {
        return addon.dap_chain_net_srv_deinit();
    },
};

exports.chain.net.cli = {
    init: function chain_net_cli_init(config) {
        return addon.dap_chain_node_cli_init(config);
    },
    deinit: function chain_net_cli_deinit() {
        return addon.dap_chain_node_cli_delete();
    },
    addCmd: function chain_net_cli_add_cmd(name, func, context, doc, doc_ex) {
        return addon.dap_chain_node_cli_cmd_item_create(name, func, context, doc, doc_ex);
    },
};
