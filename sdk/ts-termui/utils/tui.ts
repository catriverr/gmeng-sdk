import chalk, { ChalkInstance } from "chalk";
import rl, { createInterface } from "readline";
import { readdirSync } from "fs";
import robot from 'robotjs';
import { TSGmeng, __gm_err65536_overlay_id_unimpl_e__, __gm_err_unimpl_e__, __vgm_err6556_contl__, absolute } from "../../app/tsgmeng.js";
export async function cli_find_pages(path: string, model: any): Promise<void> {
    return new Promise(async (resolve, reject) => {
        let files = readdirSync(path).filter(i=>i.endsWith(`.ts`))
        files.forEach((file,i)=>{
            import(`../` + path + `/` + file).then(f => { model.pages.set(f.default.id, f.default); });
        });
        setTimeout(resolve, files.length*70, void 0);
    });
};

export const upk = `\u001b[A\r`;
export const block = chalk.bold.bgWhite(` `);


export namespace anim {

    export class prgbar_utils {
        public add_prog(prg: number): void {}; public current_prog: number; public callback: Function; 
        public change_text(text: string): void {};
        public onfinish<Func extends (progress: number) => void>(callback: Func): void { this.callback = callback; };
        public end() { this.callback(this.current_prog); };
    };

    export const hosts_load = [
        `${upk}${chalk.bold.dim(`hosts`)} ${block}   `,
        `${upk}${chalk.bold.dim(`hosts`)}  ${block}`,
        `${upk}${chalk.bold.dim(`hosts`)}   ${block}`,
        `${upk}${chalk.bold.dim(`hosts`)}    ${block}`,
    ]; export const hosts_finish = `${upk}${chalk.bold.green(`hosts     `)}`;
    export const redir_load = [
        `${upk}${chalk.bold.dim(`redirects`)} ${block}   `,
        `${upk}${chalk.bold.dim(`redirects`)}  ${block}`,
        `${upk}${chalk.bold.dim(`redirects`)}   ${block}`,
        `${upk}${chalk.bold.dim(`redirects`)}    ${block}`,
    ]; export const redir_finish = `${upk}${upk}| ${chalk.bold.green(`redirects     `)}\n`;
    export const LOAD = [
        ...anim.hosts_load, ...anim.hosts_load, ...anim.hosts_load, hosts_finish, `\n`,
        ...anim.redir_load, ...anim.redir_load, ...anim.redir_load, redir_finish,
    ];

    export const start_load = [
        chalk.bold.white(`${upk}  ( ${chalk.bold.dim(`starting`)} )   `),
        chalk.bold.white(`${upk} (  ${chalk.bold.dim(`starting`)}  )`),
        chalk.bold.white(`${upk}(   ${chalk.bold.dim(`starting`)}   )`),
    ]; export const start_finish = [
        chalk.bold.white(`${upk}${chalk.bold.green(`initializing`)}        `),
        chalk.bold.white(`${upk}${chalk.bold.white(`initializing`)}        `),
        chalk.bold.white(`${upk}${chalk.bold.green(`initializing`)}        `),
        chalk.bold.white(`${upk}${chalk.bold.white(`initializing`)}        `),
    ];
    export const START = [
        ...start_load, ...start_load, ...start_load, ...start_load, ...start_finish, ...start_finish
    ];

    export const PRELUDE = [...LOAD, ...START];

    export let frm1_ldsc = (text: string) => chalk.bold.white(`${upk}${tui.center_align(`  [ ${chalk.bold.dim(text)} ]   `)}`);
    export let frm2_ldsc = (text: string) => chalk.bold.white(`${upk}${tui.center_align(` (  ${chalk.bold.dim(text)}  ) `)}`);
    export let frm3_ldsc = (text: string) => chalk.bold.white(`${upk}${tui.center_align(`{   ${chalk.bold.dim(text)}   }`)}`);
    export let framelist = [frm1_ldsc, frm2_ldsc, frm3_ldsc];

    export let loadscrn = (text: string, do_for: number, align: boolean) => { let a = align ? [frm1_ldsc(text), frm2_ldsc(text), frm3_ldsc(text)] : [
            chalk.bold.white(`${upk}  ( ${chalk.bold.dim(text)} )   `),
            chalk.bold.white(`${upk} (  ${chalk.bold.dim(text)}  )`),
            chalk.bold.white(`${upk}(   ${chalk.bold.dim(text)}   )`),]; let fin = []; for (let i=0;i<do_for;i++) fin.push(...a); return fin; };

