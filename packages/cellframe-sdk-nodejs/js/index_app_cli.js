var addon = require('bindings')('cellframe-sdk-nodejs');

if (exports.app == null)
{
    exports.app = {};
}

exports.app.cli = {
    main: function app_cli_main(app_name, socket_path, argv) {
        return addon.dap_app_cli_main(app_name, socket_path, argv);
    },
    Connection: addon.AppCLIConnection,
};
