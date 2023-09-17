import chalk from 'chalk';
import builder, { TSGmeng } from './envs/builder.js'
import tui from '../ts-termui/utils/tui.js';

export function main(args: string[]): void {
    console.log(`${chalk.bold.red(`G${chalk.blue(`m${chalk.green(`e`)}n`)}g`)} ${chalk.bold.yellow(`DevKit`)} version 1.0`);
    console.log(`launching...`);
    tui.usleep(200);
    if (args.includes(`-b`) || args.includes(`--builder`)) builder.mainMenu(args);
    // unpack & load
    else TSGmeng.loadMap(`test.gm`).launchGame();
};

main(process.argv.slice(2));