    export async function progress_bar(text: string): Promise<anim.prgbar_utils> {
        let blocks = [tui.colortable.get(`tan`)`█`, tui.colortable.get(`green`)`█`];
        let progress = 0;
        let total_blocks = 50;
        let title = text;
        let utils = new class util extends anim.prgbar_utils {
            public current_prog = 0; 
            public add_prog(prg: number) { this.current_prog += prg; progress += prg; draw(); };
            public change_text(text: string) { title = text; };
        };
        console.log();
        function draw() {
            console.log(tui.upk, blocks[1].repeat(Math.floor(progress)) + blocks[0].repeat(Math.ceil(total_blocks-progress)) + ` ` + title + ` `.repeat(process.stdout.columns-total_blocks-title.length-2));
            if (total_blocks == progress) return utils.end();
        };
        draw();
        return utils;
    };
};

export namespace tui {
    //* up-key, goes up by 1 line in the console.
    export const upk = `\u001b[A\r`
    export const dwk = `\u001b[B\r`
    export const lfk = `\u001b[D`
    export const rgk = `\u001b[C`
    
    export type colors = `dark_red` | `red` | `black` | `cyan` | `swamp` | `yellow` | `pink` | `green` | `blue` | `tan` | `grayish` | `orange`;
    export const color_names: colors[] = [`dark_red`, `red`, /*`black`,*/ `cyan`, /*`swamp`,*/ `yellow`, `pink`, `green`, `blue`, `tan`, `grayish`, `orange`];
    export let button_storage: Array<Function> = [];
    export const colortable: Map<tui.colors, ChalkInstance> = new Map<colors, ChalkInstance>([
        [`red`, chalk.bold.rgb(244, 73, 52)],
        [`dark_red`, chalk.bold.rgb(204, 36, 29)],
        [`black`, chalk.bold.rgb(40, 40, 40)],
        [`cyan`, chalk.bold.rgb(134, 180, 117)],
        [`swamp`, chalk.bold.rgb(151, 150, 26)],
        [`yellow`, chalk.bold.rgb(249, 188, 47)],
        [`pink`, chalk.bold.rgb(211, 134, 155)],
        [`green`, chalk.bold.rgb(184, 187, 38)],
        [`blue`, chalk.bold.rgb(131, 165, 152)],
        [`tan`, chalk.bold.rgb(224, 209, 170)],
        [`grayish`, chalk.bold.rgb(167, 153, 132)],
        [`orange`, chalk.bold.rgb(246, 128, 25)]
    ]);

