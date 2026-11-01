'use strict';
const fs=require('fs'),path=require('path'),crypto=require('crypto'),cp=require('child_process'),assert=require('assert/strict');
const dir=__dirname,target='C:/Users/Skysect/Downloads/anadius64.dll';
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),digest='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';
const image=fs.readFileSync(target);assert.equal(sha(image),digest);
const pe=image.readUInt32LE(0x3c),optional=pe+24,base=image.readBigUInt64LE(optional+24),n=image.readUInt16LE(pe+6),table=optional+image.readUInt16LE(pe+20);
assert.equal(image.readUInt16LE(optional),0x20b);
function offset(va,count) {
 const rva=Number(BigInt(va)-base);
 for(let i=0;i<n;i++){const q=table+i*40,r=image.readUInt32LE(q+12),size=image.readUInt32LE(q+16),off=image.readUInt32LE(q+20);
 if(rva>=r&&rva-r+count<=size)return off+rva-r;}
 throw Error('Missing raw range');
}
const va=0x180064468n,file=offset(va,20),bytes=image.subarray(file,file+20);
const expected=Buffer.from('488d0571d303004889056a0e0400b001c3','hex');
assert(bytes.subarray(0,17).equals(expected));
assert.equal(va+7n+BigInt(bytes.readInt32LE(3)),0x1800a17e0n);
assert.equal(va+14n+BigInt(bytes.readInt32LE(10)),0x1800a52e0n);
const e=JSON.parse(fs.readFileSync(path.join(dir,'rea-evidence.json'),'utf8')).read.structuredContent;
assert.equal(e.subject.digest.sha256,digest);
assert.equal(e.normalized_result.address,'0x180064468');assert.equal(e.normalized_result.complete,true);
assert.equal(e.normalized_result.returned_bytes,20);
assert(bytes.equals(Buffer.from(e.normalized_result.bytes_hex,'hex')));
const objdump=path.resolve(dir,'../../work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64/bin/llvm-objdump.exe');
const result=cp.spawnSync(objdump,['-d','--start-address=0x180064468','--stop-address=0x180064479',target],{encoding:'utf8'});
if(result.status!==0)throw Error(result.stderr);
fs.writeFileSync(path.join(dir,'initializer.selected.asm.txt'),result.stdout);
assert(/lea[\s\S]*1800a17e0/i.test(result.stdout));assert(/mov[\s\S]*1800a52e0/i.test(result.stdout));
const pairs=JSON.parse(fs.readFileSync(path.resolve(dir,'../anadius-crt-initializers-source/initializer-tables.json'),'utf8').replace(/^\uFEFF/,'')).tables.find(t=>t.name==='acrt_initializer_pairs');
assert.equal(pairs.entries[0].initialize.va,'0x180064468');
assert.equal(pairs.entries[0].uninitialize.va,'0x0');
const report={result:'PASS',target_sha256:digest,evidence_id:e.evidence_id,read_address:'0x180064468',read_length:20,body_length:17,file_offset:file,body_hex:expected.toString('hex'),body_sha256:sha(expected),read_sha256:sha(bytes),lea_target:'0x1800a17e0',mov_target:'0x1800a52e0',boolean_result:true,initial_ref_increment:false,initializer_pair_index:0,objdump_sha256:sha(fs.readFileSync(objdump)),assembly_sha256:sha(Buffer.from(result.stdout)),limitations:['Function body is local static decode of a table-referenced 17-byte leaf; Ghidra inventory omitted this function. REA Evidence establishes bytes, not new Ghidra function classification.','Return recovered as boolean low AL only; upper register bits unspecified.','No original target execution, writes, or ABI/SEH equivalence test.']};
fs.writeFileSync(path.join(dir,'initializer-byte-proof.json'),JSON.stringify(report,null,2)+'\n');
console.log('PASS:17 initializer bytes/rawPE/REA;RIP-relative targets;first table identity;static LLVM decode');
