import chalk from "chalk";
import tui from "../ts-termui/utils/tui.js";
import * as Plib from '../../plib/src/index.js';
import { BaseCfgTemplate, Directory } from "../../plib/lib/dirhandle.js";
import path, { dirname } from 'path';
import { fileURLToPath } from 'url';
import {existsSync, readFileSync, readdirSync, truncate, unlinkSync, writeFileSync } from "fs";
import { ChildProcess, spawn } from "child_process";
import { Key, emitKeypressEvents } from "readline";
import EventEmitter from "events";
import { Writable } from "stream";

export type absolute<typename_t> = typename_t;
const __dirname = dirname(fileURLToPath(import.meta.url));
const mac_gmeng_path = path.join(__dirname, `..`, `..`, `lib`, `out`) + `/gmeng.out`;
const win_gmeng_path = path.join(__dirname, `..`, `..`, `lib`, `out`) + `/gmeng.exe`;
export namespace builder {
    function lv_objname(v: "txtr" | "mdl"): string { return (v + Math.floor(Math.random()*100000).toString()); };
    /// string sanitization protocol for all 4.0 framework names
    function lv_sanitize(s: string) {
        return (s)
                .replaceAll(` `, `_`).replaceAll(`+`, `_`).replaceAll(`>`, `_`).replaceAll(`<`, `_`).replaceAll(`,`, `_`)
                .replaceAll(`[`, `_`).replaceAll(`]`, `_`).replaceAll(`{`, `_`).replaceAll(`}`, `_`).replaceAll(`:`, `_`)
                .replaceAll(`;`, `_`).replaceAll(`/`, `_`).replaceAll(`\\`, `_`).replaceAll(`)`, `_`).replaceAll(`(`, `_`)
                .replaceAll(`*`, `_`).replaceAll(`&`, `_`).replaceAll(`^`, `_`).replaceAll(`!`, `_`).replaceAll(`@`, `_`)
                .replaceAll(`$`, `_`).replaceAll(`#`, `_`).replaceAll(`?`, `_`).replaceAll(`'`, `_`).replaceAll(`"`, `_`);
    };

