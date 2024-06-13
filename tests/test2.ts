import { TSGmeng as tsgmeng, builder as builder, TSGmeng } from './sdk/app/tsgmeng.js';

console.log(TSGmeng.bgcolors.map((color, indx) => `${color}  ${indx}  ${TSGmeng.resetcolor}`).join(`\t`));
