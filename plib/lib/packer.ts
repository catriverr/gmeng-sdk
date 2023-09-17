import fs from 'fs';

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
    export function CreatePackedObject(text: string): PackedObject {
        let fin: string = ``; 
        [...text].map(char=>{fin+=char.charCodeAt(0)+`x`;});
        if (fin.length % 2 != 0) fin += `x`;
        return {
            _bitmap_length: fin.length, _bitmap_raw: [...text],
            _bits: [...fin].map(c=>c.charCodeAt(0)).join(`x`), bitmap: fin
        };
    };

    /**
     * Unpacks a CharObjectFile created with `Packer.CreatePackedObject(file: string)` into a normal String object.
     * __Example Usage:__
     * ```
     * UnpackCharObject(`104x101x108x108x111x32x119x111x114x108x100x33x`); -> `hello world!`
     * ```
     * @param text CharObject. Usually looks like `104x101x108x108x111x32x119x111x114x108x100x33x`
     * @returns 
     */
    export function UnpackCharObject(text: string): string { 
        let fin: string = ``;
        text.split(`x`).map(char=>{fin+=String.fromCharCode(parseInt(char))}); 
        return fin.replaceAll(`\x00`, ``); 
    };

    /**
     * Packages a utf-8 encoded file into a CharObject encoded file.
     * @param __fn Name of the file to Package.
     * @param __uf Name of the file to pipe output. This file will be the output of this function.
     */
    export function ShipFile(__fn: string, __uf: string): void {
        let content = fs.readFileSync(__fn, `utf-8`);
        content = CharObjects.CreatePackedObject(content).bitmap;
        fs.writeFileSync(__uf, content, `base64`);
    }; 
};

export class PackedObject {
    public _bitmap_length: number; public bitmap: string;
    public _bitmap_raw: string[];  public _bits:  string;
};