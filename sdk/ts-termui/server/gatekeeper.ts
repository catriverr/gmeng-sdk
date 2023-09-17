import dns, { WebDomain } from './load-dns.js';

export namespace Gatekeeper {
    export function find_door(domain: string): WebDomain { if (!dns.has(domain)) return undefined; else return dns.get(domain); };
};

export default Gatekeeper;