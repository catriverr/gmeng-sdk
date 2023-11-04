declare module 'packmgr-lib' {
    export namespace Packer {
        export namespace CharObjects {
            /**
             * Creates a bitmap of the provided file. <br>
             *
             * The file is split into characters with `[...text_contents]` and
             * mapped into the charcodes of each `c` joined with `x`.
             * __Example return value:__
             * ```
             * CreatePackedObject(`hello world!`); -> '104x101x108x108x111x32x119x111x114x108x100x33x' 
             * ```
             * @param file File to pack.
             * @returns {PackedObject} Packed instance of the file object.
             */
            export function CreatePackedObject(text: string): PackedObject;
            /**
             * Unpacks a CharObjectFile created with `Packer.CreatePackedObject(file: string)` into a normal String object.
             * __Example Usage:__
             * ```
             * UnpackCharObject(`104x101x108x108x111x32x119x111x114x108x100x33x`); -> `hello world!`
             * ```
             * @param text CharObject. Usually looks like `104x101x108x108x111x32x119x111x114x108x100x33x`
             * @returns
             */
            export function UnpackCharObject(text: string): string;
            /**
             * Packages a utf-8 encoded file into a CharObject encoded file.
             * @param __fn Name of the file to Package.
             * @param __uf Name of the file to pipe output. This file will be the output of this function.
             */
            export function ShipFile(__fn: string, __uf: string): void;
        }
        export class PackedObject {
            public _bitmap_length: number; public bitmap: string;
            public _bitmap_raw: string[];  public _bits:  string;
        }
    }
    export namespace Dirs {
        export namespace DirHandler {
            export function CreateDirectoryObject(file: string): Directory<string>;
        }
        export class Directory<FileList extends string> {
            public constructor(fname: string);
            public contents: BaseCfgTemplate<string>; public dirname: string;
            public import<Object>(file: FileList, encoding: BufferEncoding): Promise<Object>;
            public writeFile(file: string, content: string, encoding?: BufferEncoding): Promise<void>;
            public appendFile(file: string, content: string, encoding?: BufferEncoding): Promise<void>;
            public moveFile(path: string, npath: string): Promise<void>;
            public copyFile(path: string, npath: string): Promise<void>;
            public unlinkFile(path: string): Promise<void>;
            public importJSON<obj>(config_name: string): obj;
            public static BASE_CONFIG: BaseCfgTemplate<string>;
            private refresh_json(json: BaseCfgTemplate<string>): void;
            private rl_content(): void;
            private get_contents_json(): BaseCfgTemplate<string>;
            private write_json(json: any): void;
        }
        export interface BaseCfgTemplate<obj> { FILE_LIST: {name: obj, content: string}[] }
    }
}
