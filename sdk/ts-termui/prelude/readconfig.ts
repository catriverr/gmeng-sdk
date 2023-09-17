import chalk from 'chalk';
import fs from 'fs';
import svlog from '../src/util/serverlog.js';

namespace cfg {
    export let domains: Map<string, any> = new Map<string, any>; export let hosts: Map<string, string> = new Map<string, string>; export let redirects: Map<string, string> = new Map<string, string>;
    export let port: number = 1;
    export async function readHosts(path: string): Promise<void> {
        let file = fs.readFileSync(path, { encoding: 'utf8' }).split('\n').filter(line => !line.startsWith('#') && line.length > 1 && line.replaceAll(` `, ``).length > 0);
        file.forEach((line, index) => {
            let spl = line.split(' '); let CURRENT_LIST = new Map();
            switch (spl[0].toLocaleLowerCase()) { 
                case `use`:
                    let filef = spl[1]; let type = spl[2].substring(1).slice(0, -1).toLocaleLowerCase();
                    if (type == `hosts`) return cfg.readHosts(filef);
                    else if (type == `redirects`) cfg.setRedirects(filef);
                    break;
                case `set`:
                    let varname = spl[1], value = JSON.parse(spl.slice(2).join(' ')); cfg[varname] = value;
                    break;
                case `def`:
                    let nm = spl[1], lname = spl[2].substring(1).slice(0, -1).toLocaleLowerCase();
                    let readlist = file.slice(index); let hosts = readlist.slice(1, -(readlist.length - readlist.findIndex(val => val == `ends`)));
                    hosts.forEach((v, i) => {
                        let __s1 = v.split(` `), __ip = __s1[0], __nm = __s1[1]; cfg.domains.set(__nm, {name: __nm, ip: __ip}), svlog.puts(`${chalk.green(nm)} ${chalk.magenta(lname)} - ${chalk.yellow(`domain`)}::${chalk.bold.green(`reg(${chalk.magenta(__ip)}) ${chalk.white(`->`)} ${chalk.cyan(__nm)}`)} ${chalk.bold(`LOADED`)}`);
                    });
                    break;
                default: break;
            };
        });
    };
    export async function setRedirects(file: string): Promise<void> {
        
    };
};

export default cfg; 