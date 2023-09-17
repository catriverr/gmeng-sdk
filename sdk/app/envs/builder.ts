import chalk from "chalk";
import tui from "../../ts-termui/utils/tui.js";
import * as Plib from '../../../plib/src/index.js';
import { Directory } from "../../../plib/lib/dirhandle.js";
import path, { dirname } from 'path';
import { fileURLToPath } from 'url';
import { existsSync, unlinkSync, writeFileSync } from "fs";
import { ChildProcess, spawn } from "child_process";

const __dirname = dirname(fileURLToPath(import.meta.url));
const mac_gmeng_path = path.join(__dirname, `..`, `..`, `..`, `lib`, `out`) + `/gmeng.out`;

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
        WorldData.description = await tui.show_input_screen(`enter world description`, null, false);
        await tui.show_input_screen(`enter world size in blocks (example: 2x2)`, null, false).then(a => {
            let wsze = a.split(`x`);
            if (wsze.length < 2) return console.log(`bad argument: ${a}`), process.exit(0);
            WorldData._w = parseInt(wsze[1]); WorldData._h = parseInt(wsze[0]);
        });
        WorldData.player.colorId = parseInt(await tui.show_input_screen(`enter player colorId (0-7)`, null, false));
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
        for (let i = 0; i < worldData._w * worldData._h; i++) {
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
        // ? TODO: if world.mpd exists in wordData.name.gm, use that instead
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
        return new Launcher(mac_gmeng_path);
    };
    
    export class Launcher {
        private gmeng_dir: string;
        constructor(gmeng_path: string) { this.gmeng_dir = gmeng_path; };
        public async launchGame(): Promise<void> {
            return new Promise<void>((resolve, reject) => {
                let proc__a = spawn(`${this.gmeng_dir}`)
                proc__a.stdout.pipe(process.stdout);
                proc__a.stderr.pipe(process.stderr);
                proc__a.stdin.pipe(process.stdin);
                proc__a.on(`exit`, () => {
                    unlinkSync(process.cwd() + `/world.dat`);
                    unlinkSync(process.cwd() + `/player.dat`);
                    unlinkSync(process.cwd() + `/world.mpd`);
                });
            });
        };
    };
};


export default builder;