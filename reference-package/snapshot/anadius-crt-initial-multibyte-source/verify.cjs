'use strict';
const fs=require('node:fs'),cp=require('node:child_process'),crypto=require('node:crypto'),path=require('node:path');
const target='C:/Users/Skysect/Downloads/anadius64.dll',b=fs.readFileSync(target),sha=crypto.createHash('sha256').update(b).digest('hex');
if(sha!=='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c')throw Error('Target hash changed');
const compiled=JSON.parse(cp.execFileSync(process.argv[2],['--dump'],{encoding:'utf8'})),pe=b.readUInt32LE(60),opt=pe+24,base=Number(b.readBigUInt64LE(opt+24)),start=opt+b.readUInt16LE(pe+20),sections=[];
for(let i=0;i<b.readUInt16LE(pe+6);++i){const p=start+40*i;sections.push({va:base+b.readUInt32LE(p+12),size:b.readUInt32LE(p+16),file:b.readUInt32LE(p+20)});}
let checks=0;
for(const [name,va,n] of [['record',0x1800a1c70,0x228],['initial_type_table',0x1800a1ea0,257],['initial_case_table',0x1800a1fb0,256]]){const s=sections.find(s=>va>=s.va&&va+n<=s.va+s.size);if(!s)throw Error('Not raw backed');const raw=b.subarray(s.file+va-s.va,s.file+va-s.va+n),c=Buffer.from(compiled[name],'hex');if(c.length!==n)throw Error('compiled size mismatch');for(let i=0;i<n;++i){++checks;if(raw[i]!==c[i])throw Error(name+' differs at'+i);}}
const maskVA=0x1800a1c6c,maskSection=sections.find(s=>maskVA>=s.va&&maskVA+4<=s.va+s.size);if(!maskSection)throw Error('mask not raw backed');
if(compiled.locale_mask!==b.readUInt32LE(maskSection.file+maskVA-maskSection.va))throw Error('mask mismatch');++checks;
const result={result:'PASS',checks,scope:'Independent direct PE byte comparison against compiled own C++ fixture output; original DLL not executed',target_sha256:sha};fs.writeFileSync(path.join(__dirname,'byte-validation.json'),JSON.stringify(result,null,2)+'\n');console.log(JSON.stringify(result));
