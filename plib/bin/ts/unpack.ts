import { CharObjects } from "../../lib/packer.js";
let args = process.argv.slice(2);

console.log(JSON.parse(CharObjects.UnpackCharObject(args.join(` `))));