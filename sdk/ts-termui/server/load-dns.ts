import fs from 'fs';
import chalk from 'chalk';
import { readIniFileSync } from 'read-ini-file';


export interface DomainConfiguration {
    info: DomainInfo; host: DomainHostInfo;
};

export interface DomainInfo {
    server_at: string; port: number; domain_name: string; 
    location: string; cache_key: string;
};

export interface DomainHostInfo {
    ip: string; domain_holder: string;
};

export class WebDomain {
    public config: DomainConfiguration; constructor(conf: DomainConfiguration) { this.config = conf; };
    get scriptPath(): string { return `./server/domains/${this.config.info.location}/${this.config.info.server_at}` };
};

export let DomainServer = fs.readdirSync(`./server/domains`, { withFileTypes: true });
export let DOMAIN_MAP: Map<string, WebDomain> = new Map;

DomainServer.forEach((v, i) => {
    let contents = fs.readdirSync(`./server/domains/${v.name}`, { withFileTypes: true });
    let site_conf_path = contents.find(f => f.name === `site.conf`).name;
    let site_conf_load: any = readIniFileSync(`./server/domains/${v.name}/${site_conf_path}`), site_conf: DomainConfiguration = site_conf_load;
    DOMAIN_MAP.set(site_conf.host.ip, new WebDomain(site_conf));
});

export default DOMAIN_MAP;