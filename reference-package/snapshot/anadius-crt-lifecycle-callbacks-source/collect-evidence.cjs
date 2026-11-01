'use strict';
const fs=require('fs'),path=require('path'),crypto=require('crypto'),assert=require('assert/strict');
const dir=__dirname,outputs=path.resolve(dir,'..'),target='C:/Users/Skysect/Downloads/anadius64.dll';
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),image=fs.readFileSync(target);
const digest='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';
assert.equal(sha(image),digest);
const asm=fs.readFileSync(path.join(outputs,'anadius-all-functions.asm.txt'));
const pseudo=fs.readFileSync(path.join(outputs,'anadius-all-functions.pseudo.c'));
assert.equal(sha(asm),'9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42');
assert.equal(sha(pseudo),'26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb');
const pe=image.readUInt32LE(0x3c),opt=pe+24,base=image.readBigUInt64LE(opt+24),n=image.readUInt16LE(pe+6),st=opt+image.readUInt16LE(pe+20);
assert.equal(image.readUInt16LE(opt),0x20b);
const sections=Array.from({length:n},(_,i)=>{const p=st+i*40;return {rva:image.readUInt32LE(p+12),raw:image.readUInt32LE(p+20),size:image.readUInt32LE(p+16)};});
const offset=(va,count)=>{const r=Number(BigInt(va)-base);for(const s of sections)if(r>=s.rva&&r-s.rva+count<=s.size)return s.raw+r-s.rva;throw Error('raw range');};
const relocationRva=image.readUInt32LE(opt+112+5*8),relocationSize=image.readUInt32LE(opt+112+5*8+4);
const relocationStart=offset(base+BigInt(relocationRva),relocationSize),fixups=new Map();
for(let p=relocationStart;p<relocationStart+relocationSize;){
 const page=image.readUInt32LE(p),size=image.readUInt32LE(p+4);assert(size>=8&&p+size<=relocationStart+relocationSize);
 for(let q=p+8;q<p+size;q+=2){const word=image.readUInt16LE(q);if(word>>>12===10)fixups.set(page+(word&4095),{block_file_offset:p,entry_file_offset:q,entry_word:word});}
 p+=size;
}
const metadata=[],selectedAsm=[],selectedPseudo=[];
for(const [address,length,index,kind] of [['0x18006447c',35,15,'initialize'],['0x1800644f8',64,13,'uninitialize']]){
 // Separate multiline boundary expression: end of line must not end the body.
 const find=(text,prefix)=>{const re=new RegExp('^'+prefix+' '+address+' [\\s\\S]*?(?=^'+prefix+' 0x[0-9a-f]+ |(?![\\s\\S]))','m');return text.match(re)?.[0];};
 const fullAsm=find(asm.toString(),';'),fullPseudo=find(pseudo.toString(),'/\\*');
 assert(fullAsm&&fullPseudo);selectedAsm.push(fullAsm);selectedPseudo.push(fullPseudo);
 const file=offset(address,length),bytes=image.subarray(file,file+length);
 const pairVa=0x18008aa60n+BigInt(index*16+(kind==='initialize'?0:8)),pairOffset=offset(pairVa,8);
 assert.equal(image.readBigUInt64LE(pairOffset),BigInt(address));
 const proof=fixups.get(Number(pairVa-base));assert(proof);
 metadata.push({address,length,file_offset:file,bytes_hex:bytes.toString('hex'),sha256:sha(bytes),
  assembly_evidence_id:fullAsm.match(/Evidence (ev_[0-9a-f]+)/)[1],pseudo_evidence_id:fullPseudo.match(/Evidence (ev_[0-9a-f]+)/)[1],
  pair:{index,kind,slot_va:'0x'+pairVa.toString(16),file_offset:pairOffset,relocation_proof:proof}});
}
assert(selectedAsm[0].includes('CALL 0x1800643f8'));
assert(selectedAsm[1].includes('XADD.LOCK')&&selectedAsm[1].includes('0x180064529: MOV'));
fs.writeFileSync(path.join(dir,'callbacks.selected.asm.txt'),selectedAsm.join('\n'));
fs.writeFileSync(path.join(dir,'callbacks.selected.pseudo.c'),selectedPseudo.join('\n'));
fs.writeFileSync(path.join(dir,'evidence.json'),JSON.stringify({result:'PASS',target_sha256:digest,method:'Retained actual REA assembly/pseudocode IDs and direct hash-bound raw PE/relocation proof. No new provider query or original execution.',bodies:metadata},null,2)+'\n');
console.log('PASS:2 callback bodies;99 raw bytes;2 DIR64 table bindings;retained REA identities');
