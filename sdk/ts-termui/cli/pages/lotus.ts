import chalk from "chalk";
import tui from "../../utils/tui.js";

export default {
    id: 124534, name: `lotus`,
    run(intf: tui.NamespaceController) {
        const lotus_info = `Information on this community is listed here. Follow the instructions.\n%pos=center%${chalk.bold.green(`[how to attend meetings]`)}\n-> visit dns: 124534.lotus.org via dnscli\n-> enter password\n-> select option [meetings]\n%pos=center%${chalk.bold.green(`[why we exist]`)}\nwe believe that smart people should be a higher class in society.\nsmart people should be in charge of everything in the world.\nknowledge is power. the uneducated & reckless are to pity.`;
        
        console.clear();
        tui.typewriter(`you seem to know where you are. `);
        tui.usleep(2000);
        console.clear();
        console.log(tui.make_line(process.stdout.columns, `cyan`));
        tui.typewriter(`{ ` + chalk.white(`lotus secretive society.`) + ` }`, true); 
        console.log(tui.make_line(process.stdout.columns, `cyan`));
        console.log(tui.center_align(chalk.bold.green(`[ ${chalk.magenta(`welcome to lotus. we do not exist, we are an underground community.`)} ]`)));
        console.log(tui.make_text_box(`%pos=center%` + chalk.underline.bold.blue(`News`) + ``, `%pos=center%` + chalk.underline.bold.yellow(`Information`) + `\n${lotus_info}`));
        process.exit(0);
    }
}