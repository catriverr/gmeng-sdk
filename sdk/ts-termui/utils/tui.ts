import chalk, { ChalkInstance } from "chalk";
import rl from "readline";
import { readdirSync } from "fs";

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

    export type colors = `dark_red` | `red` | `black` | `cyan` | `swamp` | `yellow` | `pink` | `green` | `blue` | `tan` | `grayish` | `orange`;

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

    export function clr(text: string, clr: tui.colors): string {
        return tui.colortable.get(clr)(text);
    };
    export const undrln = (text: string) => chalk.underline(text);

    export function capture<Func extends (key: rl.Key) => void>(keys: Array<string>, callback: Func) {
        rl.emitKeypressEvents(process.stdin);
        process.stdin.on(`keypress`, (ch: string, key: rl.Key) => { if (key.sequence == `\x03`) process.exit(0); if (keys.includes(key?.name)) callback(key); });
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
    export let make_line = (length: number = process.stdout.columns, color: string = `blue`) => chalk.bold[color].strikethrough(`${` `.repeat(length)}`);
    export let center_align = (text: string) => ' '.repeat(Math.floor((process.stdout.columns -1 - remove_colorcodes(text).length) / 2)) + chalk.bold.yellow(text) + ' '.repeat(Math.ceil((process.stdout.columns - remove_colorcodes(text).length) / 2));
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