    export const colortable_bg: Map<tui.colors, ChalkInstance> = new Map<colors, ChalkInstance>([
        [`red`, chalk.bold.bgRgb(244, 73, 52)],
        [`dark_red`, chalk.bold.bgRgb(204, 36, 29)],
        [`black`, chalk.bold.bgRgb(40, 40, 40)],
        [`cyan`, chalk.bold.bgRgb(134, 180, 117)],
        [`swamp`, chalk.bold.bgRgb(151, 150, 26)],
        [`yellow`, chalk.bold.bgRgb(249, 188, 47)],
        [`pink`, chalk.bold.bgRgb(211, 134, 155)],
        [`green`, chalk.bold.bgRgb(184, 187, 38)],
        [`blue`, chalk.bold.bgRgb(131, 165, 152)],
        [`tan`, chalk.bold.bgRgb(224, 209, 170)],
        [`grayish`, chalk.bold.bgRgb(167, 153, 132)],
        [`orange`, chalk.bold.bgRgb(246, 128, 25)]
    ]);
    export function clr(text: string, clr: tui.colors): string {
        return tui.colortable.get(clr)(text);
    };
    export const undrln = (text: string) => chalk.underline(text);
    export const boldn = (text: string) => chalk.bold(text);
    export const show_cursor = () => process.stdout.write('\u001B[?25h\u001B[?0c');
    export const hide_cursor = () => process.stdout.write('\u001B[?25l\u001B[?1c');
    export function text_block(text: string) {
        let width = 0;
        for (let i = 0; i < text.split(`\n`).length; i++) {
            let val = text.split(`\n`)[i];
            if (val.length > width) width = val.length;
        };
        console.log(tui.clr(TSGmeng.c_outer_unit.repeat(width+4), `orange`));
        text.split(`\n`).forEach(vl => { console.log(tui.clr(TSGmeng.c_unit, `orange`) + ` ` + (vl) + ` ` + tui.clr(TSGmeng.c_unit, `orange`)); });
        console.log(tui.clr(TSGmeng.c_outer_unit_floor.repeat(width+4), `orange`));
    };
    class wcontrollers {
        public display = new class __wcontrollers_inital_variable__ { 
            public width: number = 5; public height: number = 5;
            public big_border: boolean = false; public pos: { x: number, y: number } = { x: 0, y : 0 }
            public lines: Array<Array<string>> = [];
        };
        constructor(width: number, height: number, pos: { x: number, y: number }, big_border = true) { 
            this.display.pos = pos;
            this.display.big_border = big_border;
            this.display.width = width; this.display.height = height;
        };
        public wchar_at(pos: { x: number, y: number }, s: string) {
            if (remove_colorcodes(s).length > 1) return;
            if (this.display.lines.length <= pos.y) return;
            if (this.display.lines[pos.y].length <= pos.x) return;
            lines[pos.y][pos.x] = s;
        };
        public wdraw() {

        };
    };
    export function display_w(width: number, height: number, pos: { x: number, y: number }, bborder = true): wcontrollers {
        let controller = new wcontrollers(width, height, pos, bborder);
        return controller;
    };
    export function capture<Func extends (key: rl.Key, ctrl: { closed: boolean; close: () => void } ) => void>(keys: Array<string>, callback: Func) {
        process.stdin.setRawMode(true);
        process.stdin.setDefaultEncoding("utf-8");
        rl.emitKeypressEvents(process.stdin);
        let ofl = true;
        process.stdin.on(`keypress`, (ch: string, key: rl.Key) => { 
        if (!ofl) return;
            if (key.sequence == `\x03`) process.stdout.write('\x1b[?25h'), process.exit(0); 
            if (keys.includes(key?.name) || keys.includes("**")) callback(key, { closed: false, close() { ofl = false; } }); 
        });
    };
    export interface overlay_item {
        name: string; id: number; selected?: boolean;
    };
    export const display_delegates = {
        TOP_LEFT: "┌",
        TOP_RIGHT: "┐",
        BOTTOM_LEFT: "└",
        BOTTOM_RIGHT: "┘",
        TITLE_START: "┤",
        TITLE_END: "├",
        LINE: "─",
        SIDE: "│"
    };
    type delegate_assign_types = string | any;
    export class OverlayController<delegate_names extends string> {
        public callback_delegates: Map<string, Function> = new Map<string, Function>; public __overlay_id__: number = Math.floor(Math.random() * 1000000);
        public constructor(id?: number) {
            if (!this.__overlay_id__ && !id) throw new RangeError(__gm_err65536_overlay_id_unimpl_e__ ?? __gm_err_unimpl_e__)
            id ? this.__overlay_id__ = id : void 0;
            this.callback_delegates.set(`__overlay_destroy__`, () => { return void 0; });
            this.callback_delegates.set(`destroy`, this.callback_delegates.get(`__overlay_destroy__`));
        };
        public run_delegate(delegate: delegate_names, ...args: any[]) {
            this.callback_delegates.get(delegate)(...args);
        };
        public on(delegate: delegate_names, callback: Function) {
            this.callback_delegates.set(delegate, callback);
        };
        public assign_delegate = (...args: absolute<delegate_assign_types | never | unknown | (() => (void[] | any))>[]) => this.on(args[0], args[1]);
        public assign_macro(pointee: string, pointer: string) { this.callback_delegates.set(pointee, (...args) => { this.callback_delegates.get(pointer)(args); }); };
    };
    export function overlay(title: string, menu_items: Array<overlay_item>, pos?: {x : number, y : number}): absolute<OverlayController<
        "selection_made" | "selection_change" | "item_change" | "item_select" | "destroy">
                                                                                         > {
        let controller = new OverlayController();
        controller.assign_delegate(`selection_change`, (selection: number) => {
            let voidptr: absolute<void> = void 0;
            return voidptr;
        });
        /// all macros for selection_change
        controller.assign_macro(`item_change`,     `selection_change`);
        controller.assign_macro(`item_select`,     `selection_change`);
        controller.assign_macro(`selectionChange`, `selection_change`);
        controller.assign_macro(`SelectionChange`, `selection_change`);
        controller.assign_macro(`itemChange`,      `selection_change`);
        controller.assign_macro(`ItemChange`,      `selection_change`);

        let selected = 0;
        function __build_overlay__() {
            process.stdout.cursorTo(pos?.x ?? 0, pos?.y ?? 0);
            console.log(tui.clr(display_delegates.TOP_LEFT + display_delegates.TITLE_START + tui.clr(title, `orange`) + display_delegates.TITLE_END + display_delegates.LINE.repeat(37-tui.remove_colorcodes(title).length) + display_delegates.TOP_RIGHT, `cyan`));
            menu_items.forEach((item, indx) => {
                process.stdout.cursorTo(pos?.x ?? 0, (pos?.y ?? 0) + indx + 1);
                let text_t = ``;
                let spacing = 42 - Math.floor((41 - item.name.length - 8)/2);
                if (item.selected) {
                    selected = indx;
                    text_t = tui.clr(`->`, `orange`) + ` ` + tui.undrln(tui.clr(item.name, `yellow`)) + ` `.repeat(spacing) + `  `;
                } else {
                    text_t = tui.clr(`->`, `grayish`) + ` ` + item.name + ` `.repeat(spacing) + `  `;
                };
                while (tui.remove_colorcodes(text_t).length < 37) text_t += `  `;
                while (tui.remove_colorcodes(text_t).length > 37) text_t = text_t.slice(0, -1);
                console.log(tui.clr(display_delegates.SIDE + ` ${tui.clr(text_t, `tan`)} ` + display_delegates.SIDE, `cyan`));
            });
            process.stdout.cursorTo(pos?.x ?? 0);
            console.log(tui.clr(display_delegates.BOTTOM_LEFT + display_delegates.LINE.repeat(39) + display_delegates.BOTTOM_RIGHT, `cyan`));
        };
        __build_overlay__();
        tui.capture([`up`, `down`, `escape`, `return`], (key, ctrl) => {
            let WeAreDestroyingThisListenerDontExecute = false;
            switch (key.name) {
                case 'up':
                    menu_items[selected].selected = false;
                    if (selected-1 < 0) { menu_items[menu_items.length-1].selected = true; break; };
                    menu_items[selected-1].selected = true;
                    break;
                case 'down':
                    menu_items[selected].selected = false;
                    if (selected+1 > menu_items.length-1) { menu_items[0].selected = true; break; };
                    menu_items[selected+1].selected = true;
                    break;
                case 'return':
                    controller.run_delegate("selection_made", selected);
                case 'escape':
                    ctrl.close();
                    controller.run_delegate('destroy');
                    WeAreDestroyingThisListenerDontExecute = true;
                    break;
            };
            if (WeAreDestroyingThisListenerDontExecute) return;
            __build_overlay__();
            controller.run_delegate('selection_change', selected);
        });
        controller.assign_delegate(`selection_change`, (selection: number) => {
            void 0; // __asign__( ASSERT( __DO_NOTHING__ ) )
        });
        return controller;
    };
    export function show_input_menu(title: string, predefined_input?: string): Promise<string> {
        return new Promise<string>((resolve) => {
            let input = predefined_input ?? ``;
            process.stdin.setRawMode(true);
            process.stdin.setDefaultEncoding("utf-8");
            function drawmenu(val: boolean = false) {
                process.stdout.cursorTo(0, 3);
                console.log(tui.clr(`+`, `red`) + tui.clr(`-`.repeat(40), `orange`) + tui.clr(`+`, `red`));
                console.log(tui.clr(`|`, `red`) + tui.clr(` ${title}:` + ` `.repeat(40 - `${title}:`.length - 1), val ? `green` : `cyan`) + tui.clr(`|`, `red`));
                console.log(tui.clr(`|`, `red`) + tui.clr(`  >`, `green`), tui.clr(input + ` `.repeat(40 - input.length - 4), val ? `green` : `tan`) + tui.clr(`|`, `red`));
                console.log((tui.clr(`+`, `red`)) + chalk.overline(tui.clr(`-`.repeat(40), `orange`)) + (tui.clr(`+`, `red`)));
                process.stdout.write(`\x1b[A\x1b[A${tui.rgk.repeat(input.length+5)}`);
            };
            tui.capture(["**"], (key, ctrl) => {
                if (key.sequence == "\r") { ctrl.close(); drawmenu(true); return resolve(input); };
                if (key.sequence == "\x7F") { input = input.slice(0, -1); drawmenu(); };
                if (!key || (key?.name?.length > 1 && key?.sequence != ' ')) return drawmenu();
                input += key.sequence;
                drawmenu();
            });
            drawmenu();
        });
    };
    export function get_curXY() {
        const screenX = robot.getMousePos().x;
        const screenY = robot.getMousePos().y;
        const terminalWidthInPixels = robot.getScreenSize().width;
        const terminalHeightInPixels = robot.getScreenSize().height;
        const terminalColumns = process.stdout.columns;
        const terminalRows = process.stdout.rows;
        const column = Math.floor((screenX / terminalWidthInPixels) * terminalColumns);
        const row = Math.floor((screenY / terminalHeightInPixels) * terminalRows)-1;
        return { row, column };
    };
    export async function mouse_button<Position extends {x: number, y: number}>(pos: Position, title: string, color: tui.colors, highlight_color: tui.colors, callback: Function): Promise<Function> {
        return new Promise<Function>((resolve, reject) => {
            process.stdout.cursorTo(pos.x, pos.y);
            process.stdout.write(tui.colortable_bg.get(color)(title));
            let highlighted = false;
            let force_hi = true;
            let disabled = false;
            function click_highlight() { highlighted = true;  process.stdout.cursorTo(pos.x, pos.y); process.stdout.write(tui.colortable_bg.get(`orange`).underline(title));};
            function highlight() { highlighted = true;  process.stdout.cursorTo(pos.x, pos.y); process.stdout.write(tui.colortable_bg.get(highlight_color).underline(title));};
            function normalize() { force_hi = false; highlighted = false; process.stdout.cursorTo(pos.x, pos.y); process.stdout.write(tui.colortable_bg.get(color)(title)); };
            let this_thread_1 = setInterval(function __js1() {
                if (disabled) return;
                let cur_pos = tui.get_curXY();
                if (cur_pos.row == pos.y) {
                    for (let i = 0; i < tui.remove_colorcodes(title).length; i++) {
                        if (i+pos.x == cur_pos.column) highlight();
                    };
                } else highlighted = false;
                if (!highlighted && !force_hi) normalize();
            }, 75);
            let controller = (x: number, y: number, clearIt: boolean) => {
                if (disabled) return;
                if (!(x == pos.x && y == pos.y)) return;
                click_highlight();
                setTimeout(normalize, 160);
                if (clearIt) disabled = true, clearInterval(this_thread_1);
                callback();
            };
            tui.button_storage.push(controller);
            return resolve(controller);
        });
    };
    export function run_button(x: number, y: number, disableAfterClick: boolean): void {
        tui.button_storage.forEach(B_Controller => {
            B_Controller(x, y, disableAfterClick);
        });
    };
    export async function show_menu(MenuScreen: tui.MenuScreen, selected_button: number = 0): Promise<number> {
        // I know that this code seems unorganized, but it needs to be
        // this way in order to be similar to the text displayed in the screen.
        function writeMenu() {
            if (selected_button > MenuScreen.buttons.length-1) selected_button = (selected_button-MenuScreen.buttons.length);
            if (selected_button < 0) selected_button = MenuScreen.buttons.length + selected_button;
            MenuScreen.buttons.find(btn=>btn.id===selected_button).selected=true;
            console.clear();
            console.log(tui.make_line()); 
            console.log(tui.center_align(MenuScreen.title), `\n`);
            console.log(tui.create_buttons(MenuScreen.buttons), ``);
            console.log(tui.make_line());
        };
        
        return new Promise((resolve, reject) => {
            writeMenu();
            process.stdin.setRawMode(true); process.stdin.setDefaultEncoding('utf8');
            let __closed = false;
            process.stdin.on('data', (data: string) => {
                if (__closed) return;
                data = Buffer.from(data, 'utf-8').toString();
                MenuScreen.buttons.find(btn=>btn.id===selected_button).selected=false;
                switch (data) {
                    case `\x1B[C`: selected_button++; writeMenu(); break; // >
                    case `\x1B[D`: selected_button--; writeMenu(); break; // <
                    case `\x1B[B`: selected_button=selected_button+6; writeMenu(); break; // ^ 
                    case `\x1B[A`: selected_button=selected_button-6; writeMenu(); break;  // v
                    default: writeMenu(); break;
                    case `\r`: __closed = true; process.stdin.setRawMode(false); resolve(selected_button); break;
                    case `\x1B`: reject(selected_button); case `\x03`: process.exit(0); break;
                };
            });
        });
    };

