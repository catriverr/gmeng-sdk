import { writeFileSync } from "fs";
import { CharObjects } from "../../lib/packer.js";
let args = process.argv.slice(2);

writeFileSync(`./__unpack.lmf`, JSON.stringify(JSON.parse(CharObjects.UnpackCharObject(args.join(` `))), null, 4));