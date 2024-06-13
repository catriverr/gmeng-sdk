import readline, {Key} from 'readline';
import { tui } from '../../../sdk/ts-termui/utils/tui.js';
import chalk from 'chalk';
async function main() {
    let decoration = 'o', leaves = '#', star = '$', snow = '*';
    const random_decoration = () => tui.colortable.get(tui.color_names[(Math.floor(Math.random() * tui.color_names.length))])(decoration);
    let leaves_color: tui.colors = `swamp`, star_color: tui.colors[] = [`yellow`, `orange`];
    let height = 10, max_width = 19;
    const tree_log = "mWm";
    function __add_snow__() {
        let random_x = Math.floor(Math.random() * process.stdout.columns);
        let random_y = Math.floor(Math.random() * process.stdout.rows) + 5;
        while(random_y >= Math.floor(process.stdout.rows / 2) + height) random_y--;
        process.stdout.cursorTo(random_x, random_y);
        process.stdout.write(snow);
        setTimeout(() => { process.stdout.cursorTo(random_x, random_y); process.stdout.write(` `); }, 2000);
    };
    function __draw_tree__() {
        process.stdout.cursorTo(0, Math.floor(process.stdout.rows/2) - Math.floor(height) + 4);
        let leaves_array = [];
        for (let i = 0, j = 1; i < height; i++) {
            let current_line = [];
            if (j == 1) { current_line.push(tui.clr(star, star_color[Math.floor(Math.random() * star_color.length)])); j += 2; leaves_array.push(current_line.join(``)); continue; };
            for (let k = 0; k < j; k++) current_line.push((Math.floor(Math.random() * 4) == 1) ? random_decoration() : tui.clr(leaves, leaves_color));
            leaves_array.push(current_line.join(``));
            j += 2; // total characters
            while (j > max_width) j -= 1;
        };
        leaves_array.forEach(ln => {
            console.log(tui.center_align(ln));
        });
        console.log(chalk.bold.rgb(141, 90, 0)(tui.center_align(tree_log)));
        console.log(chalk.bold.rgb(141, 90, 0)(tui.center_align(tree_log)));
        console.log(tui.center_align(tui.clr(`press ${tui.clr(`q`, `orange`)} to quit`, `dark_red`)));
        console.log(tui.center_align(tui.clr(`press ${tui.clr(`m`, `orange`)} to modify tree`, `dark_red`)));
        for (let i = 0; i < 20; i++) __add_snow__();        
    };
    process.stdin.setRawMode(true); process.stdin.setDefaultEncoding(`utf-8`);
    let quit: boolean = false, exitvalue: boolean = false;
    let editing = false;
    process.stdin.on(`keypress`, (key: string, e: Key) => {
        if (key == `\x03`) process.exit(0);
        console.log(e.name + (` `.repeat(process.stdout.columns - e.name?.length)));
        if (editing) return;        
        if (e.name == `q`) exitvalue = true;
        if (e.name == `m`) {
            const modifiers: {name: string, value: any}[] = [
                {name: `tree height`, value: height},
                {name: `tree width`, value: max_width},
                {name: `tree color`, value: leaves_color}
            ];
            let selected = 0;
            function __modify_menu__ (do_list?: {name: string, value: any}[]) {
                if (selected < 0 || selected > modifiers.length-1) selected = 0;
                process.stdout.cursorTo(0, 0);
                console.log(tui.make_line(30, `red`));
                modifiers.forEach((val, indx) => {
                    console.log((indx == selected ? tui.clr(`->`, `orange`) : tui.clr(`--`, `grayish`)), ( indx == selected ? tui.clr(`${val.name}${tui.clr(`:`, `tan`)} ${tui.clr(val.value.toString(), `red`)}`, `green`) : tui.clr(`${val.name}: ${val.value.toString()}`, `grayish`) ));
                });
                console.log(tui.make_line(30, `dark_red`));
            };
            function __apply_modifiers__() {
                modifiers.forEach((mf, indx) => {
                    switch (indx) {
                        case 0:
                            height = mf.value;
                            break;
                        case 1:
                            max_width = mf.value;
                            break;
                        case 2:
                            leaves_color = mf.value;
                            break;
                    };
                });
            };
            function __edit_menu__(id: number = selected) {
                console.clear();
                let input = ``;
                let current_modifier = modifiers[id] ?? modifiers[selected];
                function __draw_line__(err: boolean = false): string {
                    return err ? `${tui.clr(`       invalid input.${` `.repeat(process.stdout.columns - 21)}`, `red`)}` : `${tui.clr(`->`, `green`)} ${tui.clr(current_modifier.name, `orange`)}${tui.clr(`:`, `tan`)} ${tui.clr(input, `pink`)}`;
                };
                function __draw_screen__(err: boolean = false) {
                    process.stdout.cursorTo(0, 0);
                    console.log(tui.make_line(30, `dark_red`));
                    console.log(__draw_line__(err));
                    console.log(tui.make_line(30, `dark_red`));
                };
                tui.capture([`**`], async (key, ctrl) => {
                    switch (key.name) {
                        case 'backspace':
                            input = input.slice(0, -1);
                            console.clear();
                            break;
                        case 'escape':
                            ctrl.close();
                            editing = false;
                            console.clear();
                            return __modify_menu__();
                            break;
                        case 'return':
                            if (typeof current_modifier.value == 'number') {
                                if (isNaN(parseInt(input))) { __draw_screen__(true); await tui.usleep(2000); }
                                else modifiers[id].value = parseInt(input);
                            } else modifiers[id].value = input;
                            ctrl.close();
                            editing = false;
                            console.clear();
                            return __modify_menu__();
                            break;
                        default:
                            if ((key.name?.length > 1 || key.name?.length < 1) && (![`_`, ` `].includes(key.sequence))) { input += chalk.bold.bgBlack.red(`#`); break; };
                            input += key.sequence;
                            break;
                    };
                    __draw_screen__();
                });
                __draw_screen__();
            };
            __modify_menu__();
            tui.capture([`up`, `down`, `return`, `tab`, `escape`], (key, ctrl) => {
                if (editing) return;
                switch (key.name) {
                    case 'up': 
                        selected--;
                        break;
                    case 'down': case 'tab':
                        selected++;
                        break;
                    case 'return':
                        editing = true;
                        __edit_menu__(selected);
                        break;
                    case 'escape':
                        ctrl.close();
                        console.clear();
                        __apply_modifiers__();                        
                        selected = -1;
                        break;
                };
                if (selected == -1) return selected = 0;
                if (selected < 0) selected = modifiers.length-1;
                if (selected > modifiers.length-1) selected = 0;
                if (!editing && !ctrl.closed) __modify_menu__();
            });
        };
    });
    process.stdout.on(`resize`, () => { console.clear(); });
    process.title = `dec 31, new years day`;
    process.on(`exit`, () => { tui.show_cursor(); });
    readline.emitKeypressEvents(process.stdin);
    console.clear();
    tui.hide_cursor();
    setInterval(() => {
        let n1 = quit, n2 = exitvalue;
        if (n1) return; // hang        
        if (n2) tui.show_cursor(), process.exit(0); // exit
        __draw_tree__();
    }, 200);
};

main();