    export async function show_input_screen(title: string, footer?: string, hide_input: boolean = true, checker?: string, lasttext?: string): Promise<string> {
        process.stdin.setRawMode(true); process.stdin.setDefaultEncoding('utf8');
        let input = ``;
        let _hide = hide_input;
        function writeMenu(WRONG: boolean = false) {
            if (WRONG) {
                console.clear();
                console.log(tui.make_line());
                console.log(center_align(title));
                console.log(center_align(`${chalk.bold.red(`${chalk.green(`[`)} ${(input + (input.length % 2 == 0 ? `` : ``))} ${chalk.green(`]`)}`)}`));
                console.log(center_align(footer ?? ``));
                console.log(tui.make_line());
                tui.usleep(2000);
            };
            console.clear();
            console.log(tui.make_line());
            console.log(center_align(title));
            console.log(center_align(`${chalk.bold.white(`${chalk.green(`[`)} ${((_hide ? `•`.repeat(input.length) : input) + (input.length == 0 ? `▃` : ``) + (input.length % 2 != 0 || input.length == 0 ? `▃` : ``))} ${chalk.green(`]`)}`)}`));
            console.log(center_align(footer ?? ``));
            console.log(tui.make_line());
            console.log(lasttext ?? ``);
        };

        writeMenu();
        let __closed = false;
        return new Promise((resolve, reject) => {
            process.stdin.on(`data`, (data: string) => {
                if (__closed) return;
                data = Buffer.from(data).toString();
                switch (data) {
                    case `\x7F`: input = input.slice(0, -1); break; case `H`: _hide = !_hide; break;
                    case `\x1B[3~`: input = ``; break; case `\x1B`: process.exit(0); break; case `\x03`: process.exit(0); break;
                    case `\r`: __closed = true; console.clear(); if (checker && input != checker) { writeMenu(true); return reject(input); break; } else { resolve(input); break; };
                    default: input += data; break;
                };
                writeMenu();
            });
        });
    };

