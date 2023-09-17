import { DomainConfiguration } from "../server/load-dns.js";
import fs from "fs";
import path from "path";
import readIniFile from "read-ini-file";


export let confs = fs.existsSync(`./_dns.config.ini`) ? `./_dns_config.ini` : path.join(`./`, `container`) + `/base_config.ini`;

export interface DomainConfigurationLike {
    "info.server_at": string; "info.domain_name": string; "host.ip": string;
    "info.port": number; "info.ip": string; "info.location": string;
};

export class SRVRCFG {
    public domain_holder: string; public cache_key: string; public webserver: DomainConfigurationLike; 
};

export let _cfg_read: any = readIniFile.readIniFileSync(confs);
export let _cfg: SRVRCFG = _cfg_read;

export namespace cfg { export let WebServerData: DomainConfigurationLike = _cfg.webserver; export let KeyholderData: { cache_key: string; domain_holder: string; } = { domain_holder: _cfg.domain_holder, cache_key: _cfg.cache_key }; };


export default cfg; 
