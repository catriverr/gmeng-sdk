import chalk from "chalk";
import fs from "fs";
import tui from "../../utils/tui.js";
import rl from "readline";
import EventEmitter from "events";


let splice = function(txt: string, idx: number, rem: number, str: string) {
    return txt.slice(0, idx) + str + txt.slice(idx + Math.abs(rem));
};

export default {
    id: 7, name: `cedit`,
    async run(intf: tui.NamespaceController) {
        console.clear();
        const fname = await tui.show_input_screen(`file name`, `[del] to clear entry `, false);
        console.clear();
        let  f1: Array<string> = [``]; 
        let  f2: Array<string> = [``]; 
        const params1: { id: number, selected: boolean, new_file: boolean, name: string, row: number, column: number } = { id: 0, selected: true, new_file: false, name: fname, row: 0, column: 0 };
        const params2: { id: number, selected: boolean, new_file: boolean, name: string, row: number, column: number } = { id: 1, selected: false, new_file: true, name: chalk.bold.red(`[empty]`), row: 0, column: 0 };
        if (fs.existsSync(fname)) params1.new_file=false, f1 = fs.readFileSync(fname, 'utf8').split(`\n`);
        const files = [{params: params1, lines: f1}, {params: params2, lines: f2}];
        rl.emitKeypressEvents(process.stdin);
        process.stdin.setRawMode(true);
        function draw_lines(): void {
            let selected = files.find(f=>f.params.selected);
            if (selected.params.column > selected.lines[selected.params.row].length) { 
                if (selected.params.row+1 == selected.lines.length) {} else selected.params.column = 0, selected.params.row++; 
            };
            if (selected.params.column < 0) selected.params.column = 0, ((selected.params.row-1) > -1 ? selected.params.row-- : void 0);
            files[selected.params.id] = selected;
            let drawlines = [
                files[0].lines.map((ln, i) => i==files[0].params.row ? tui.hl_column(ln, files[0].params.column) : ln),
                files[1].lines.map((ln, i) => i==files[1].params.row ? tui.hl_column(ln, files[1].params.column) : ln),
            ];
            console.clear();
            console.log(tui.make_text_box(`%pos=center%` + tui.clr(tui.undrln(params1.selected ? params1.name : chalk.dim(params1.name)), `dark_red`) + `\n` + drawlines[0].join('\n'), `%pos=center%` + params2.name + `\n` + drawlines[1].join('\n')));
            return void 0;
        };
        draw_lines();
        process.stdin.on(`keypress`, (ch: string, key: {sequence: string, name: string, ctrl: boolean, meta: boolean, shift: boolean}) => {
        let selected = files.find(f=>f.params.selected);
            if (key != undefined  && ch != undefined && !key.ctrl && !key.meta && !key.shift && ![`return`, `up`, `backspace`, `left`, `right`, `down`].includes(key.name)) {
                selected.lines[selected.params.row] = splice(selected.lines[selected.params.row], selected.params.column, 0, ch);
                selected.params.column++;
            };
            if (key.sequence === '\x03') return process.exit(0);
            if (key.name == `return`) {
                let result = [...selected.lines.slice(0, selected.params.row), ``, ...selected.lines.slice(selected.params.row)];
                selected.lines = result; selected.params.row++;
                selected.params.column = 0;
            };
            if (key.name == `backspace` && selected.params.column != 0) selected.lines[selected.params.row] = selected.lines[selected.params.row].slice(0, selected.params.column-1) + selected.lines[selected.params.row].substring(selected.params.column), selected.params.column--;
            if (key.name == `left`) { selected.params.column--; }; if (key.name == `right`) { selected.params.column++; };
            if (key.name == `up` && selected.params.row-1 > -1) { (selected.lines[selected.params.row-1].length > selected.params.column ? void 0 : selected.params.column = 0); selected.params.row--; }; if (key.name == `down` && selected.params.row+1 < selected.lines.length) { (selected.params.column > selected.lines[selected.params.row+1].length ? selected.params.column = 0 : void 0); selected.params.row++; };
            draw_lines();
        });
    }
}