    /**
     * loads a texture from a .gt file
     */
    export function load_texture(fname: string): TSGmeng.fw4_texture {
        if (!existsSync(fname)) throw RangeError(fname + " does not exist | ltxtr(file) -> error");
        let f_indx = readFileSync(process.cwd() + "/" + fname, `utf-8`);
        let w_h = [];
        let name = ``;
        let collision = true;
        let v_units: Array<TSGmeng.Unit> = [];
        f_indx.split(`\n`).forEach((ln, i) => {
            if (ln.startsWith(`;`)) return;
            if (i == 0) {
                let values = ln.split(`,`);
                values.forEach((param,indx) => {
                    if (indx == 0) { name = param.substring(5); }; /// name=
                    if (indx == 1) { w_h[0]=parseInt(param.substring(6)); }; /// width=
                    if (indx == 2) { w_h[1]=parseInt(param.substring(7)); }; /// height=
                    if (indx == 3) { collision=(param.substring(10) == "true"); }; /// collision=
                }); return void 0;
            }
            else {
                let str = ln, params = ln.split(` `);
                if (str == "__gtransparent_unit__") return v_units.push({
                    color: 7, transparent: true
                });
                v_units.push({
                    color: parseInt(params[0]),
                    special: params[1] == "true",
                    special_clr: parseInt(params[2]),
                    special_c_unit: params[3]
                });
            };
        });
        let texture: TSGmeng.fw4_texture = {
            width: w_h[0], height: w_h[1],
            collidable: collision, name: name,
            units: v_units
        };
        return texture;
    };
    export async function texture4_0(name: string = null): Promise<TSGmeng.fw4_texture> {
        process.stdout.write('\x1b[?25l'); // hide cursor
        let cubic_render = false;
        console.clear();
        /// v! important - transparent units
        let vt_name = name ?? lv_objname("txtr");
        let texture: TSGmeng.fw4_texture = {
            height: 0, width: 0, name: vt_name, units: [], collidable: true,
        };
        let v_filename = ``;
        if (!name) {
            texture.name = lv_sanitize(await tui.show_input_menu("texture name"));
            console.clear();
        };
        let v_continue = true;
        while (texture.name.endsWith(`.gt`)) { texture.name = texture.name.slice(0, -3); /* remove .gt suffix */ };
        v_filename = texture.name + `.gt`;
        if (existsSync(process.cwd() + "/" + v_filename)) {
            texture = builder.load_texture(v_filename); v_continue = false;
        };
        process.stdin.setRawMode(true);
        if (v_continue) {
            let w_h = await tui.show_input_menu("texture size (HeightxWidth | def 0x0)");
            texture.height = parseInt(w_h.split("x")[0]), texture.width = parseInt(w_h.split("x")[1]);
            for (let i = 0; i < texture.height * texture.width; i++) {
                texture.units.push({ collidable: texture.collidable, color: 0, special: false, special_c_unit: `X`, special_clr: 1 });
            };
            console.clear();
            texture.collidable = (await tui.show_input_menu("collision (true/false | def: true)")) == "true";
            console.clear();
        };
        console.log(tui.clr(`gmeng::texture v_txtr -> const_l create_static() ; Data :`, `orange`));
        console.log(texture);
        console.clear(); console.log(tui.clr(`gmeng::texture v_txtr ; Preview : `, `orange`));
        function __gtexture_editor() {
        let v2d_curpos = { x: 0, y: 0 };
        let cur_unit: TSGmeng.Unit = {
            color: 1, transparent: false, special: false, special_c_unit: `X`, special_clr: 0
        };
        let prev_curpos = v2d_curpos;
        let v1d_curpos = () => { return (v2d_curpos.y*texture.width)+v2d_curpos.x; };
        function _urender1(val: TSGmeng.Unit, val2: TSGmeng.Unit = null): string { return _urender_basic_unit(val, null) };
        function _urender(): string { let final_ = ``; for (let i = 0; i < texture.units.length; i++) {  final_ += _urender1(texture.units[i]); }; return final_; };
        console.clear();
        function _uplace_cursor() {
            let redone_texture_units = texture.units;
            let minw  = texture.width+1; // +(2-1) is border c_unit and coord_pointers (since /2, we do /2 to +4) | -1 is offset_value for ceil operation
            let v_pos = Math.ceil((process.stdout.columns - minw) / 2);
            if (cubic_render) {
                let vindx = v2d_curpos.y >= 3 ? (texture.width*3) : 0;
                let pl = v2d_curpos.y % 2 != 0 && v2d_curpos.y==0 ? (texture.width) : (-texture.width);
                let drawn_unit = [redone_texture_units[v1d_curpos()-vindx],redone_texture_units[v1d_curpos()+(v2d_curpos.y!=0?pl:0)-vindx]];
                console.log(`unit at cursor: ${_urender_basic_unit(drawn_unit[0])} | id: ${v1d_curpos()}`)
                let selector = 1;
                if (v2d_curpos.y % 2 == 0) selector = 0;
                let dagkurdu = drawn_unit[selector];
                drawn_unit[selector] = cur_unit;
                process.stdout.cursorTo(v_pos+v2d_curpos.x,4+Math.floor(v2d_curpos.y/2));
                process.stdout.write(_urender_basic_unit(drawn_unit[0]));
            } else {
                process.stdout.cursorTo(0, process.stdout.rows-2);
                console.log(`unit at cursor: ${_urender_basic_unit(texture.units[v1d_curpos()])} | id: ${v1d_curpos()}`)
                process.stdout.cursorTo(v_pos+v2d_curpos.x, 4 + v2d_curpos.y);
                if (texture.units[v1d_curpos()].color == cur_unit.color) return process.stdout.write(TSGmeng.bgcolors[cur_unit.color] + TSGmeng.colors[0] +  "+" + TSGmeng.resetcolor);
                process.stdout.write(_urender_basic_unit(cur_unit));
            }
        };
        let _gu_modifiers = [
            {name: `color`, selected: false},
            {name: `transparent`, selected: false},
            {name: `special`, selected: false},
            {name: `special_clr`, selected: false},
            {name: `special_c_unit`, selected: false}
        ];
        let _gu_selected = 0;
        function __gdraw(active: boolean = false) {
            process.stdout.cursorTo(0, 4);
            console.log(tui.make_line(50, active ? `blue` : `grayish`));
            console.log(tui.clr(`brush:`, active ? `blue` : `grayish`));
            _gu_modifiers.forEach((c, i) => {
                if (i == _gu_selected) c.selected = true;
                c.selected ? 
                console.log(`-> ${tui.clr(c.name, `orange`)} -> ${tui.clr(cur_unit[c.name] + ` `, `green`)}${c.name == `color` || c.name == `special_clr` ? ` ${TSGmeng.colors[cur_unit[c.name]] + TSGmeng.c_unit.repeat(2) + TSGmeng.resetcolor} ` : ``}`) : 
                console.log(`-- ${tui.clr(c.name, `yellow`)} -> ${tui.clr(cur_unit[c.name] + ` ` + `<null>      `, `swamp`)} `);
                c.selected = false; /// so that when it changes we dont have to trace the previous selection
            });
            console.log(tui.clr(`[tab]       to switch selection`, `grayish`));
            console.log(tui.clr(`[enter]     to increase/switch value`, `grayish`));
            console.log(tui.clr(`[backspace] to reduce value (only for integers)`, `grayish`));
            console.log(tui.clr(`[space]     to set value (only for strings)`, `grayish`));
            console.log(tui.clr(`[esc]       to close this menu`, `grayish`));
            console.log(tui.make_line(50, active ? `blue` : `grayish`));
        };
        async function _unit_modifier_menu() {
            return new Promise<void>((resolve, reject) => {
                _udraw_editor(); /// to clear any other menu that might have been displayed beforehand
                __gdraw(true);
                tui.capture([`tab`, `return`, `backspace`, `space`, `escape`], (key, ctrl) => {
                    if (key.name == `escape`) { ctrl.close(); resolve(); return; };
                    if (key.name == `tab`) { _gu_selected++; if (_gu_selected == _gu_modifiers.length ) _gu_selected = 0; };
                    if (key.name == `return`) {
                        let cur_v = cur_unit[_gu_modifiers[_gu_selected].name];
                        if (typeof(cur_v) == "number") cur_unit[_gu_modifiers[_gu_selected].name]++;
                        if (typeof(cur_v) == "boolean") cur_unit[_gu_modifiers[_gu_selected].name] = !cur_unit[_gu_modifiers[_gu_selected].name];
                        if (cur_unit[_gu_modifiers[_gu_selected].name] > 7) cur_unit[_gu_modifiers[_gu_selected].name] = 0;
                    };
                    if (key.name == `backspace`) {
                        let cur_v = cur_unit[_gu_modifiers[_gu_selected].name];
                        if (typeof(cur_v) == "number") cur_unit[_gu_modifiers[_gu_selected].name]--;
                        if (typeof(cur_v) == "boolean") cur_unit[_gu_modifiers[_gu_selected].name] = !cur_unit[_gu_modifiers[_gu_selected].name];
                        if (cur_unit[_gu_modifiers[_gu_selected].name] < 0) cur_unit[_gu_modifiers[_gu_selected].name] = 7;
                    };
                    __gdraw(true);
                });
            });
        };
        let _udrawl = (): string[] => {
            let final = [];
            let ln = ``;
            let skip_to = -1;
            texture.units.forEach((u, i) => {
                let y = Math.floor(i / texture.width);
                let nx_unit = texture.units[i + texture.width];
                if (i == 0) { ln += _urender1(u,nx_unit); return; };
                if (i == texture.units.length - 1) { ln += _urender1(u,nx_unit); final.push(ln); return; };
                if (i % texture.width == 0) { final.push(ln), ln = ``; };
                if (!cubic_render) ln += _urender1(u);
                else ln += _urender1(u, null);
            });
            return final;
        };
        function _udraw_editor(specification: tui.colors[] = null) {
            let v_starttime = process.hrtime();
            process.stdout.cursorTo(0, 0);
            console.log(tui.center_align(tui.clr(`${texture.name} - ${texture.height}x${texture.width} | ${texture.height} rows, ${texture.width} columns`, `orange`)));
            process.stdout.cursorTo(0, 0);
            console.log(tui.clr(`gmeng texture editor`, `dark_red`), tui.clr(`framework: 4.0_glvl`, `red`));
            console.log(tui.make_line(process.stdout.columns, `cyan`));
            /// x coordinate writing
            let v_cxs = (` `);
            for (let i = 0; i < texture.width; i++) { v_cxs += (i % 10 == 0 ? chalk.bold.white.underline(((i / 10 != 0) ? (i / 10) : i / 10).toString()[((i / 10 != 0) ? (i / 10) : i / 10).toString().length > 1 ? 1 : 0]) : chalk.dim((i > 10 ? (i % 10) : i).toString())); }; v_cxs += (` `);
            console.log(tui.center_align(chalk.overline(v_cxs)));
            if (!specification) {
                (border(_udrawl(), texture.width)).split(`\n`).forEach((ln: string, indx: number) => {
                    if (indx == 0 || indx == (texture.height+1)) return console.log(tui.center_align(ln));
                    let v_lnidx  = ((indx - 1) %  10 == 0 || indx == 1) ? ((`${chalk.bold.underline.white(indx-1).toString()} `)) : chalk.dim((`${(indx-(cubic_render?0:1)).toString()} `));
                    let v2_lnidx = ((indx - 1) %  10 == 0 || indx == 1) ? ((` ${chalk.bold.underline.white(indx-1).toString()}`)) : chalk.dim((` ${(indx-(cubic_render?0:1)).toString()}`));
                    console.log(tui.center_align((v_lnidx) + ln + v2_lnidx));
                }); 
            } else {
                (border(_udrawl(), texture.width)).split(`\n`).forEach((ln: string, indx: number) => {
                    if (indx == 0 || indx == (texture.height+1)) return console.log(tui.center_align(ln));
                    let v_lnidx  = ((indx - 1) %  10 == 0 || indx == 1) ? ((`${chalk.bold.underline.white(indx-1).toString()} `)) : chalk.dim((`${(indx-(cubic_render?0:1)).toString()} `));
                    let v2_lnidx = ((indx - 1) %  10 == 0 || indx == 1) ? ((` ${chalk.bold.underline.white(indx-1).toString()}`)) : chalk.dim((` ${(indx-(cubic_render?0:1)).toString()}`));
                    console.log(tui.center_align((v_lnidx) + (specification[indx] ? tui.clr(tui.remove_colorcodes(ln), specification[indx]) : ln) + v2_lnidx));
                });
            };
            let v_rendertime = process.hrtime(v_starttime);
            console.log(tui.center_align(chalk.overline(v_cxs)));
            console.log(tui.make_line(process.stdout.columns, `cyan`));
            console.log(tui.center_align(tui.clr(`pos: ${v2d_curpos.x},${v2d_curpos.y} - ${v1d_curpos()} | for controls press [shift+tab]`, `blue`)));
            let v_drawtime = process.hrtime(v_starttime);
            const v_rendertime_ms = v_rendertime[0] * 1000 + v_rendertime[1] / 1e6;
            const v_drawtime_ms = v_drawtime[0] * 1000 + v_drawtime[1] / 1e6;
            _uplace_cursor();
            process.stdout.cursorTo(0, 5 + texture.height);
            process.stdout.write(tui.clr(`render: ${tui.clr(v_rendertime_ms.toString() + `ms`, `yellow`)} | draw: ${tui.clr(v_drawtime_ms.toString() + `ms`, `yellow`)} | fps: ${tui.clr(Math.floor(1000 / v_drawtime_ms).toString(), `yellow`)}`, `dark_red`))
        };
        _udraw_editor();
        __gdraw();
        setTimeout(() => { _udraw_editor(); }, 5000);
        let v_menu_paused = false;
        let _ulast = texture;
        let kc = (str: string, color: tui.colors = `blue`) => tui.clr(str, color);
        tui.capture([`d`, `escape`, `tab`, `space`, `up`, `down`, `right`, `left`, `pageup`, `pagedown`, `m`, `b`, `q`], async (key, ctrl) => {
            if (v_menu_paused) return;
            if (key.sequence == `\x04`) {
                let vprev = texture;
                texture = _ulast;
                _ulast = vprev;
                return _udraw_editor();
            }; 
            _ulast = texture;
            switch (key.name) {
                case `q`:
                    if (!key.ctrl) return _udraw_editor();
                    let varr_clist: Array<tui.colors> = [];
                    for (let i = 0; i < texture.height+2; i++) {
                        varr_clist[i-1] = "green";
                        varr_clist.push("swamp");
                        _udraw_editor(varr_clist);
                    };
                    console.clear();
                    console.log(`\n`.repeat(Math.floor(process.stdout.rows / 2)));
                    console.log(tui.center_align(tui.clr(`save & quit...`, `dark_red`)));
                    let txstr = builder.txstring(texture);
                    writeFileSync(process.cwd() + `/${v_filename}`, txstr);
                    console.log(`\n`.repeat(Math.floor(process.stdout.rows / 2)));
                    process.exit(1);
                    break;
                case `space`:
                    texture.units[(v2d_curpos.y-(cubic_render&&v2d_curpos.y>=1?1:0))*texture.width+v2d_curpos.x].color = cur_unit.color;
                    texture.units[(v2d_curpos.y-(cubic_render&&v2d_curpos.y>=1?1:0))*texture.width+v2d_curpos.x].special = cur_unit.special;
                    texture.units[(v2d_curpos.y-(cubic_render&&v2d_curpos.y>=1?1:0))*texture.width+v2d_curpos.x].special_clr = cur_unit.special_clr;
                    texture.units[(v2d_curpos.y-(cubic_render&&v2d_curpos.y>=1?1:0))*texture.width+v2d_curpos.x].special_c_unit = cur_unit.special_c_unit;
                    texture.units[(v2d_curpos.y-(cubic_render&&v2d_curpos.y>=1?1:0))*texture.width+v2d_curpos.x].transparent = cur_unit.transparent;
                    process.stdout.cursorTo(0, 3);
                    console.log(texture.units.findIndex(v=>v.color==cur_unit.color), texture.units.find(v=>v.color==cur_unit.color));
                    await tui.await_keypress();
                    _udraw_editor();
                    break;
                case `right`:
                    if (v2d_curpos.x + 1 > texture.width-1) { _udraw_editor(); break; };
                    prev_curpos = v2d_curpos;
                    if (key.shift && v2d_curpos.x + 3 <= texture.width-1) v2d_curpos.x += 2;
                    v2d_curpos.x++;
                    _udraw_editor();
                    break;
                case `left`:
                    if (v2d_curpos.x - 1 < 0) { _udraw_editor(); break; };
                    prev_curpos = v2d_curpos;
                    if (key.shift && v2d_curpos.x - 3 >= 0) v2d_curpos.x -= 2;
                    v2d_curpos.x--;
                    _udraw_editor();
                    break;
                case `up`:
                    if (v2d_curpos.y - 1 < 0) { _udraw_editor(); break; };
                    prev_curpos = v2d_curpos;
                    if (key.shift && v2d_curpos.y - 3 >= 0) v2d_curpos.y -= 2;
                    v2d_curpos.y--; 
                    _udraw_editor();
                    break;
                case `down`:
                    if (v2d_curpos.y + 1 > texture.height - 1) { _udraw_editor(); break; };
                    prev_curpos = v2d_curpos;
                    if (key.shift && v2d_curpos.y + 3 <= texture.height-1) v2d_curpos.y += 2;
                    v2d_curpos.y++;
                    _udraw_editor();
                    break;
                case `pageup`:
                    prev_curpos = v2d_curpos;
                    v2d_curpos.y = 0; _udraw_editor();
                    break;
                case `pagedown`:
                    prev_curpos = v2d_curpos;
                    v2d_curpos.y = texture.height-1; _udraw_editor();
                    break;
                case `tab`: 
                    if (!key.shift) { 
                        prev_curpos = v2d_curpos;
                        if (v2d_curpos.x == texture.width-1) v2d_curpos.x = 0;
                        else v2d_curpos.x = texture.width-1;
                        _udraw_editor(); break; 
                    };
                    __gdraw(true);
                    process.stdout.cursorTo(0, texture.height-27);
                    console.log(kc(`keys:`, `dark_red`));
                    console.log(kc(`[shift+tab]  `), kc(`->`, `grayish`), kc(`show controls`, `pink`));
                    console.log(kc(`[tab]        `), kc(`->`, `grayish`), kc(`set cursor X to max or 0`, `pink`));
                    console.log(kc(`[space]      `), kc(`->`, `grayish`), kc(`place unit at cursor`, `pink`));
                    console.log(kc(`[esc]        `), kc(`->`, `grayish`), kc(`exits editor`, `pink`));
                    console.log(kc(`[shift+m]    `), kc(`->`, `grayish`), kc(`unit modifiers menu`, `pink`))
                    console.log(kc(`[→ ← ↑ ↓]    `), kc(`->`, `grayish`), kc(`control cursor`, `pink`))
                    console.log(kc(`[pageup]     `), kc(`->`, `grayish`), kc(`set cursor Y to 0`, `pink`))
                    console.log(kc(`[pagedown]   `), kc(`->`, `grayish`), kc(`set cursor Y to max`, `pink`))
                    console.log(kc(`[b]          `), kc(`->`, `grayish`), kc(`returns to last cursor position`, `pink`))
                    console.log(tui.make_line(50, `blue`));
                    await tui.usleep(10000);
                    _udraw_editor(); 
                    break;
                case `escape`:
                    _udraw_editor();
                    v_filename = await tui.show_input_menu(`enter filename (_usavetx)`, v_filename);
                    process.stdout.cursorTo(0, 7);
                    console.log(`saving texture -> _uconv_tracer1x_txstring(${texture.name})\nv_result -> ${v_filename} _usavetx() ; ${texture.width},${texture.height} collision=${texture.collidable}`)
                    let v_texture_string = builder.txstring(texture);
                    writeFileSync(process.cwd() + `/${v_filename}`, v_texture_string);
                    console.log(tui.clr(`save successful`, `green`));
                    await tui.usleep(1000);
                    _udraw_editor();
                    break;
                case `m`:
                    if (!key.shift) { _udraw_editor(); break; };
                    v_menu_paused = true;
                    await _unit_modifier_menu();
                    v_menu_paused = false;
                    _udraw_editor();
                    break;
                case `b`:
                    v2d_curpos = prev_curpos;
                    _udraw_editor(); 
                    break;
            };
        });
        };
        __gtexture_editor();
        return texture;
    };
    export function txstring(texture: TSGmeng.fw4_texture): string {
        let final = `name=${texture.name},width=${texture.width},height=${texture.height},collision=${texture.collidable}\n`;
        texture.units.forEach((unit, i) => {
            if (unit.transparent) return final += `__gtransparent_unit__\n`;
            final += `${unit.color} ${unit.special ?? false} ${unit.special_clr ?? 0} ${unit.special_c_unit ?? "!"}\n`;
        });
        final += `; %tsgmeng::builder.texture4_0() -> auto_generated ${(new Date()).toDateString()}`;
        return final;
    };
    export async function model4_0(name: string): Promise<TSGmeng.Model> {
        return;
    };
    /**
     * renders a basic unit (does not support special colors or c_ent_tags \ players \ entities \ etc)
     */
    export function _urender_basic_unit(v: TSGmeng.Unit, v2: TSGmeng.Unit = null): string {
        let currentUnit = v;
        let nextUnit = v2;
        let nu =  v2 != null;
        let preferSecond = false;
        // By default, colors are transparent (void/black)
        let funitColor = TSGmeng.colors[currentUnit.color];
        let bunitColor = nu ? TSGmeng.bgcolors[nextUnit.color] : TSGmeng.bgcolors[0]; // BLACK is assumed to be index 0
        
        // Handling special cases
        if (nu && currentUnit.color == nextUnit.color) {
            return (currentUnit.color != 7 ? TSGmeng.bgcolors_bright[currentUnit.color] : TSGmeng.bgcolors[7]) + " " + TSGmeng.resetcolor;
        }
        
        if (currentUnit.special && !preferSecond) {
            return (v.color != 7 ? TSGmeng.bgcolors_bright[currentUnit.color] : TSGmeng.bgcolors[7]) + chalk.bold(TSGmeng.colors[currentUnit.special_clr]) + currentUnit.special_c_unit + TSGmeng.resetcolor;
        } else if (nu && nextUnit.special && (!currentUnit.special || preferSecond)) {
            return TSGmeng.bgcolors_bright[nextUnit.color] + chalk.bold(TSGmeng.colors[nextUnit.special_clr]) + nextUnit.special_c_unit + TSGmeng.resetcolor;
        }
        
        // Handling transparency
        if (currentUnit.transparent) {
            funitColor = TSGmeng.colors[7]; // Assuming index 7 is the transparent color
        }
        if (nu && nextUnit.transparent) {
            bunitColor = TSGmeng.bgcolors[7];
        }
        // Default rendering logic
        let final = nu ? 
            ((nextUnit.color == 7 ? TSGmeng.bgcolors[7] : TSGmeng.bgcolors_bright[nextUnit.color]) + TSGmeng.colors[currentUnit.color] + ( currentUnit.color != 7 ? chalk.bold(TSGmeng.c_outer_unit_floor):TSGmeng.c_outer_unit_floor )) :
            (TSGmeng.colors[v.color] + TSGmeng.c_unit);
        
        return final + TSGmeng.resetcolor;
    };
    export function border(text: string[], width: number): string {
        let final = ``;
        final += text.map(ln => `${tui.clr(TSGmeng.c_unit, `orange`)}${ln}${tui.clr(TSGmeng.c_unit, `orange`)}`).join(`\n`)
        return  tui.clr(`${TSGmeng.c_outer_unit.repeat(width+2)}`, `orange`) + `\n` +
        final + `\n` + tui.clr(`${TSGmeng.c_outer_unit_floor.repeat(width+2)}`, `orange`);
    };
    export function chunk_deltaX(vp: TSGmeng.fw4_viewpoint): number {
        return (vp.end.x - vp.start.x);
    };
    export function chunk_deltaY(vp: TSGmeng.fw4_viewpoint): number {
        return (vp.end.y - vp.start.y);
    };
        /**
    editor for the `gm4.0-glvl` gamemap framework.
    
    this framework is the recommended and preferred way to handle maps.
    it uses chunking to divide a big 'skybox'-like texture's width-height information into a display.
    a display then renders a chunk and displays it to the screen with a Camera instance.
    This provides the ability to infinitely expand levels, instead of a hard-capped 300x300 top-down view limit on the `gm1.1-sdk` framework.

    note - use the `TSGmeng.install_glvl()` method to set-up a .glvl file (gmeng-level)
    from a .glcp (gmeng-level-compiler-parameters) file
    */
    export async function framework4_0(args: string[]): Promise<void> {
        console.clear();
        let fname = await tui.show_input_menu("enter filename");
        fname = lv_sanitize(fname);
        while (fname.endsWith(".glvl")) { fname = fname.slice(0, -3); }; // remove extensions
        let filename = fname + ".glvl";
        let current_level: TSGmeng.fw4_levelinfo = {
            base: {
                height: null,
                width: null,
                lvl_template: null
            },
            chunks: null,
            description: null,
            display_res: null,
            name: null
        };
        let active_overlay: number = 0;
        let focused_overlay: number = -1;
        if (existsSync(process.cwd() + "/" + filename)) {
            current_level = gm_parsers.fw4_0.__glvl__(process.cwd() + "/" + filename);
        } else {
            // default_init for level (we dont have to deal with annoying menus)
            // they can be tuned by the user via the menu instead
            gm_parsers.fw4_0.__default_init__(current_level); // passed as reference (&namespace)
            current_level.name = fname;
        };
        let current_chunk: { id: number, models: Array<TSGmeng.fw4_model>, vp: TSGmeng.fw4_viewpoint } = 
        current_level.chunks?.length > 0 ?
        { id: 0, vp: current_level.chunks[0].vp, models: current_level.chunks[0].models } 
        : 
        {
            id: 0, vp: { start: { x: 0, y: 0 }, end: { x: 20, y: 10 } },
            models: []
        };
        const __editor_helpers__ = new class __fw4_0__editor__helpers__ {
            /// draws text (supports newlines) at a specific point in the screen as a starting position
            __ui_text__(text: string, posX: number = 0, posY: number = 0) {
                if (posX > process.stdout.columns || posY > process.stdout.rows || posX < 0 || posY < 0) return;
                text.split(`\n`).forEach((ln, indx) => {
                    let rel_x = posX, rel_y = posY+indx;
                    process.stdout.cursorTo(rel_x, rel_y); process.stdout.write(ln);
                });
            };
            /// overlay for the glcp (gmeng level compiler parameters) base data
            /// (width, height, name, description, chunk width, chunk height, base texture)
            __overlay_level_settings__(active: boolean = true, focused = false) {
                let pos = process.stdout.columns - 37;
                if (!active && focused) {
                    __editor_helpers__.__ui_text__([
                        tui.clr(tui.display_delegates.TOP_LEFT + tui.display_delegates.TITLE_START + tui.clr(`settings`, `pink`) + tui.display_delegates.TITLE_END + tui.display_delegates.LINE.repeat(18) + tui.display_delegates.TOP_RIGHT,       `yellow`),
                        tui.clr(tui.clr(tui.display_delegates.SIDE, `yellow`) + `  set overlay to 1 to view  ` + tui.clr(tui.display_delegates.SIDE, `yellow`), `tan`),
                        tui.clr(tui.display_delegates.BOTTOM_LEFT + tui.display_delegates.LINE.repeat(28) + tui.display_delegates.BOTTOM_RIGHT, `yellow`)
                    ].join(`\n`), pos, 2);
                    return;
                };
                if (!active) {
                    __editor_helpers__.__ui_text__([
                        tui.clr(tui.display_delegates.TOP_LEFT + tui.display_delegates.TITLE_START + tui.clr(`settings`, `grayish`) + tui.display_delegates.TITLE_END + tui.display_delegates.LINE.repeat(18) + tui.display_delegates.TOP_RIGHT,       `cyan`),
                        tui.clr(tui.clr(tui.display_delegates.SIDE, `cyan`) + `  set overlay to 1 to view  ` + tui.clr(tui.display_delegates.SIDE, `cyan`), `tan`),
                        tui.clr(tui.display_delegates.BOTTOM_LEFT + tui.display_delegates.LINE.repeat(28) + tui.display_delegates.BOTTOM_RIGHT, `cyan`)
                    ].join(`\n`), pos, 2);
                    return;
                };
                let ui_text = __editor_helpers__.__ui_text__;
                let items = [
                    { name: `level name   -> ${tui.clr(current_level.name       .replace(current_level.name.substring(15),        ``), `blue`)}`, id: 0, selected: true  },
                    { name: `level desc   -> ${tui.clr(current_level.description.replace(current_level.description.substring(15), ``), `blue`)}`, id: 1, selected: false },
                    { name: `chunk width  -> ${tui.clr(`${current_level.display_res[0]}`, `blue`)}`, id: 2, selected: false },
                    { name: `chunk height -> ${tui.clr(`${current_level.display_res[1]}`, `blue`)}`, id: 2, selected: false },
                ];
                for (let i = 0; i < items.length; i++) {
                    process.stdout.cursorTo(pos, i + 2);
                    process.stdout.write(` `.repeat(process.stdout.columns - pos));
                };
                let overlay__ = tui.overlay('settings', items, { x: pos - current_level.description.length + 5, y: 2});
                overlay__.on(`selection_made`, async (item: number) => {
                    console.clear();
                    switch (item) {
                        case 0: // LEVEL_NAME
                            let nname = await tui.show_input_menu(`enter level name`);
                            current_level.name = nname.replace(nname.substring(15), ``);;
                            break;
                        case 1: // LEVEL_DESC
                            let ndesc = await tui.show_input_menu(`enter level desc`);
                            current_level.name = ndesc.replace(ndesc.substring(15), ``);
                            break;
                        case 2: // LEVEL_CHUNK_SPLIT_WIDTH
                            let ncwidth = await tui.show_input_menu(`enter width`);
                            if (isNaN(parseInt(ncwidth))) { console.clear(); console.log(tui.center_align(tui.clr(`(int) value of (string)${ncwidth} is NaN`, `dark_red`))); await tui.usleep(3000); break; };
                            current_level.display_res[0] = parseInt(ncwidth);
                            break;
                        case 3: // LEVEL_CHUNK_SPLIT_HEIGHT
                            let ncheight = await tui.show_input_menu(`enter height`);
                            if (isNaN(parseInt(ncheight))) { console.clear(); console.log(tui.center_align(tui.clr(`(int) value of (string)${ncheight} is NaN`, `dark_red`))); await tui.usleep(3000); break; };
                            current_level.display_res[1] = parseInt(ncheight);
                            break;   
                        default: // UNKNOWN SPOT
                            ui_text(tui.clr(`<unknown option: ??????>`, `dark_red`), pos, 2);
                            break;
                    };
                });
                overlay__.on(`destroy`, () => {
                    focused_overlay = -1;
                    __draw__();
                });
            };
            /// overlay for the glcp (gmeng level compiler parameters) textures 
            /// what textures should be packed with it (if any)
            /// this overlay lists the textures & models within the vgm
            /// and any other texture / model that was imported to the editor
            /// this is then compiled into the compiler parameters file
            /// as a plib directory or lmf (lexed master file) 
            __overlay_vgm_controller__(active: boolean = true, focused: boolean = false) {
                let pos = 2;
                if (!active && focused) {
                    __editor_helpers__.__ui_text__([
                        tui.clr(tui.display_delegates.TOP_LEFT + tui.display_delegates.TITLE_START + tui.clr(`textures`, `pink`) + tui.display_delegates.TITLE_END + tui.display_delegates.LINE.repeat(18) + tui.display_delegates.TOP_RIGHT,       `yellow`),
                        tui.clr(tui.clr(tui.display_delegates.SIDE, `yellow`) + `  set overlay to 0 to view  ` + tui.clr(tui.display_delegates.SIDE, `yellow`), `tan`),
                        tui.clr(tui.display_delegates.BOTTOM_LEFT + tui.display_delegates.LINE.repeat(28) + tui.display_delegates.BOTTOM_RIGHT, `yellow`)
                    ].join(`\n`), pos, 2);
                    return; 
                };
                if (!active) {
                    __editor_helpers__.__ui_text__([
                        tui.clr(tui.display_delegates.TOP_LEFT + tui.display_delegates.TITLE_START + tui.clr(`textures`, `grayish`) + tui.display_delegates.TITLE_END + tui.display_delegates.LINE.repeat(18) + tui.display_delegates.TOP_RIGHT,       `cyan`),
                        tui.clr(tui.clr(tui.display_delegates.SIDE, `cyan`) + `  set overlay to 0 to view  ` + tui.clr(tui.display_delegates.SIDE, `cyan`), `tan`),
                        tui.clr(tui.display_delegates.BOTTOM_LEFT + tui.display_delegates.LINE.repeat(28) + tui.display_delegates.BOTTOM_RIGHT, `cyan`)
                    ].join(`\n`), pos, 2);
                    return;
                };
                let items: Array<{name: string, id: number, selected: boolean}> = [
                    {name: `[import texture_pack (.gtxp)]`, id: 0, selected: true}
                ];
                let textures: Array<{name: string, id: number, texture: TSGmeng.fw4_texture}> = [];
                let indx = 0;
                gm_parsers.__vgm_init__();
                vgm_defaults.vg_textures.forEach((tx) => {
                    let obj = {
                        name: tx.name + " (" + Math.floor(Math.random() * 1000000) + " - " + tx.width +"*" + tx.height + " - " + (tx.collidable ? tui.clr("c", "green") : tui.clr("c", "dark_red")) +")",
                        id: indx+1,
                        selected: false,
                        texture: tx
                    };
                    items.push({
                        name: obj.name, id: obj.id, selected: obj.selected
                    });
                    textures.push(obj);
                    indx++;
                });
                let vgm_controller = tui.overlay(`textures`, items, { x: 2, y: 2 });
                vgm_controller.on(`selection_change`, (item: number) => {
                    for (let i = 0; i < 28; i++) {
                        process.stdout.cursorTo(0, (process.stdout.rows - 29) + i);
                        console.log(` `.repeat(process.stdout.columns));
                    };
                    process.stdout.cursorTo(2, process.stdout.rows-29);
                    if (item == 0) return; // import texture pack, not supported currently
                    let fin = ""; let selected_item = items.find(i => i.id == item);
                    let texture = textures.find(f => f.id == item).texture;
                    if (texture.width > process.stdout.columns - 2 - 3 - 2 - 3 ||
                        texture.height > process.stdout.rows- 29 -3            ) { 
                        return console.log(tui.clr(`<texture can not fit into the screen>`, `red`)); };
                    for (let y = 0; y < texture.height; y++) {
                        for (let x = 0; x < texture.width; x++) {
                            fin += _urender_basic_unit(texture.units[
                                                y * texture.width + x
                                                                    ]
                                                      );
                        };
                        fin += "\n";
                    };
                    fin = fin.slice(0, -1); /// remove trailing \n
                    let v_cxs = (` `);
                    for (let i = 0; i < texture.width; i++) { v_cxs += (i % 10 == 0 ? chalk.bold.white.underline(((i / 10 != 0) ? (i / 10) : i / 10).toString()[((i / 10 != 0) ? (i / 10) : i / 10).toString().length > 1 ? 1 : 0]) : chalk.dim((i > 10 ? (i % 10) : i).toString())); }; v_cxs += (` `);
                    process.stdout.cursorTo(4, process.stdout.rows-29);
                    console.log((chalk.overline(v_cxs)));
                    process.stdout.cursorTo(5, process.stdout.rows-28);
                    (border(fin.split("\n"), texture.width).split(`\n`).forEach((ln, indx) => {
                            if (indx == 0 || indx == (texture.height)+1) return process.stdout.cursorTo(4, process.stdout.rows-28+indx), console.log((ln));
                            let v_lnidx  = ((indx - 1) %  10 == 0 || indx == 1) ? ((`${chalk.bold.underline.white(indx-1).toString()} `)) : chalk.dim((`${(indx-1).toString()} `));
                            let v2_lnidx = ((indx - 1) %  10 == 0 || indx == 1) ? ((` ${chalk.bold.underline.white(indx-1).toString()}`)) : chalk.dim((` ${(indx-1).toString()}`));
                            process.stdout.cursorTo(2, process.stdout.rows-28 + indx);
                            console.log((v_lnidx) + ln + (v2_lnidx));
                    }));
                    process.stdout.cursorTo(0), process.stdout.cursorTo(4); console.log((chalk.overline(v_cxs)));
                });
                vgm_controller.on(`selection_made`, (item) => {

                });
                vgm_controller.on(`destroy`, () => {
                    // __draw__() is executed FASTER than a tui.capture listener is destroyed
                    // this is unbelievably retarded
                    focused_overlay = -1;
                    __draw__();
                });
            };
            /// calculates the position of &(ref __p, __s, __w) within a world.
            /// an object with the size __s at the position __p in a world with the size __w.
            /// imported into typescript from the c++ engine lib: Gmeng::Renderer::get_placement(__p, __s, __ws);
            __placement_calculator__(__p: TSGmeng.fw4_drawpoint, __s: TSGmeng.fw4_drawpoint, __w: TSGmeng.fw4_drawpoint) {
                const __delegate_positions__ = [];
                if (__s.x > __w.x || __s.y > __w.y) return [-1]; // errored (__s is bigger than __w)
                if ((__p.x < 0 || __p.y < 0 || __p.x + __s.x > __w.x || __p.y + __s.y > __w.y)) return [-1]; // errored (__p is out of bounds)
                for (let x = 0; x < __s.x; x++) {
                    for (let y = 0; y < __s.y; y++) {
                        const __delegate_x__ = __p.x + x, __delegate_y__ = __p.y + y;
                        __delegate_positions__.push({ x: __delegate_x__, y: __delegate_y__ });
                    };
                };
                return __delegate_positions__; 
            };
            /// chunk draw - draws the chunk to the screen
            /// same as fw4.0_glvl.texture4_0 display
            __chunk_view__() {
                let lines: Array<Array<string>> = [];
                for (let y = current_chunk.vp.start.y; y < current_chunk.vp.end.y; y++) {
                    lines[y] = [];
                    for (let x = current_chunk.vp.start.x; x < current_chunk.vp.end.x; x++) {
                        lines[y][x] = _urender_basic_unit(current_level.base.lvl_template.units[
                                                   y * current_level.base.lvl_template.width + x
                                                                                             ]);
                    };
                };
                let fin = [];
                lines.map(ln => fin.push(ln.join(``)));
                let v_cxs = (` `);
                for (let i = 0; i < chunk_deltaX(current_chunk.vp); i++) { v_cxs += (i % 10 == 0 ? chalk.bold.white.underline(((i / 10 != 0) ? (i / 10) : i / 10).toString()[((i / 10 != 0) ? (i / 10) : i / 10).toString().length > 1 ? 1 : 0]) : chalk.dim((i > 10 ? (i % 10) : i).toString())); }; v_cxs += (` `);
                console.log(tui.center_align(chalk.overline(v_cxs)));
                let sfin = border(fin, chunk_deltaX(current_chunk.vp));
                sfin.split(`\n`).forEach((ln: string, indx: number) => {
                    if (indx == 0 || indx == (chunk_deltaY(current_chunk.vp)+1)) return console.log(tui.center_align(ln));
                    let v_lnidx  = ((indx - 1) %  10 == 0 || indx == 1) ? ((`${chalk.bold.underline.white(indx-1).toString()} `)) : chalk.dim((`${(indx-1).toString()} `));
                    let v2_lnidx = ((indx - 1) %  10 == 0 || indx == 1) ? ((` ${chalk.bold.underline.white(indx-1).toString()}`)) : chalk.dim((` ${(indx-1).toString()}`));
                    console.log(tui.center_align((v_lnidx) + ln + v2_lnidx));
                });
                console.log(tui.center_align(chalk.overline(v_cxs)) + "\n");
            };
            __draw_texture__(tx: TSGmeng.fw4_texture): string {
                let fin = ""
                for (let y = 0; y < tx.height; y++) {
                    for (let x = 0; x < tx.width; x++) {
                        fin += _urender_basic_unit(tx.units[y * tx.width + x]);
                    };
                    fin += "\n";
                };
                return fin;
            };
            __draw_image_to__(tx: string, pos: { x: number, y : number }) {
                let spacing = " ".repeat(Math.ceil((process.stdout.columns - chunk_deltaX(current_chunk.vp)+1)/2));
                let line_count_of_useless_text_for_morons = 4;
                tx.split(`\n`).forEach((l, indx) => {
                    process.stdout.cursorTo(spacing.length + pos.x, pos.y + line_count_of_useless_text_for_morons + indx);
                    console.log(l);
                });
            };
            /// model placement method
            async __model_placement__(__item: TSGmeng.fw4_texture): Promise<TSGmeng.fw4_model> {
                return new Promise<TSGmeng.fw4_model>(async (resolve, reject) => {                    
                    let dp_pos: TSGmeng.fw4_drawpoint = { x: 0, y: 0 };
                    let model = new TSGmeng.fw4_model();
                    let id = (Math.floor(Math.random() * 1000000));
                    model.texture = __item;
                    model.name = __item.name + "_gmdl." + id.toString();
                    model.width = __item.width;
                    model.height = __item.height;
                    model.id = id;
                    function __write_texture__(pos: { x: number, y: number }) {
                        __editor_helpers__.__draw_image_to__(__editor_helpers__.__draw_texture__(__item), pos);
                    };
                    let __inited__ = false;
                    async function placement_k() {
                        return new Promise<void>((resolve, reject) => {
                            tui.capture([`up`, `down`, `left`, `right`, `return`, `escape`], (key, ctrl) => {
                                switch(key.name) {
                                    case 'return':
                                        model.position = dp_pos;
                                        current_chunk.models.push(model);
                                        break;
                                    case 'escape':
                                        model.name == "__UNKNOWN__";
                                        model.id = -1;
                                        break;
                                    case 'up':
                                        if (dp_pos.y - 1 < 0) break;
                                        dp_pos.y--;
                                        break;
                                    case 'down':
                                        if (dp_pos.y + 1 > chunk_deltaY(current_chunk.vp)) break;
                                        dp_pos.y++;
                                        break;
                                    case 'right':
                                        if (dp_pos.x + 1 > chunk_deltaX(current_chunk.vp)) break;
                                        dp_pos.x++;
                                        break;
                                    case 'left':
                                        if (dp_pos.x - 1 < 0) break;
                                        dp_pos.x--;
                                        break;
                                };
                                ctrl.close();
                                __write_texture__(dp_pos);
                            });
                            resolve(void 0);
                        });
                    };
                    placement_k();
                });
            };
        };
        const titles = {
            'level_editor': tui.clr(`gmeng ${tui.clr(`level editor`, `red`)}`, `dark_red`),
        };
        async function __draw__(no_overlays = false) {
            process.stdout.cursorTo(0, 0);
            process.stdout.write(tui.center_align(tui.clr(`${current_level.name} (${filename}) | ${current_level.description}`, `yellow`)));
            process.stdout.cursorTo(0, 0);
            process.stdout.write((titles.level_editor));
            console.log(`\n` + tui.make_line(process.stdout.columns, `blue`));
            __editor_helpers__.__chunk_view__();
            console.log(tui.upk + tui.make_line(process.stdout.columns, `blue`));
            console.log(tui.center_align(`overlay: ${active_overlay} | chunk_number: ${current_chunk.id}`));
            let overlays = [
                __editor_helpers__.__overlay_vgm_controller__,
                __editor_helpers__.__overlay_level_settings__
            ];
            overlays.forEach((mtd, indx) => { mtd(false, active_overlay == indx); });
            let focused = overlays[focused_overlay < overlays.length-1 ? focused_overlay : overlays.length-1];
            if (focused_overlay != -1) focused(true, true);
            process.stdout.cursorTo(0, 2 + current_level.base.height);
        }
        function __edit__() {
            process.stdin.setRawMode(true); process.stdin.setDefaultEncoding(`utf-8`);
            console.clear();
            __draw__();
        };
        let __focused__ = true;
        tui.capture([`tab`, `return`], (key, ctrl) => {
            if (focused_overlay == -1) __focused__ = true;
            if (!__focused__) return;
            switch (key.name) {
                case `tab`: // switch active overlay 
                    active_overlay++;
                    if (active_overlay > 1 || active_overlay < 0) active_overlay = 0;
                    __draw__();
                    break;
                case `return`:
                    __focused__ = false;
                    focused_overlay = active_overlay;
                    __draw__();
                    break;
                default: 
                    break;
            };
        });
        __edit__();
    };
    /**
    editor for the `gm1.1-sdk` gamemap framework. 

    this framework is outdated, it does not support models, textures, chunks
    and it is deprecated. It also compiles into a single file, but is decompiled
    into 3 different files at runtime when the player is in the world.

    (player.dat - player data, color)

    (world.dat - world data, width, height)

    (world.mpd - unit data)

    This framework handles maps (wmap.gm -> world.dat, player.dat, world.mpd) as a single top-down Camera instance.
    */
    export async function framework1_1(args: string[]): Promise<void> {
        console.clear();
        let WorldData: TSGmeng.WorldData = {
            player: {
                startDX: 0, startDY: 0,
                colored: true, colorId: 1,
                c_ent_tag: `o`
            },
            _w: 2, _h: 2, name: `world1`,
            description: `hello world!`
        };
        WorldData.name = await tui.show_input_screen(`enter WorldMap filename`, null, false, null, `level framework: 1.1_gm `);
        if (existsSync(`${WorldData.name}.gm`)) return builder.show_editor(TSGmeng.parseMapData(WorldData.name), new Plib.Dirs.Directory(WorldData.name + `.gm`));
        WorldData.description = await tui.show_input_screen(`enter world description`, null, false);
        await tui.show_input_screen(`enter world size in blocks (example: 2x2)`, null, false).then(a => {
            let wsze = a.split(`x`);
            if (wsze.length < 2) return console.log(`bad argument: ${a}`), process.exit(0);
            WorldData._w = parseInt(wsze[1]); WorldData._h = parseInt(wsze[0]);
        });
        WorldData.player.colorId = 0;
        await tui.show_input_screen(`enter player spawn position in blocks (example: 2,2)`, null, false).then(a => {
            let wsze = a.split(`,`);
            if (wsze.length < 2) return console.log(`bad argument: ${a}`), process.exit(0);
            WorldData.player.startDX = parseInt(wsze[0]); WorldData.player.startDY = parseInt(wsze[1]);
        });
        console.log(`configuration complete.`);
        console.log(`creating mapfile ${WorldData.name}.gm ...`);
        let wmap_dirfile = Plib.Dirs.DirHandler.CreateDirectoryObject(`${WorldData.name}.gm`);
        wmap_dirfile.writeFile(`world.dat`, `width=${WorldData._h}\nheight=${WorldData._w}\nname=${WorldData.name}\ndescription=${WorldData.description}`);
        wmap_dirfile.writeFile(`player.dat`, `startDX=${WorldData.player.startDX}\nstartDY=${WorldData.player.startDY}\ncolored=${WorldData.player.colored ? `true` : `false`}\ncolorId=${WorldData.player.colorId}`);
        console.log(WorldData);
        console.log(wmap_dirfile.__raw_json());
        console.log(`data is saved to ${WorldData.name}.gm/world.dat | player data is saved to ${WorldData.name}.gm/player.dat`);
        console.log(`launching editor...`);
        console.log(`${tui.clr(`WARNING!`, `orange`)}`);
        console.log(`the end result of the map will be slightly different from the map that will be rendered in-game.`);
        console.log(`this is caused by the limitations of TypeScript being slow at calculating different things related to Gmeng units.`);
        console.log(`${tui.clr(`press any key to continue.`, `swamp`)}`);
        await tui.await_keypress();
        console.clear();
        builder.show_editor(WorldData, wmap_dirfile);
    };
    /**
     * framework 1.1_sdk editor (will not cooperate with fw4.0_glvl)
     * @deprecated use framework 4.0_glvl instead
     */
    export async function show_editor(worldData: TSGmeng.WorldData, mapfile: Directory<string>) {
        let WMAP_UNITS: Array<TSGmeng.Unit> = [];
        if (mapfile.contents.FILE_LIST.find(a=>a.name==`world.mpd`) != null) WMAP_UNITS = TSGmeng.ReadMapData(worldData, mapfile.contents.FILE_LIST.find(a=>a.name==`world.mpd`).content);
        else for (let i = 0; i < worldData._w * worldData._h; i++) {
            WMAP_UNITS.push({
                transparent: false,
                collidable: true,
                color: 7,
                entity: null,
                is_entity: false,
                is_player: false,
                player: null,
                special: false,
                special_clr: 0,
                special_c_unit: `!`
            });
        };
        // ^^ map is created, editor can now make changes

        let SIZEX = 1; let SIZEY = 1; let SELECTCLR = 0;
        let POSX = 0; let POSY = 0;
        let currnt_unit: TSGmeng.Unit = {
            color: SELECTCLR, entity: null, is_entity: false, is_player: false, transparent: false,
            collidable: false, player: null, special: false, special_clr: 0, special_c_unit: `!`,
        };
        function draw() {
            console.clear();
            let rmap = TSGmeng.render_unitmap(worldData, WMAP_UNITS, POSX, POSY, SIZEX, SIZEY, SELECTCLR);
            console.log(`${worldData._h}x${worldData._w} - ${worldData.description} ` + tui.clr(`${worldData.name}.gm`, `pink`), `| ${chalk.bold.red(`G${chalk.bold.blue(`m${chalk.bold.green(`e`)}n`)}g`)} ${tui.clr(`MapEditor`, `orange`)} ${tui.clr(`BETA 1.0`, `red`)}`)
            process.stdout.write(rmap);
            console.log(`\n${chalk.bold.bgWhite.yellow(`KEYBOARD CONTROLS:`)} [space] place object | [enter] change collision modifier | [w a s d] move | [→ ← ↑ ↓] resize object | [tab] change color |`);
            process.stdout.write(`${chalk.bold.yellow.bgWhite(`OBJECT INFO:`)} posX: ${POSX} | posY: ${POSY} | collidable: ${currnt_unit.collidable} | color: ${currnt_unit.color} | width: ${SIZEX} | height: ${SIZEY} | occupied space: ${Math.round(((SIZEX*SIZEY)/(worldData._h*worldData._w))*100)}% | object_type: TSGmeng.Unit & Gmeng::Unit | special: ${currnt_unit.special}`);
            process.stdout.write(tui.center_align(`Use [ctrl+s] to save the map to: ${worldData.name}.`));
        };
        tui.capture([`space`, `return`, `tab`, `w`, `a`, `s`, `d`, `up`, `right`, `down`, `left`], (key) => {
            switch (key.name) {
                case 'tab':
                    SELECTCLR ++;
                    if (SELECTCLR > 7) SELECTCLR = 0;
                    currnt_unit.color = SELECTCLR;
                    break;
                case 'right':
                    if (key.shift) SIZEX+=2;
                    SIZEX++;
                    break;
                case 'left':
                    if (key.shift) SIZEX-=2;
                    SIZEX--;
                    if (SIZEX < 1) SIZEX = 1;
                    break;
                case 'up':
                if (key.shift) SIZEY-=2;

                    SIZEY--;
                    if (SIZEY < 1) SIZEY = 1;
                    break;
                case 'down':
                    if (key.shift) SIZEY+=2;
                    SIZEY++;
                    if (SIZEY < 1) SIZEY = 1;
                    break;
                case 'w':
                    if (key.shift) POSY-=2;
                    POSY--;
                    break;
                case 's':
                    if (key.ctrl) {
                        mapfile.writeFile(`world.mpd`, TSGmeng.CreateMapData(worldData, WMAP_UNITS));
                        console.log(mapfile.contents.FILE_LIST.find(i=>i.name==`world.mpd`));
                        console.log(`\nSaving map to ${worldData.name}.gm/world.mpd`);
                        let SAVED_DATA = mapfile.contents.FILE_LIST.find(i=>i.name=='world.mpd').content;
                        let toWorldData = TSGmeng.ReadMapData(worldData, SAVED_DATA);
                        console.log(TSGmeng.render_unitmap(worldData, toWorldData, 0, 0, 1, 1, 0));
                        tui.usleep(5000);
                        break;
                    };
                    if (key.shift) POSY+=2;
                    POSY++;
                    break;
                case 'a':
                    if (key.shift) POSX-=2;
                    POSX--;
                    break;
                case 'd':
                    if (key.shift) POSX+=2;
                    POSX++;
                    break;
                case 'return':
                    currnt_unit.collidable = !currnt_unit.collidable;
                    break;
                case 'space':
                    let highlighted_unitIds: Array<{x: number, y: number}> = [];
                    for (let i = 0; i < SIZEX; i++) {
                        for (let j = 0; j < SIZEY; j++) {
                            highlighted_unitIds.push({x: (POSX+i), y: (POSY+j)});
                        };
                    };
                    highlighted_unitIds.map(unitCoords => {
                        WMAP_UNITS[(worldData._h*(unitCoords.y))+unitCoords.x] = {
                            color: SELECTCLR, entity: null, is_entity: false, is_player: false, transparent: false,
                            collidable: currnt_unit.collidable, player: null, special: false, special_clr: 0, special_c_unit: `!`,
                        };
                        return void 0;
                    });
                    break;
            };
            setTimeout(draw, 50);
        });
        draw();
    };
};

