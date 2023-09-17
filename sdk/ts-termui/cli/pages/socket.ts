import chalk from "chalk";
import tui from "../../utils/tui.js";
import http from "../../src/load.js";

export default {
    id: 2, name: `create_socket`,
    async run(intf: tui.NamespaceController) {
        http.contact();
    }
};