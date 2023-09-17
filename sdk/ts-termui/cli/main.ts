import chalk from "chalk";
import tui, { anim, cli_find_pages } from "../utils/tui.js";
import { readdirSync } from "fs";

export namespace ClientInterface {
    export let pages: Map<number, tui.ConsolePage> = new Map;
    export function show_screen(id: number) { ClientInterface.pages.get(id)?.run(ClientInterface); };
};

cli_find_pages(`./cli/pages`, ClientInterface)
    .then(() => { ClientInterface.show_screen(99); });



export default ClientInterface;