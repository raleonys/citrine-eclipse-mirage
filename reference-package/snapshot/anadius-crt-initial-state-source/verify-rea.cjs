'use strict';
const fs=require('fs'),path=require('path'),crypto=require('crypto');
const root=path.resolve(__dirname,'..'),target='C:/Users/Skysect/Downloads/anadius64.dll';
const digest='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';
const image=fs.readFileSync(target),sha=x=>crypto.createHash('sha256').update(x).digest('hex');
if(sha(image)!==digest)throw Error('Target digest');
const pe=image.readUInt32LE(0x3c),optional=pe+24,base=image.readBigUInt64LE(optional+24);
if(image.readUInt16LE(optional)!==0x20b)throw Error('PE32+');
const n=image.readUInt16LE(pe+6),st=optional+image.readUInt16LE(pe+20);
function raw(va,length){
 const rva=Number(BigInt(va)-base);
 for(let i=0;i<n;i++){const p=st+i*40,r=image.readUInt32LE(p+12),size=image.readUInt32LE(p+16),off=image.readUInt32LE(p+20);
  if(rva>=r&&rva-r+length<=size)return {offset:off+rva-r,bytes:image.subarray(off+rva-r,off+rva-r+length)};}
 throw Error('Raw PE range');
}
const evidencePath=path.join(root,'source-workspace/initial-data-rea-evidence.json');
const evidence=JSON.parse(fs.readFileSync(evidencePath,'utf8').replace(/^\uFEFF/,''));
const expected=[['0x1800a17e0',0x160],['0x1800a1c70',0x228],['0x18008ad20',0x2c0]];
const checks=evidence.reads.map((r,i)=>{
 if(r.status!=='fulfilled')throw Error('REA failed read');
 const e=r.value.structuredContent,v=e.normalized_result,[address,length]=expected[i];
 if(e.subject.digest.sha256!==digest||v.address!==address||!v.complete||v.requested_bytes!==length||v.returned_bytes!==length)throw Error('REA coverage/identity');
 const analyzed=Buffer.from(v.bytes_hex,'hex'),file=raw(address,length);
 if(!analyzed.equals(file.bytes))throw Error('REA/PE byte difference');
 return {address,length,file_offset:file.offset,evidence_id:e.evidence_id,sha256:sha(analyzed),matches_raw_file:true};
});
const result={result:'PASS',target_sha256:digest,evidence_file_sha256:sha(fs.readFileSync(evidencePath)),checks,scope:'Exact three freshly obtained Ghidra initialized-memory reads compared to hash-bound raw PE ranges. No target execution or writes.'};
fs.writeFileSync(path.join(__dirname,'rea-comparison.json'),JSON.stringify(result,null,2)+'\n');
console.log('PASS: 3 REA/PE ranges;1608 bytes exact');
