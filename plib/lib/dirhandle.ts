import fs from 'fs'; import fsExstra from 'fs-extra';
import { CharObjects as Packer } from './packer.js';


export namespace DirHandler {
    export function CreateDirectoryObject(file: string): Directory<string> {
        var dir: Directory<string> = new Directory(file); dir.dirname = file; return dir;
    };
};

export class Directory<FileList extends string> {
    public contents: BaseCfgTemplate<string> = { FILE_LIST: [] }; public dirname: string = ``;

    public constructor(fname: string) { this.dirname = fname; this.rl_content() /* reload the content (loads the files if the package already exists) */ };

    public async import<Object>(file: FileList, encoding: BufferEncoding): Promise<Object> { 
        this.rl_content();
        if (!this.contents.FILE_LIST.find(a=>a?.name==file)) throw new RangeError(`File ${file} does not exist in packed object ${this.dirname}.`);
        let cid = crypto.randomUUID();
        if (!fs.existsSync(`.plib-import-cache`)) fs.mkdirSync(`.plib-import-cache`); 
        fsExstra.emptyDirSync(`.plib-import-cache`);
        let file_writable = file.replace(`/`, `__`);
        fs.writeFileSync(`.plib-import-cache/_${cid}_${file_writable}`, Buffer.from(this.contents.FILE_LIST.find(a=>a?.name==file).content, encoding).toString(`utf-8`));
        let dnow = Date.now();
        let imp: any = await import(`${process.cwd()}/.plib-import-cache/_${cid}_${file_writable}`); 
        return imp;
    };
    
    public readFile(file: string): string {
        this.rl_content();
        let json = this.get_contents_json();
        if (json.FILE_LIST.find(a=>a?.name==file) == null) throw new RangeError(`${file} does not exist in packed file ${this.dirname}.`);
        let data = json.FILE_LIST.find(a=>a?.name==file);
        return data.content;
    };
    public async writeFile(file: string, content: string, encoding: BufferEncoding = `utf-8`): Promise<void> {
        this.rl_content();
        let json = this.get_contents_json();
        if (!json.FILE_LIST.find(a=>a?.name == file)) json.FILE_LIST.push({name: file, content: content});
        else json.FILE_LIST[json.FILE_LIST.findIndex(a=>a?.name==file)].content = Buffer.from(content).toString(encoding);
        this.write_json(json);
        this.rl_content();
    };

    public async appendFile(file: string, content: string, encoding: BufferEncoding = `utf-8`): Promise<void> {
        this.rl_content();
        let json = this.get_contents_json();
        let cntnt = (json.FILE_LIST.find(c=>c?.name==file)?.content ?? '') + content;
        if (!json.FILE_LIST.find(a=>a?.name==file)) json.FILE_LIST.push({name: file, content: Buffer.from(cntnt).toString(encoding)});
        json.FILE_LIST[json.FILE_LIST.findIndex(a=>a?.name==file)].content = Buffer.from(cntnt).toString(encoding);
        this.write_json(json);
        this.rl_content();
    };

    public async moveFile(path: string, npath: string) {
        this.rl_content();
        let json = this.get_contents_json();
        if (json.FILE_LIST.find(a=>a?.name==path) == null) throw new RangeError(`${path} does not exist in packed file ${this.dirname}.`);
        json.FILE_LIST[json.FILE_LIST.findIndex(a=>a?.name==path)].name = npath;
        this.write_json(json);
        this.rl_content();
    };

    public async copyFile(path: string, npath: string): Promise<void> {
        this.rl_content();
        let json = this.get_contents_json();
        if (json.FILE_LIST.find(a=>a?.name==path) == null) throw new RangeError(`${path} does not exist in packed file ${this.dirname}.`);
        let data = json.FILE_LIST[json.FILE_LIST.findIndex(a=>a?.name==path)];
        json.FILE_LIST.push({name: npath, content: data.content});
        this.write_json(json);
        this.rl_content();
    };

    public async unlinkFile(path: string): Promise<void> {
        this.rl_content();
        let json = this.get_contents_json();
        if (json.FILE_LIST.find(a=>a?.name==path) == null) throw new RangeError(`${path} does not exist in packed file ${this.dirname}.`);
        delete json.FILE_LIST[json.FILE_LIST.findIndex(a=>a?.name==path)];
        this.write_json(json);
        this.rl_content();
    };

    public importJSON<obj>(config_name: string): obj { return undefined; };
    public __raw_json(): any { return this.get_contents_json() };

    private refresh_json(json: BaseCfgTemplate<string>): void {
        let nflist = [];
        json.FILE_LIST.forEach(lndx => {
            if (lndx == null) return;
            nflist.push(lndx);
        });
        json.FILE_LIST = nflist;
    };
    private rl_content(): void {
        let json = this.get_contents_json();
        this.refresh_json(json);
        this.contents = json;
    };

    private get_contents_json(): BaseCfgTemplate<string> {
        if (!fs.existsSync(this.dirname)) fs.writeFileSync(this.dirname, Packer.CreatePackedObject(JSON.stringify(Directory.BASE_CONFIG)).bitmap, `base64`);
        let content = Packer.UnpackCharObject(fs.readFileSync(this.dirname, `base64`));
        return JSON.parse(content);
    };

    private write_json(json: any) {
        let wcontent = Packer.CreatePackedObject(JSON.stringify(json));
        fs.writeFileSync(this.dirname, wcontent.bitmap, `base64`);
    };

    public static BASE_CONFIG: BaseCfgTemplate<string> = { FILE_LIST: [] };
};

export interface BaseCfgTemplate<obj> { FILE_LIST: {name: obj, content: string}[] }