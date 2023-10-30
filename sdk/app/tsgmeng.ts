import chalk from "chalk";
import tui from "../ts-termui/utils/tui.js";
import * as Plib from '../../plib/src/index.js';
import { BaseCfgTemplate, Directory } from "../../plib/lib/dirhandle.js";
import path, { dirname } from 'path';
import { fileURLToPath } from 'url';
import {existsSync, readFileSync, truncate, unlinkSync, writeFileSync } from "fs";
import { ChildProcess, spawn } from "child_process";
import { Key, emitKeypressEvents } from "readline";
import EventEmitter from "events";
import { Writable } from "stream";

const __dirname = dirname(fileURLToPath(import.meta.url));
const mac_gmeng_path = path.join(__dirname, `..`, `..`, `lib`, `out`) + `/gmeng.out`;

export namespace builder {
    export async function mainMenu(args: string[]): Promise<void> {
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
        WorldData.name = await tui.show_input_screen(`enter WorldMap filename`, null, false);
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
    export async function show_editor(worldData: TSGmeng.WorldData, mapfile: Directory<string>) {
        let WMAP_UNITS: Array<TSGmeng.Unit> = [];
        if (mapfile.contents.FILE_LIST.find(a=>a.name==`world.mpd`) != null) WMAP_UNITS = TSGmeng.ReadMapData(worldData, mapfile.contents.FILE_LIST.find(a=>a.name==`world.mpd`).content);
        else for (let i = 0; i < worldData._w * worldData._h; i++) {
            WMAP_UNITS.push({
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
            color: SELECTCLR, entity: null, is_entity: false, is_player: false,
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
                            color: SELECTCLR, entity: null, is_entity: false, is_player: false,
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

export namespace TSGmeng {
    export const colors: Array<string> = [
        "\x1B[39m", "\x1B[34m", "\x1B[32m", "\x1B[36m", "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m"
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
    export interface Unit {
        color: number; collidable: boolean;
        is_entity: boolean; is_player: boolean;
        special: boolean; special_clr: number;
        entity: TSGmeng.Entity; player: TSGmeng.Player;
        special_c_unit: string;
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
            collidable: false, is_entity: false,
            color: 0, entity: null, special: false,
            is_player: false, player: null, special_clr: 0,
            special_c_unit: '' 
        };
        let dX = 0, dY = 0;
        ls.forEach((j, i) => {
            if ( i == 0 ) { let dt = j.split(`,`); dX = parseInt(dt[0]); dY = parseInt(dt[1]); }; 
            if ( i == 1 ) { unit.color = parseInt(j); }
            if ( i == 2 ) { unit.collidable = stob(j) }
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
                process.stdout.write('\x1b[?25h'); //show cursor
                unlinkSync(process.cwd() + `/world.dat`);
                unlinkSync(process.cwd() + `/player.dat`);
                unlinkSync(process.cwd() + `/world.mpd`);
                process.exit(0);
        };
        public async launchGame(): Promise<void> {
            return new Promise<void>((resolve, reject) => {
                let proc__a = spawn(`${this.gmeng_dir}`, { stdio: ['pipe', 'pipe', 'pipe'] });
                process.stdout.write('\x1b[?25l');
                proc__a.on(`exit`, () => { this.app_exit(), resolve(); });
                process.on(`exit`, () => { this.app_exit(), resolve(); });
                proc__a.stdout.on(`data`, (data: Buffer) => {
                    process.stdout.write(Buffer.from(data).toString().replaceAll(`$!__GMENG_WMAP`, this.env));
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
                    if (e.sequence == `\x03`) return process.exit(0); 
                    if (inmenu) return;
		            if (e.name == `f2` && e.shift) { proc__a.stdin.write(`[dev-c] r_update` + `\n`); return void 0; };
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



async function SHOW_DEVC(): Promise<string> {
    return new Promise((resolve, reject) => {
        process.stdout.write(tui.upk.repeat(7));
        let input = ``; let usable = true;
        let line1 = `Enter command to send (gm0/core->manager)`
        function draw_c() {
            process.stdout.write(tui.upk.repeat(3));
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit}${TSGmeng.c_outer_unit_floor.repeat(20)}${chalk.bgRgb(204, 36, 29)(tui.clr(`DEV-C`, `tan`))}${TSGmeng.c_outer_unit_floor.repeat(20)}${TSGmeng.c_unit}`, "dark_red") + `\n`);
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit}  ${chalk.underline(tui.clr(line1, `blue`))}${tui.clr(TSGmeng.c_unit.repeat(43-line1.length), `black`)}${TSGmeng.c_unit}`, "dark_red") + `\n`);
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit} ${tui.clr(`>`, `tan`)} ${tui.clr(input, `green`)}${` `.repeat(42-input.length)}${TSGmeng.c_unit}`, "dark_red") + `\n`);
            process.stdout.write(tui.rgk.repeat(2) + tui.clr(`${TSGmeng.c_unit}${TSGmeng.c_outer_unit.repeat(45)}${TSGmeng.c_unit}`, "dark_red"));
        }
        draw_c();
        process.stdin.on(`data`, (key: string) => { 
            if (!usable) return; 
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
