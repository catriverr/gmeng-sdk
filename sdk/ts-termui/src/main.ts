import cfg from '../prelude/readconfig.js';
import chalk from 'chalk';
import express from 'express';
import svlog from './util/serverlog.js';
import { createReadStream, createWriteStream, readFileSync, writeFileSync } from 'fs';
import Gatekeeper from '../server/gatekeeper.js';
import zlib from 'zlib';
import { Parse } from 'unzip';


const app = express(); app.use(express.json()); app.use(express.urlencoded()); 
let stats: ServerStatistics = JSON.parse(readFileSync('./bin/statistics.json', 'utf8'));
cfg.readHosts(`./bin/hosts`);
console.log(cfg.domains);


app.get(`/`, (req, res) => {
    res.redirect(`https://mybutton.org`);
});

app.get(`/redir/:domain_name`, (req, res) => {
    let domain_id = req.params.domain_name;
    if (!cfg.domains.has(domain_id)) return res.sendStatus(404); else if (cfg.domains.get(domain_id) == ``) return res.sendStatus(201);
    let info = cfg.domains.get(domain_id); console.log(info); return res.status(200).send(info);
});

app.get(`/gatekeeper/:ip`, (req, res) => {
    let file = Gatekeeper.find_door(`${req.params.ip}`);
    res.status(200).send({ certificate: file.config, script: readFileSync(file.scriptPath, 'utf-8') });
});

app.get(`/stats`, (req, res) => { return res.status(200).send(stats); });


app.listen(cfg.port, () => {
    stats.total_restarts++;
    svlog.puts(`listening on ${chalk.bold.magenta(cfg.port)}`);
    stats.total_domains = cfg.domains.size;
    setInterval(() => { writeFileSync(`./bin/statistics.json`, JSON.stringify(stats, null, 4)) }, 5*1000);
}).on(`request`, (req, res) => {
    stats.total_requests++; 
    svlog.puts(`acknowledged request from ${chalk.bold.magenta(req.headers[`x-real-ip`])} at ${chalk.bold.magenta(req.url)}`);
});




export interface ServerStatistics {
    total_requests: number; total_domains: number; total_restarts: number;
    popular_urls: { url: string, req_count: number }[]
};