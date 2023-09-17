import chalk from "chalk";
import tui from "../../utils/tui.js";

export default {
    id: 4, name: `admin`,
    async run(intf: tui.NamespaceController) {
        tui.typewriter(`welcome to the administrator page.`); await tui.sleep(900);
        tui.show_input_screen(chalk.reset.underline.green(`enter auth_key`), chalk.reset.dim(`[del] to clear entry | [shift+h] to hide/show input`), true, `!mkadmin MYBUTTON`, tui.texts[`authkey_info`]).then((data) => { console.clear(); console.log(`${data} is correct, launching admincpl.`); process.exit(0); }).catch(r => {
            if (r == `124534`) { return intf.show_screen(124534); };
            console.clear(); console.log(chalk.red`too bad.`)
        });
    }
};