export namespace vgm_defaults {
    export let vg_rdmodels: Map<string, TSGmeng.fw4_model> = new Map;
    export let vg_textures: Map<string, TSGmeng.fw4_texture> = new Map;
};
export const __vgm_err6556_contl__ = `libts-gmeng: __vgm_init__ failed: no vgm directory (./envs/models does not exist)`;
export const __vgm_err6557_unimpl_e__ = `libts-gmeng: __vgm_init__ failed: tsgmeng currently does not support .mdl files`;
export const __gm_err_unimpl_e__ = `libts-gmeng: unknown system error`;
export const __gm_err65536_overlay_id_unimpl_e__ = `libts-gmeng: OverlayController must include an id`;
export namespace gm_parsers {
    export declare namespace gmeng {};
    export function __vgm_init__() {
        if (!existsSync(`./envs/models`)) throw new RangeError((__vgm_err6556_contl__ ?? __gm_err_unimpl_e__));
        readdirSync(`${process.cwd()}/envs/models`).forEach((fl, indx) => {
            if (fl.endsWith(`.gt`)) { let dv = builder.load_texture(`./envs/models/${fl}`); vgm_defaults.vg_textures.set(dv.name, dv); }
            else throw new TypeError((__vgm_err6557_unimpl_e__ ?? __gm_err_unimpl_e__));
        });
    };
};

