'use strict';
const fs=require('fs'),path=require('path'),crypto=require('crypto'),assert=require('assert/strict');
const dir=__dirname,out=path.resolve(dir,'..'),target='C:/Users/Skysect/Downloads/anadius64.dll';
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),image=fs.readFileSync(target);
const digest='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';
assert.equal(sha(image),digest);
const asm=fs.readFileSync(path.join(out,'anadius-all-functions.asm.txt'));
const pseudo=fs.readFileSync(path.join(out,'anadius-all-functions.pseudo.c'));
assert.equal(sha(asm),'9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42');
assert.equal(sha(pseudo),'26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb');
const pe=image.readUInt32LE(0x3c),opt=pe+24,base=image.readBigUInt64LE(opt+24),n=image.readUInt16LE(pe+6),st=opt+image.readUInt16LE(pe+20);
assert.equal(image.readUInt16LE(opt),0x20b);
function offset(va,count){const r=Number(BigInt(va)-base);for(let i=0;i<n;i++){
 const q=st+i*40,rva=image.readUInt32LE(q+12),size=image.readUInt32LE(q+16),off=image.readUInt32LE(q+20);
 if(r>=rva&&r-rva+count<=size)return off+r-rva;
}throw Error('raw range');}
function find(text,prefix,address){return text.match(new RegExp('^'+prefix+' '+address+' [\\s\\S]*?(?=^'+prefix+' 0x[0-9a-f]+ |(?![\\s\\S]))','m'))?.[0];}
const bodies=[],aout=[],pout=[];
for(const [address,end,role] of [['0x180067890','0x1800678ca','initialize'],['0x1800678cc','0x18006790b','uninitialize'],['0x180073258','0x1800732a7','array-destructor']]){
 const a=find(asm.toString(),';',address),p=find(pseudo.toString(),'/\\*',address);assert(a&&p);
 const last=[...a.matchAll(/^0x([0-9a-f]+):/gm)].at(-1);assert.equal('0x'+last[1],end);
 const count=Number(BigInt(end)-BigInt(address)+1n),file=offset(address,count),raw=image.subarray(file,file+count);
 aout.push(a);pout.push(p);
 const b={address,end,length:count,file_offset:file,sha256:sha(raw),bytes_hex:raw.toString('hex'),assembly_evidence_id:a.match(/Evidence (ev_[0-9a-f]+)/)[1],pseudo_evidence_id:p.match(/Evidence (ev_[0-9a-f]+)/)[1],role};
 if(role!=='array-destructor'){
  const slot=0x18008aa60n+8n*16n+(role==='uninitialize'?8n:0n),po=offset(slot,8);
  assert.equal(image.readBigUInt64LE(po),BigInt(address));b.pair={index:8,slot_va:'0x'+slot.toString(16),file_offset:po};
 }
 bodies.push(b);
}
assert(aout[0].includes('TEST EAX, EAX')&&aout[0].includes('CALL 0x1800732a8'));
assert(aout[1].includes('CMP RBX, 0x400')&&aout[1].includes('CALL 0x180073258'));
assert(aout[2].includes('LEA RSI, [RCX + 0x1200]')&&aout[2].includes('ADD RDI, 0x48'));
fs.writeFileSync(path.join(dir,'lowio.selected.asm.txt'),aout.join('\n'));
fs.writeFileSync(path.join(dir,'lowio.selected.pseudo.c'),pout.join('\n'));
fs.writeFileSync(path.join(dir,'evidence.json'),JSON.stringify({result:'PASS',target_sha256:digest,method:'Hash-bound raw PE and retained exact REA assembly/pseudocode identities;no original execution;full implementation of lower handle providers excluded',bodies,lower_provider_addresses:['0x1800732a8','0x180067698','0x180067788'],layout:{table_slots:128,table_extent:1024,array_records:64,record_stride:72,array_extent:4608,capacity_reset:false}},null,2)+'\n');
const evidencePath=path.join(out,'source-workspace/pair-leaves-rea-evidence.json');
const e=JSON.parse(fs.readFileSync(evidencePath,'utf8'));
const checks=e.reads.map((r,i)=>{
 assert.equal(r.status,'fulfilled');const ev=r.value.structuredContent,v=ev.normalized_result;
 const [address,count]=[['0x1800704e8',11],['0x1800644f0',7]][i];
 assert.equal(ev.subject.digest.sha256,digest);assert.equal(v.address,address);
 assert.equal(v.complete,true);assert.equal(v.returned_bytes,count);assert.equal(v.requested_bytes,count);
 const bytes=image.subarray(offset(address,count),offset(address,count)+count);
 assert(bytes.equals(Buffer.from(v.bytes_hex,'hex')));
 return {address,length:count,file_offset:offset(address,count),sha256:sha(bytes),evidence_id:ev.evidence_id,matches_raw_file:true};
});
fs.writeFileSync(path.join(out,'source-workspace/pair-leaves-rea-comparison.json'),JSON.stringify({result:'PASS',target_sha256:digest,evidence_file_sha256:sha(fs.readFileSync(evidencePath)),checks},null,2)+'\n');
console.log('PASS:3 lowio bodies/203 raw bytes;2 retained pair8 bindings;2 fresh REA leaf ranges/18 raw bytes');