    export async function typewriter(text: string, centered?: boolean, tm: number = 50): Promise<void> {
        return new Promise((resolve, reject) => {
            let chars = [...text];
            let txt = ``;
            if (centered) { for (let i=0;i<chars.length;i++) { let ch = chars[i]; txt += ch; process.stdout.write(`\r` + tui.center_align(txt + `${i % 2 == 0 || i == 0 ? `▃` : ``}`)); tui.usleep(tm); }; return resolve(); };
            for (let i=0;i<chars.length;i++) { let ch = chars[i]; txt += ch; process.stdout.write(`\r` + txt + `${i % 2 == 0 || i == 0 ? `▃` : ``}`); tui.usleep(tm); }; return resolve();
        });
    };

    export async function sleep(ms: number): Promise<void> { return new Promise((resolve, reject) => { setTimeout(resolve, ms); }); };
    export async function usleep(ms: number) { let tm = Date.now() + ms; while (Date.now() != tm); };
    export async function await_keypress(): Promise<void> { process.stdin.setRawMode(true); process.stdin.setDefaultEncoding('utf8'); return new Promise<void>((resolve, reject) => { process.stdin.once(`data`, resolve); }); };
    
    export let hl_column = (ln: string, col: number) => `${ln.slice(0, col)}${tui.clr(chalk.bgRgb(224, 209, 170)(ln[col] ?? ` `), `black`)}${ln.slice(col+1, ln.length)}`;
    export let lines = (length: number, lines: string[]) => { let fin: string = ``; for (let i =0; i < length; i++) { if (process.stdout.columns < 181) return; let lntext = `| ${chalk.bold.red.dim(`${i.toString()[i.toString().length-1]}`)} ` + (lines[0].split(`\n`)[i]?.startsWith(`%pos=center%`) ? ` `.repeat((82-remove_colorcodes(lines[0].replaceAll(`%pos=center%`,``).split(`\n`)[i]).length)/2) : ``) + (lines[0].split(`\n`)[i]?.replace(`%pos=center%`, ``) ?? ``) + ` `.repeat(Math.floor(process.stdout.columns/2)-3-(1)-((lines[0].split(`\n`)[i]?.startsWith(`%pos=center%`) ? ` `.repeat((82-remove_colorcodes(lines[0].replaceAll(`%pos=center%`,``).split(`\n`)[i]).length)/2) : ``).length)-remove_colorcodes(lines[0].replaceAll(`%pos=center%`, ``).split(`\n`)[i] ?? ``).length) + ` | ` + chalk.bold.red.dim(`${i.toString()[i.toString().length-1]}`) + ` ` + (lines[1].split(`\n`)[i]?.startsWith(`%pos=center%`) ? ` `.repeat(Math.ceil((82-remove_colorcodes(lines[1].replaceAll(`%pos=center%`, ``).split(`\n`)[i]).length)/2)) : ``) + (lines[1].split(`\n`)[i]?.replace(`%pos=center%`, ``) ?? ``) + ` `.repeat(Math.floor(process.stdout.columns/2)-5-(1)-((lines[1].split(`\n`)[i]?.startsWith(`%pos=center%`) ? ` `.repeat((82-remove_colorcodes(lines[1].replaceAll(`%pos=center%`,``).split(`\n`)[i]).length)) : ``).length/2)-remove_colorcodes(lines[1].split(`\n`)[i]?.replace(`%pos=center%`, ``) ?? ``).length) + ` |`; fin += lntext + `\n`; }; return fin; };
    export let make_text_box = (text1: string, text2: string): string => `${tui.make_box_lid()}\n${tui.lines(tui.find_newlinebig(text1, text2), [text1, text2])}${tui.make_box_lid()}`;
    export let make_box_lid = () => `+${`-`.repeat(Math.floor((process.stdout.columns)/2))}+${`-`.repeat((process.stdout.columns-4)/2)}+`;
    export let find_newlinebig = (text1: string, text2: string): number => text1.split(`\n`).length > text2.split(`\n`).length ? text1.split(`\n`).length : text2.split(`\n`).length;    
    export let create_buttons = (buttons: tui.MenuButton[]) => { let bi = 0; return `\t${buttons.sort((a, b) => a.id-b.id).map(button => { bi++; return `\t${chalk.bold.blue(`[${button.selected ? chalk.underline.blue(button.title) : chalk.white(button.title)}]`)}${bi % 6 == 0 ? `\n` : ``}` }).join('\t')}`; };
    export let make_line = (length: number = process.stdout.columns, color: tui.colors = `blue`) => chalk.strikethrough(tui.clr(` `.repeat(length), color));
    export let center_align = (text: string) => ' '.repeat(Math.ceil((process.stdout.columns -1 - remove_colorcodes(text).length) / 2) > 0 ? Math.ceil((process.stdout.columns -1 - remove_colorcodes(text).length) / 2) : 1) + (text) + ' '.repeat(Math.ceil((process.stdout.columns - remove_colorcodes(text).length) / 2) > 0 ? Math.floor((process.stdout.columns - remove_colorcodes(text).length) / 2) : 1);
    export let remove_colorcodes = (text: string) => text.replace(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, '');
    export interface MenuScreen { title: string; description?: string; buttons: tui.MenuButton[] };
    export class ConsolePage { public id: number; public name: string; public async run(controller: tui.NamespaceController): Promise<number> { return 0; }; };
    export class MenuButton { public title: string; public id: number; public selected?: boolean = false; };
    export class NamespaceController { public pages: Map<number, ConsolePage> = new Map; public show_screen(id: number) { this.pages[id].run(this); }; };

    export namespace codes {
        export let getinput = chalk.bold.green(`dnscli::termui<dnsli::StdinWorker, dnscli::StdoutWorker> handle;\nauto txtbox = dnscli::get_object(handle.mkobject<dnscli::termui::textbox>());\ntxtbox.get_input().send_to(dnscli::worker<dnscli::auth>);`);
        export let authorize = chalk.bold.green(`dnscli::worker<dnscli::server, dnscli::auth> *app;\napp.mkrt<dnscli::auth>(bool test(app->key __k) { return app.test(__k, app->env); });\napp.listen_from("https://dns.mybutton.dev/worker").port(443).commence();`);
    };

    export namespace texts {
        export let authkey_info: string = 
        chalk.bold.white(`${tui.center_align(`DNSCLI ${chalk.magenta(`AUTHSYS_ADMIN_SERVER`)}`)}
${tui.center_align(`Welcome to the DNSCLI Admin page. The following code is going to authorize you to the server, or fail the system if you don't have access.`)}`)
        +chalk.bold.white(tui.make_text_box(tui.codes.getinput, tui.codes.authorize));
    };
};

export default tui;