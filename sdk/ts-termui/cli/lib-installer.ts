import chalk from "chalk";
import http from "../src/load.js";
import tui, { anim } from "../utils/tui.js";


let animation = await anim.progress_bar(`hello there`);


let itnv = setInterval(function() {
    animation.change_text(Math.floor(animation.current_prog*2).toString() + `%`);
    animation.add_prog(0.25);
}, 50);

animation.onfinish(() => { clearInterval(itnv); });
