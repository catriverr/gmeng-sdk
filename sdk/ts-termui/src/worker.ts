import chalk from "chalk";

async function main() {
    let argv = process.argv.slice(2);
    if (argv.includes(`lib-install`)) return await import(`../cli/lib-installer.js`); 
    if ((process.stdout.columns < 181 || process.stdout.columns > 181) || process.stdout.rows < 13) console.log(chalk.bold.red(`dnscli TUI is not available on ${chalk.yellow(process.stdout.columns + `x` + process.stdout.rows)} resolution. Fine-tuned requirement is ${chalk.yellow(`181x13`)}.`)), process.exit(1);
    
    process.stdout.write(`\u001B[?25l`); // hide cursor
    
    process.on(`SIGINT`, () => { process.stdout.write(`\u001B[?25h`); process.exit(0); }); // return cursor
    process.on(`beforeExit`, () => { process.stdout.write(`\u001B[?25h`); }); // return cursor
    
    if (argv.includes(`-s`) || argv.includes(`--server`)) return await import(`../prelude/start.js`); else { await import(`../cli/start.js`) };
};

main();