// Reads retained pseudocode as data. Does not evaluate it or load any DLL.
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const root=__dirname;
const evidencePath=path.resolve(root,'../anadius-all-functions.pseudo.c');
const raw=fs.readFileSync(evidencePath);
const text=raw.toString('utf8');
const whole=text.slice(text.indexOf('/* 0x180032d00 '),text.indexOf('/* 0x180033d70 '));
const begin=whole.indexOf('local_7b8 = 0;');
const end=whole.indexOf('DAT_1800a39c8 = (byte *)0x0;');
if(begin<0||end<begin)throw Error('Retained registry initializer boundary missing');
const initializer=whole.slice(begin,end);
const marker=/local_[0-9a-f]+ = (?:FUN_180|&LAB_180)([0-9a-f]+);/g;
let previous=0;
const recovered=[];
for(const match of initializer.matchAll(marker)){
  const group=initializer.slice(previous,match.index);
  previous=match.index+match[0].length;
  const literal=group.match(/"([^"\\]*)"/);
  let name;
  if(literal)name=literal[1];
  else {
    const parts=[...group.matchAll(/(?:local_|uStack_)[0-9a-f]+ = 0x([0-9a-f]{3,16});/g)].map(x=>{
      const buffer=Buffer.alloc(8);buffer.writeBigUInt64LE(BigInt('0x'+x[1]));return buffer;
    });
    const bytes=Buffer.concat(parts);
    const nul=bytes.indexOf(0);
    name=bytes.subarray(0,nul<0?bytes.length:nul).toString('ascii');
  }
  if(!name)throw Error('Could not decode registry entry');
  recovered.push({name,rva:'0x'+match[1].padStart(6,'0')});
}
const cpp=fs.readFileSync(path.join(root,'messages.cpp'),'utf8');
const source=[...cpp.matchAll(/\{"([A-Za-z]+)",0x([0-9a-f]+)\}/g)]
  .map(x=>({name:x[1],rva:'0x'+x[2].padStart(6,'0')}));
if(recovered.length!==39||source.length!==39)throw Error('Registry entry count mismatch');
if(JSON.stringify(recovered)!==JSON.stringify(source))
  throw Error('Recovered/source registry mismatch: '+JSON.stringify(recovered.flatMap((entry,i)=>
    JSON.stringify(entry)===JSON.stringify(source[i])?[]:[{retained:entry,source:source[i]}])));
if(new Set(source.map(x=>x.name)).size!==39)throw Error('Duplicate registry name');
const result={retained_pseudocode_sha256:crypto.createHash('sha256').update(raw).digest('hex'),
  procedure:'0x180032d00',pseudocode_evidence:'ev_4657037962313465cce62f6579fa74838943f7f547b0d739a8b152611c24c36b',
  result:'PASS',count:39,source_matches_retained_initializer:true,entries:recovered,
  coverage:'Registry name/RVA only. Parsed static retained pseudocode as data; handler bodies and MSVC initialization ABI not validated.'};
fs.writeFileSync(path.join(root,'registry-validation.json'),JSON.stringify(result,null,2)+'\n');
console.log('PASS: 39 registry names/RVAs match retained initializer');
