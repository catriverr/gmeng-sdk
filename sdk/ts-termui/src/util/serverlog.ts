import chalk from "chalk";

export const SERVER_NAME = `${chalk.bold.red(`[${chalk.bold.yellow(`server`)}]`)}`;

export namespace svlog {
    export function puts(...params: any[]): void {
        console.log(SERVER_NAME, ...params);
    };
};

export default svlog;