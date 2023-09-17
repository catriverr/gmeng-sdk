import fs from 'fs';
import { DirHandler } from '../lib/dirhandle.js';


let myDir = DirHandler.CreateDirectoryObject("test.bin");

myDir.appendFile(`test.txt`, `Merhaba, dunya! 😋 lezizo`, `utf-8`);