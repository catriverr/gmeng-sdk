import chalk from "chalk";
import builder from "../sdk/app/tsgmeng.js";
import tui from "../sdk/ts-termui/utils/tui.js";

async function main(value: number) {
    let choice = value ?? 0;
    if (!value) {
        choice = await tui.show_menu({
            title: "select editor",
            description: chalk.bold.red(`G${chalk.bold.green(`m`)}${chalk.bold.blue(`e`)}${chalk.bold.yellow(`n`)}g ${chalk.bold.magenta(`SDK`)}`),
            buttons: [
                {id: 0, title: "gm4.0 texture editor", selected: false},
                {id: 1, title: "gm4.0 gmlevel editor", selected: false},
                {id: 2, title: "gm1.1 world editor", selected: false}
            ]
        });
    };
    switch (choice) {
        case 0: builder.texture4_0(); break;
        case 1: builder.framework4_0(process.argv.slice(2)); break;
        case 2: builder.framework1_1(process.argv.slice(2)); break;
    };
};

let __argument__ = (parseInt((process.argv[2])?.substring("-e=".length)));

main(process.argv[2] != null ? __argument__ : null);