export function v_gen_units(height: number, width: number, color: number): TSGmeng.Unit[] {
    let units: TSGmeng.Unit[] = [];
    for (let i = 0; i < height * width; i++) {
        units.push({
            color: color,
            collidable: true
        });
    };
    return units;
};


export namespace gm_parsers.fw4_0 {
    export function __default_init__(val: TSGmeng.fw4_levelinfo): absolute<void> {
        val.base.width = (2**10)*2; // MAX LIMIT (should not be reached anyways)
        val.base.height = 2**10; // MAX LIMIT (should not be reached anyways)
        val.description = `New Level ${Math.floor(Math.random() * 100)}.${crypto.randomUUID().substring(29)}`;
        val.base.lvl_template = {
            collidable: true,
            height: 100,
            width: 200,
            name: `%base.template ${Math.floor(Math.random() * 100)}.${crypto.randomUUID().substring(29)}`,
            units: v_gen_units((2**10)*2, (2**10), 0)
        };
        val.display_res = [20, 10];
    };
    export function __chunk__(v_str: string): { chunk: TSGmeng.fw4_chunk, model_macros: Array<string> } {
        // #chunk p1x=<num> p1y=<num> p2x=<num> p2y=<num> <mdl1>,<mdl2>
        let data = v_str.split(` `).slice(1); // remove keyword
        let v_models: Array<string> = data[4].split(`,`); // macros for models (used in __glvl__)
        let chunk: TSGmeng.fw4_chunk = {
            vp: {
                start: {
                    x: parseInt(data[0].substring(4)),
                    y: parseInt(data[1].substring(4))
                },
                end: {
                    x: parseInt(data[2].substring(4)),
                    y: parseInt(data[3].substring(4))
                },
            },
            models: []
        };
        return { chunk, model_macros: v_models };
    };
    export function __txtr__(v_str: string): { txtr: TSGmeng.fw4_texture, macro: string } {
        // #texture <macro> <filename | vgm_id>
        let data = v_str.split(` `).slice(1); // remove keyword
        let gtx = vgm_defaults.vg_textures.has(data[1]) ? vgm_defaults.vg_textures.get(data[1]) : builder.load_texture(data[1])
        return {
            macro: data[0],
            txtr: gtx
        };
    };
    export function __mdl__(v_str: string): { mdl: TSGmeng.fw4_model, macro: string, txtr_macro: string } {
        // #model <macro> px=<num> py=<num> w=<num> h=<num> tx=<macro>
        let data = v_str.split(` `).slice(1);
        return {
            mdl: {
                position: { x: parseInt(data[1].substring(3)), y: parseInt(data[2].substring(3)) },
                width:  parseInt(data[3].substring(2)), height: parseInt(data[4].substring(2)),
                get size() { return this.width * this.height; },
                texture: null, name: data[0]
            },
            macro: data[0],
            txtr_macro: data[5].substring(3)
        };
    };
    export function __glvl__(v_str: string): TSGmeng.fw4_levelinfo {
        gm_parsers.__vgm_init__(); // load defaults into vgm
        let textures: Map<string, TSGmeng.fw4_texture> = new Map;
        let models: Map<string, TSGmeng.fw4_model> = new Map;
        vgm_defaults.vg_rdmodels.forEach((mdl) => { models.set(mdl.name, mdl); });
        vgm_defaults.vg_textures.forEach((txr) => { textures.set(txr.name, txr); });
        let lvl: TSGmeng.fw4_levelinfo;
        let lines = v_str.split(`\n`).filter(ln => !ln.startsWith(`;`) && ln.length > 1 && ln.replaceAll(` `, ``).length > 0);
        let a_lbase: Array<string> = [];
        let v_lbase: TSGmeng.fw4_lbase;
        lines.forEach((ln: string, indx: number) => {
            let line = ln.split(`;`)[0];
            let data = line.split(` `);
            if ([0,1,2,3].includes(indx)) return a_lbase.push(line.split(`=`)[1]); // only values, not the variable names
            if (indx == 4) { lvl.name = a_lbase[0]; lvl.description = a_lbase[1]; lvl.base.lvl_template = (vgm_defaults.vg_textures.get(a_lbase[2]) ?? builder.load_texture(a_lbase[2])) ?? null; lvl.base.width = parseInt(a_lbase[3].split(`,`)[0]); lvl.base.height = parseInt(a_lbase[3].split(`,`)[1]); delete a_lbase[0], a_lbase[1], a_lbase[2], a_lbase[3]; };
            switch (data[0]) {
                case `#texture`:
                    let vtx = __txtr__(line);
                    textures.set(vtx.macro, vtx.txtr);
                    break;
                case `#model`:
                    let vmdl = __mdl__(line);
                    vmdl.mdl.texture = textures.get(vmdl.txtr_macro);
                    models.set(vmdl.macro, vmdl.mdl);
                    break;
                case `#chunk`:
                    let vchunk = __chunk__(line);
                    vchunk.model_macros.forEach((macro) => { vchunk.chunk.models.push(models.get(macro)); });
                    lvl.chunks.push(vchunk.chunk);
                    break;
            };
        });
        return lvl;
    };
};

