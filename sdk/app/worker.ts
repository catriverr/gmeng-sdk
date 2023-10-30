import chalk from 'chalk';
import builder, { TSGmeng } from './tsgmeng.js';
import tui from '../ts-termui/utils/tui.js';
import { readdirSync, statSync } from 'fs';
import { Dirs } from '../../plib/src/index.js';

export function main(args: string[]): void {
    console.log(`${chalk.bold.red(`G${chalk.blue(`m${chalk.green(`e`)}n`)}g`)} ${chalk.bold.yellow(`DevKit`)} version 3.0`);
    if (args.includes(`-b`) || args.includes(`--builder`)) builder.mainMenu(args);
    // game launcher
    else { 
        console.log(tui.clr(`>>`, `pink`), tui.clr(`select a map to load (enter id):`, `tan`));
        let selected_map: string = './envs/';
        let files = readdirSync(`./envs`).filter(file => file.endsWith(`.gm`));
        files.forEach((f, i) => { 
            let lmf = Dirs.DirHandler.CreateDirectoryObject(`./envs/` + f);
            let wdata = TSGmeng.wData(lmf.readFile(`world.dat`), lmf.readFile(`player.dat`));
            console.log(tui.clr(`${i+1}.`, `pink`), tui.clr(f, `orange`), tui.clr(`-`, `yellow`), get_fsize(`./envs/${f}`, true) + `kb`,tui.clr(`-`, `yellow`), tui.clr(`${wdata._h}x${wdata._w}`, `green`), tui.clr(`-`, `yellow`), tui.clr(wdata.description, `blue`)); 
        });
        process.stdin.setRawMode(false);
        process.stdin.setEncoding('utf8');
        process.stdin.once(`data`, (data) => {
            if (Buffer.from(data).toString() == `\x03`) return process.exit(0);
            let d = parseInt(Buffer.from(data).toString());
            if ( d < 1 || d > files.length) return console.log(tui.clr(`invalid map file: ${files[d-1]}`, `dark_red`));
            selected_map += files[d-1]; game(selected_map);
        });
        
    };
};

export function game(selected_map: string) {
    let buf = TSGmeng.loadMap(`${selected_map}`);
    buf.Events.player_mv((pl_old, pl_new) => {
        if (process.argv.includes(`-ui`)) { // simple ui elements
            process.stdout.cursorTo(1, 1); 
            process.stdout.write(` `.repeat(`[${pl_old.dX}, ${pl_old.dY}]`.length));
            process.stdout.cursorTo(1, 1); 
            process.stdout.write(`[${pl_new.dX}, ${pl_new.dY}]`); 
        }
    });
    buf.Events.cmdget((cmdname) => {
        if (process.argv.includes(`-d`)) { buf.plog.write(cmdname + ` : command running on gm0/manager (TSGmeng & Gmeng processes | wfile: ${buf.env}), wmap: ${buf.env}`); };
    });
    buf.Events.modifier_change((name, old_value, new_value) => {
        if (process.argv.includes(`-d`)) buf.plog.write(`modifier ${name} was changed from ${old_value} to ${new_value}\n[ts-gm0:logger] these logs are made to test the C_PlugEvent capture system that allows for plugins to listen for events.`);
    });
    buf.launchGame(); 
};

export function get_fsize(filename: string, kb: boolean = false): number {
    var stats = statSync(filename);
    var fileSizeInBytes = stats.size / (kb ? 1024 : 1);
    return Math.floor(fileSizeInBytes);
}

main(process.argv.slice(2));
