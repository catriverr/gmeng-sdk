import chalk from "chalk";
import tui, { anim } from "../../utils/tui.js";
import http from "../../src/load.js";
import config from '../../utils/client_config.js';

export default {
    id: 99, name: `home`,
    async run(intf: tui.NamespaceController) {
        console.log(`hi2`);
        tui.show_menu({ 
        title: `Welcome to dnscli, ${chalk.magenta(process.env[`USER`])}`,
        buttons: [ 
                {id: 5, title: chalk.green`deploy site`},{id: 11, title: chalk.red(`remove site`)},{id: 2, title: chalk.white`create socket`},{id: 3, title: chalk.yellow`get info`},{id: 0, title: `control page`},
                {id: 4, title: chalk.magenta`admin tools`},{id: 6, title: `${chalk.bold.gray(`c0ming_soon`)}`},{id: 9, title: `${chalk.bold.gray(`coming_soon`)}`},{id: 10, title: `${chalk.bold.gray(`coming_soon`)}`},{id: 7, title: `${chalk.bold.gray(`coming_soon`)}`},{id: 8, title: `${chalk.bold.gray(`coming_soon`)}`}, {id: 1, title: chalk.cyan`settings`}, ]
        }, 0).then((function handle(btn: number): any {
            if (btn == 11) {  
                if (!config.WebServerData["info.domain_name"]) { return console.log(tui.center_align(chalk.red(`{ ${chalk.bold.white(`no webserver on behalf of this computer was found`)} }`))), tui.usleep(4000), intf.show_screen(99); };
                let cc = 0; console.log(); 
                let intv =setInterval(() => { console.log((anim.loadscrn(`${chalk.red(`removing site from`)} dns.mybutton.dev`, 3, true)[cc++])); if (cc==9) return clearInterval(intv), tui.usleep(2000), console.log(tui.upk + tui.center_align(chalk.bold.red(`{ ${chalk.bold.magenta(`your server has been`)} removed }`))), tui.usleep(5000), intf.show_screen(99);  }, 700); 
                return fetch(http.server + `/remove-dns`, { method: 'POST', body: JSON.stringify({cached_key: ``})})
            };
            intf.show_screen(btn);
        }));
    }
};