export namespace TSGmeng {
    export const colors: Array<string> = [
        "\x1B[39m", "\x1B[34m", "\x1B[32m", "\x1B[36m", "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m"
    ];    
    export const bgcolors: Array<string> = [
        "\x1B[47m", "\x1B[44m", "\x1B[42m", "\x1B[46m", "\x1B[41m", "\x1B[45m", "\x1B[43m", "\x1B[40m"
    ];
    export const bgcolors_bright: Array<string> = [
        "\x1B[107m", "\x1B[104m", "\x1B[102m", "\x1B[106m", "\x1B[101m", "\x1B[105m", "\x1B[103m", "\x1B[100m"
    ];
    export const resetcolor = "\x1b[0m";
    export const c_unit = "\u2588";
    export const c_outer_unit = "\u2584";
    export const c_outer_unit_floor = "\u2580";
    export interface WorldData {
        player: TSGmeng.PlayerData;
        _w: number; _h: number;
        name: string; description: string;
    };
    export interface PlayerData {
        startDX: number; startDY: number;
        colored: boolean; colorId: number;
        c_ent_tag: string;
    };
    export interface fw4_lbase {
        lvl_template: fw4_texture; width: number; height: number;
    };
    export interface fw4_drawpoint { x: number; y: number; };
    export interface fw4_viewpoint { start: fw4_drawpoint; end: fw4_drawpoint; };
    export class fw4_model {
        public width: number; public height: number; position: fw4_drawpoint;
        public name: string; public id?: number = Math.floor(Math.random() * 10000000); public texture: fw4_texture;
        private get_pointXY?(pos: number): fw4_drawpoint {
            return { x: (pos % this.width), y: (pos / this.width) };
        };
        public get size() { return (this.width * this.height); };
        public load_texture?(__tf: string): void { this.texture = builder.load_texture(__tf); };
        public attach_texture?(__t: fw4_texture): void { this.texture = __t; };
    };
    export interface fw4_chunk {
        vp: fw4_viewpoint;
        models: Array<fw4_model>;
    };
    export interface fw4_levelinfo {
        base: fw4_lbase; name: string; display_res: Array<number>;
        chunks: Array<fw4_chunk>; description: string;
    };
    export interface coord { x: number; y: number; };
    export interface Model {
        name: string; id: number; units: Array<TSGmeng.Unit>; size: number;
        texture: TSGmeng.fw4_texture; width: number; height: number; pos: TSGmeng.coord;
    };
    export interface fw4_texture {
        name: string; units: Array<TSGmeng.Unit>;
        width: number; height: number; collidable: boolean;
    };
    export class Unit {
        color: number; collidable?: boolean = true; transparent?: boolean = false;
        is_entity?: boolean; is_player?: boolean;
        special?: boolean; special_clr?: number;
        entity?: TSGmeng.Entity; player?: TSGmeng.Player;
        special_c_unit?: string;
    };
    export class Entity {
        entityId: number; textureId: number; colorId: number;
        textured: boolean; colored: boolean; coords: TSGmeng.coords;
        public setColor(id: number): void { this.colorId = id; };
    };
    export class Player extends Entity {
        textureId: number = 0; colorId: number = 1;
        textured: boolean = false; colored: boolean = true;
        c_ent_tag: string = `o`;
    };
    export interface coords {
        x: number; y: number;
    };

