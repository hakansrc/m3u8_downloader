const M3U8FileParser = require('m3u8-file-parser');
const fs = require('fs');
const content = fs.readFileSync('master.m3u8', { encoding: 'utf-8' });

const reader = new M3U8FileParser();
reader.read(content);
console.log(JSON.stringify(reader.getResult(), null, 2));