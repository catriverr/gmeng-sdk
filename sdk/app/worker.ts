import chalk from 'chalk';
import builder, { TSGmeng } from './tsgmeng.js';

export function main(args: string[]): void {
    console.log(`${chalk.bold.red(`G${chalk.blue(`m${chalk.green(`e`)}n`)}g`)} ${chalk.bold.yellow(`DevKit`)} version 1.0`);
    console.log(`launching...`);
    if (args.includes(`-b`) || args.includes(`--builder`)) builder.mainMenu(args);
    // unpack & load
    else { 
        let buf = TSGmeng.loadMap(`./tests/world.gm`);
        buf.Events.player_mv((pl_old, pl_new) => {
            if (process.argv.includes(`-ui`)) { // simple ui elements
                process.stdout.cursorTo(1, 1); 
                process.stdout.write(` `.repeat(`[${pl_old.dX}, ${pl_old.dY}]`.length));
                process.stdout.cursorTo(1, 1); 
                process.stdout.write(`[${pl_new.dX}, ${pl_new.dY}]`); 
            }
        });
        buf.launchGame(); 
    };
};

main(process.argv.slice(2));