    export const stob = (v: string) => (v == 'true');
    export function add_border(wdata: TSGmeng.WorldData, rmap: string) {
        return `${tui.clr(TSGmeng.c_outer_unit.repeat(wdata._h+2), `orange`)}\n${rmap.split(`\n`).map((uln: string) => { if (uln.length < 5) return `\u001b[A\r`; return `${tui.clr(TSGmeng.c_unit, `orange`)}${uln}${tui.clr(TSGmeng.c_unit, `orange`)}` }).join(`\n`)}\n${tui.clr(TSGmeng.c_outer_unit_floor.repeat(wdata._h+2), `orange`)}`;
    };
    export function clr_lines(forl: number): void {
        process.stdout.write((`\r` + (` `.repeat(process.stdout.columns)) + `\n`).repeat(forl));
    };
    export function parseMapData(fname: string): WorldData {
        let fcontent = readFileSync(fname + `.gm`, `base64`);
        let fdata: BaseCfgTemplate<string> = JSON.parse(Plib.Packer.CharObjects.UnpackCharObject(fcontent));
        console.log(fdata.FILE_LIST);
        let wData = TSGmeng.wData(fdata.FILE_LIST.find(a=>a.name=="world.dat").content, fdata.FILE_LIST.find(a=>a.name=="player.dat").content)
        return wData;
    };
    export function wData(fcontent: string, pcontent: string): WorldData {
        let pdata = pcontent.split(`\n`); let fdata = fcontent.split(`\n`);
        if (process.argv.includes(`-d`)) console.table(pdata), console.table(fdata);
        let data: WorldData = {
            player: {
                c_ent_tag: `o`,
                colored: true, colorId: parseInt(pdata.find(a=>a.startsWith("colorId=")).substring("colorId=".length)),
                startDX: parseInt(pdata.find(a=>a.startsWith("startDX=")).substring("startDX=".length)),
                startDY: parseInt(pdata.find(a=>a.startsWith("startDY=")).substring("startDY=".length))
            },
            _h: parseInt(fdata.find(a=>a.startsWith("width=")).substring("width=".length)),
            _w: parseInt(fdata.find(a=>a.startsWith("height=")).substring("height=".length)),
            name: fdata.find(a=>a.startsWith("name=")).substring("name=".length),
            description: fdata.find(a=>a.startsWith(`description=`)).substring(`description=`.length)
        };
        return data;
    };
    export function render_unitmap(wdata: TSGmeng.WorldData, map: Array<TSGmeng.Unit>, selected_x: number, selected_y: number, selection_size_x: number = 1, selection_size_y: number = 1, selected_color: number = 0): string {
        let highlighted_unitIds = [];
        for (let i = 0; i < selection_size_x; i++) {
            for (let j = 0; j < selection_size_y; j++) {
                highlighted_unitIds.push({x: (selected_x+i), y: (selected_y+j)});
            };
        };
        let final = ``;
        map.map((unit: TSGmeng.Unit, indx: number) => {
            if (indx % wdata._h === 0) final += `\n`;
            if (process.argv.includes(`--logs`)) console.log(`RENDERING BLOCK:`, indx);
            let curY = Math.floor(indx/wdata._h); let curX = indx-(curY*wdata._h);
            if (process.argv.includes(`--logs`)) console.log(curX, curY, undefined != (highlighted_unitIds.find(u=>u.x==curX&&u.y==curY)));
            final += (unit.special ? `\x1b[4${unit.special_clr}m` : ``) + TSGmeng.colors[unit.color] + ( undefined != (highlighted_unitIds.find(u=>u.x==curX&&u.y==curY)) ? (`${TSGmeng.colors[selected_color]}+`) : (unit.special ? unit.special_c_unit : TSGmeng.c_unit)) + TSGmeng.resetcolor;
        });
        if (process.argv.includes(`--logs`)) console.log(highlighted_unitIds);
        return TSGmeng.add_border(wdata, final);
    };

    export function CreateMapData(wdata: TSGmeng.WorldData, WMAP_UNITS: Array<TSGmeng.Unit>): string {
        return WMAP_UNITS.map((val: TSGmeng.Unit, indx: number) => {
            let curY = Math.floor(indx/wdata._h); let curX = indx-(curY*wdata._h);
            console.log(curY, curX)
            return `${curX},${curY} ${val.color} ${val.collidable} ${val.is_entity} ${val.is_player} ${val.special} ${val.special_clr} ${val.special_c_unit}`;
        }).join(`\n`);
    };

    export function ReadMapData(wdata: TSGmeng.WorldData, mpd: string): Array<TSGmeng.Unit> {
        let ls = mpd.split(`\n`);
        let reswunits: Array<TSGmeng.Unit> = [];
        ls.forEach(j => {
            let unit = TSGmeng.StringToUnit(j);
            reswunits.push(unit.unit);
        });
        console.log(reswunits.length); tui.usleep(2000);
        return reswunits;
    };
    export function StringToUnit(data: string): {unit: TSGmeng.Unit, dX: number, dY: number} {
        let ls = data.split(` `);
        let unit: TSGmeng.Unit = {
            collidable: false, is_entity: false, transparent: false,
            color: 0, entity: null, special: false,
            is_player: false, player: null, special_clr: 0,
            special_c_unit: ''
        };
        let dX = 0, dY = 0;
        ls.forEach((j, i) => {
            if ( i == 0 ) { let dt = j.split(`,`); dX = parseInt(dt[0]); dY = parseInt(dt[1]); };
            if ( i == 1 ) { unit.color = parseInt(j); }
            if ( i == 2 ) { 
                if (j == "__gtransparent_unit__") return unit.transparent = true, unit.collidable = true;
                unit.collidable = stob(j) 
            }
            if ( i == 3 ) { unit.is_entity  = stob(j); }
            if ( i == 4 ) { unit.is_player  = stob(j); }
            if ( i == 5 ) { unit.special = stob(j); }
            if ( i == 6 ) { unit.special_clr = parseInt(j); }
            if ( i == 7 ) { unit.special_c_unit = (j); }
        });
        return {unit, dX, dY};
    };

    export function loadMap(wfile: string): TSGmeng.Launcher {
        if (!existsSync(wfile)) throw new RangeError(`${wfile} does not exist`);
        const MapFileDir = Plib.Dirs.DirHandler.CreateDirectoryObject(wfile);
        const player_data = MapFileDir.contents.FILE_LIST.find(f=>f.name==`player.dat`); // ~3kb
        const world_data = MapFileDir.contents.FILE_LIST.find(f=>f.name==`world.dat`);   // ~1kb
        const world_index = MapFileDir.contents.FILE_LIST.find(f=>f.name==`world.mpd`);  // ~370kb for around 4000 blocks
        writeFileSync(process.cwd() + `/world.mpd`, world_index.content);
        writeFileSync(process.cwd() + `/player.dat`, player_data.content);
        writeFileSync(process.cwd() + `/world.dat`, world_data.content);
        return new Launcher(mac_gmeng_path, wfile);
    };
    export class Launcher {
        private gmeng_dir: string; public plog: Writable; public env: string;
        constructor(gmeng_path: string, map_file: string) { this.env = map_file; this.gmeng_dir = gmeng_path; this.plog = new Writable({
            write(chunk, encoding, callback) {
                if (process.argv.includes(`-d`)) console.log(`[ts-gm0:logger]`, chunk.toString());
                callback();
            }
        })};
        private async plugin_event(name: string, ...params: any[]) {
            let dict_events = {
                'player_move': 'p_changepos',
                'command_ran': 'gm_command',
                'modifier_change': 'gm_modchange'
            }, ev_this = dict_events[name];
            this.Events.cast_event(ev_this, ...params);
        };
        private format_stdout_buffer(data: string): string {
            return data
                .replaceAll(`$!__GMENG_WMAP`, this.env)
                .replaceAll(`$!__VERSION`, `5.2`)
                .replaceAll(`$!__BUILD`, `5.2-stable.player:48b5c`)
        };
        public Events = new class EventHandler {
            private emitter: EventEmitter = new EventEmitter();
            /**
             * executed when the player moves
             */
            public async player_mv<__ehandle extends (
                    // TODO: Controller [ev_reverse, ev_movep, p_setpos, p_modify, ...]
                    pl_old: { dX: number, dY: number },
                    pl_new: { dX: number, dY: number }
                ) => any> (efunc: __ehandle): Promise<void> {
                this.emitter.on(`p_changepos`, efunc);
            };
            /**
             * executed when a modifier is changed in-game
             */
            public async modifier_change<__ehandle extends (
                    name: string,
                    old_value: number,
                    new_value: number
                ) => any> (efunc: __ehandle): Promise<void> {
                this.emitter.on(`gm_modchange`, efunc);
            };
            /**
             * executed when a command is run on dev-c
             */
            public async cmdget<__ehandle extends (
                command_str: string
            ) => any> (efunc: __ehandle): Promise<void> {
                this.emitter.on(`gm_command`, efunc);
            };
            public async cast_event(name: string, ...params: any[]): Promise<void> { this.emitter.emit(name, ...params) };
        };
        public gen_posXY(p1: string): Array<{dX: number, dY: number}> {
            let params = p1.split(`!:`);
            let obj1: Array<{dX: number, dY: number}> = [];
            params.forEach((j: string, i: number) => {
                // j = dX=num,dY=num
                if (i > 1) return;
                let _p1 = j.split(`,`); let _dX = parseInt(_p1[0].substring("dX=".length));
                let _dY = parseInt(_p1[1].substring("dY=".length)); obj1.push({dX: _dX, dY: _dY});
            });
            return obj1;
        };
        public async app_exit() {
                process.stdout.cursorTo(0, 50);
                process.stdout.write('\x1b[?25h'); //show cursor
                unlinkSync(process.cwd() + `/world.dat`);
                unlinkSync(process.cwd() + `/player.dat`);
                unlinkSync(process.cwd() + `/world.mpd`);
                process.exit(0);
        };
        public async launchGame(): Promise<void> {
            return new Promise<void>((resolve, reject) => {
                let proc__a = spawn(`${this.gmeng_dir}`, { stdio: ['pipe', 'pipe', 'pipe'] });
                process.stdout.write('\x1b[?25l'); // hide cursor
                proc__a.on(`exit`, () => { this.app_exit(), resolve(); });
                process.on(`exit`, () => { this.app_exit(), resolve(); });
                proc__a.stdout.on(`data`, (data: Buffer) => {
                    process.stdout.write(this.format_stdout_buffer(Buffer.from(data).toString()));
                });
                process.stdin.setRawMode(true);
                process.stdin.setEncoding('utf8');
                proc__a.stdin.pipe(process.stdin);
                emitKeypressEvents(process.stdin);
                let inmenu = false;
                proc__a.stdin.write(`[dev-c] r_update` + `\n`);
                process.stdin.setMaxListeners(0);
                process.stdout.setMaxListeners(0);
                proc__a.stdout.setMaxListeners(0);
                proc__a.stdin.setMaxListeners(0);
                proc__a.stderr.on(`data`, (_buf0: string) => {
                    let data = Buffer.from(_buf0).toString();
                    if (data.startsWith(`[gm0:core/__MOUSECLICK__]`)) {
                        let pos = tui.get_curXY(); tui.run_button(pos.column, pos.row, false);
                        return;
                    };
                    if (!data.startsWith(`[gm0:core/__EVCAST]`)) return console.error(data + ` [ts-gm0:err]`);
                    let gm0: string[] = data.split(` `).slice(1);
                    let gm0_id = parseInt(gm0[0]);
                    let gm0_nm = gm0[1];
                    let gm0_params = gm0.slice(2).join(` `);
                    let gm0_params_spl = gm0_params.split(`!:`);
                    gm0_params_spl.pop();
                    if (gm0_id != 8545) {
                                          this.plog.write(`${data} raw_data`);
                                          this.plog.write(`${data} plug_event of ${gm0_id}`);
                                          this.plog.write(gm0_params_spl.length + ` params for ${gm0_id} __evcast ([${gm0_params_spl.join(`, `)}])`);
                                          this.plog.write(gm0_id + ` -> event_id [__evcast/data:id]`)
                                          this.plog.write(gm0_nm + ` -> event_nm [__evcast/data:name]`)
                                        }
                         if (gm0_id == 8545) return this.plog.write(this.gen_posXY(gm0_params).join(`!:`) + ` ` + gm0_id + ` ` + gm0_nm), this.plugin_event(gm0_nm, ...this.gen_posXY(gm0_params));
                    else if (gm0_id == 8546) return this.plugin_event(gm0_nm, gm0_params);
                    else if (gm0_id == 8547) return this.plugin_event(gm0_nm, gm0_params_spl[0], parseInt(gm0_params_spl[1]), parseInt(gm0_params_spl[2]));
                });
                process.stdin.on(`keypress`, async (ch, e: Key) => {
                    if (e.sequence == `\x03`) return this.app_exit();
                    if (inmenu) return;
		            if (e.name == `f2` && e.shift) { proc__a.stdin.write(`[dev-c] r_update` + `\n`); return void 0; };
                    if (e.name == `escape`) {
                        inmenu = true;
                        let selection = await SHOW_MAINMENU();
                        inmenu = false;
                        switch (selection) {
                            case 0:
                                process.stdin.emit(`keypress`, ...[`\t`, {
                                    sequence: `\x1B[Z`,
                                    name: `tab`,
                                    ctrl: false,
                                    meta: false,
                                    shift: true,
                                    code: `[Z`
                                }]);
                                break;
                            case 1:
                                proc__a.stdin.write(`[dev-c] r_update` + `\n`);
                                break;
                            case 2:
                                proc__a.stdin.write(`[dev-c] gm_quit` + `\n`);
                                break;
                        };
                    };
                    if (e.name == `tab` && e.shift && !inmenu) {
                        process.stdout.cursorTo(0, 45);
                        inmenu = true;
                        let cmd = await SHOW_DEVC();
                        inmenu = false;
                        if (cmd == (void 0)) return proc__a.stdin.write(`[dev-c] r_update` + `\n`);
                        if (cmd == `gm_devmode 1`) return (!process.argv.includes(`-d`) ? process.argv.push(`-d`) : void 0), proc__a.stdin.write(`[dev-c] r_update` + `\n`);
                        if (cmd == `gm_devmode 0`) return (!process.argv.includes(`-d`) ? void 0 : process.argv.pop()), proc__a.stdin.write(`[dev-c] r_update` + `\n`);
                        process.stdout.cursorTo(0, process.stdout.rows-14);
                        proc__a.stdin.write(`[dev-c] ${cmd}` + `\n`);
                        if (cmd != "r_update") await tui.await_keypress();
                        proc__a.stdin.write(`[dev-c] r_update` + `\n`);
                        return;
                    }
		            // movement (sent as commands through std::cin)
                    // does not require r_update @since 2.1:
                    // WorldMap::rewrite_mapping() handles individual unit updates, so player movement
                    // can be disregarded as an update event, instead treated as a screen change event
                    // such as the dev-c ui being shown, however r_update will still recognize
                    // player movements after being executed since the displaymap still holds values for
                    // the player's position and coordinate data.
		            if (e.name == `w`) proc__a.stdin.write(`[posy] i1` + `\n`);
		            if (e.name == `a`) proc__a.stdin.write(`[posx] d1` + `\n`);
		            if (e.name == `s`) proc__a.stdin.write(`[posy] d1` + `\n`);
		            if (e.name == `d`) proc__a.stdin.write(`[posx] i1` + `\n`);
                });
            });
        };
    };
};


export default builder;


async function SHOW_MAINMENU(): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        let selection: number = 0, mm_length: number = 4;
        let in_focus = true;
        let buttons: Array<Array<string | number>> = [
            ["show dev-c console", 0],
            ["return to game", 1],
            ["quit game", 2],
        ];
        let r2 = tui.rgk.repeat(2);
        function draw_mm(sel: number = 0) {
            if (!in_focus) return;
            process.stdout.cursorTo(0,0); // go to 0,0
            process.stdout.write(`${tui.dwk.repeat(mm_length-2)}`);
            if (sel < 0) sel = buttons.length-1, selection = buttons.length-1;
            if (sel > buttons.length-1) sel = 0, selection = 0;
            process.stdout.write(`${r2}${tui.clr(TSGmeng.c_unit, `red`)}${tui.clr(TSGmeng.c_outer_unit_floor.repeat(14), `red`)}${chalk.bold.white.bgRedBright(`gmeng`)}${tui.clr(TSGmeng.c_outer_unit_floor.repeat(14), `red`)}${tui.clr(TSGmeng.c_unit, `red`)}\n`);
            buttons.forEach(jb => {
                let sl_line = `${r2}${tui.clr(TSGmeng.c_unit, "red")} ${jb[1] == selection ? tui.clr(`>`, `blue`) : tui.clr(`-`, `tan`)} ${tui.clr(jb[0].toString(), `green`)}${` `.repeat(30-jb[0].toString().length)}${tui.clr(TSGmeng.c_unit, `red`)}\n`;
                process.stdout.write(sl_line);
            });
            process.stdout.write(`${r2}${tui.clr(TSGmeng.c_unit, `red`)}${tui.clr(TSGmeng.c_outer_unit.repeat(33), `red`)}${tui.clr(TSGmeng.c_unit, `red`)}`);
        };
        draw_mm(selection);
        process.stdin.on(`keypress`, (ch: string, e: Key) => {
            if (!in_focus) return;
            if (e.name == `down`) selection++;
            else if (e.name == `up`) selection--;
            if (e.sequence == `\r`) {
                in_focus = false;
                return resolve(selection);
            };
            if (e.name == `escape`) return resolve(1);
            draw_mm(selection);
        });
    });
};

async function SHOW_DEVC(): Promise<string> {
    return new Promise((resolve, reject) => {
        process.stdout.write(tui.upk.repeat(7));
        let input = ``; let usable = true;
        let line1 = `Enter command to send (gm0/core->manager)`;
        let cursor = `⎽`;
        function draw_c() {
            process.stdout.write(tui.upk.repeat(3));
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit}${TSGmeng.c_outer_unit_floor.repeat(20)}${chalk.bgRgb(204, 36, 29)(tui.clr(`DEV-C`, `tan`))}${TSGmeng.c_outer_unit_floor.repeat(20)}${TSGmeng.c_unit}`, "dark_red") + `\n`);
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit}  ${chalk.underline(tui.clr(line1, `blue`))}${tui.clr(TSGmeng.c_unit.repeat(43-line1.length), `black`)}${TSGmeng.c_unit}`, "dark_red") + `\n`);
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit} ${tui.clr(`>`, `tan`)} ${tui.clr(input, `green`)}${tui.clr(cursor, `tan`)}${` `.repeat(41-input.length)}${TSGmeng.c_unit}`, "dark_red") + `\n`);
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit}${TSGmeng.c_outer_unit.repeat(45)}${TSGmeng.c_unit}`, "dark_red"));
        }
        draw_c();
        process.stdin.on(`data`, (key: string) => {
            if (!usable) return;
            if ([`\u001b[A`, `\u001b[B`, `\u001b[C`, `\u001b[D`, `\x1B[3~`].includes(key)) return;
            if (key == `\x1B`) return usable = false, resolve(void 0);
            if (key == `\x7F`) return (input.length > 0 ? input = input.slice(0, -1) : void 0), draw_c();
            if (key == `\r`) {
                let data = input; input = ``;
                resolve(data); return usable = false;
            };
            if (input.length == 41) return;
            input += key; draw_c();
        });
    });
};
