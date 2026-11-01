'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const j=JSON.parse(fs.readFileSync(path.join(__dirname,'initial_locale.extraction.json'))),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
assert.equal(j.provenance,'local-file-derived; no new REA evidence IDs');
const restored=j.regions.map(r=>Buffer.from(r.normalized_hex,'hex')),seen=new Set();
for(const r of j.regions){assert.equal(r.size,restored[r.id].length);assert.equal(sha(restored[r.id]),r.normalized_sha256);assert.equal(sha(Buffer.from(r.raw_hex,'hex')),r.raw_sha256);}
for(const e of j.edges){
  const r=j.regions[e.source_region],t=j.regions[e.target_region],proof=e.relocation_proof,key=e.source_region+':'+e.source_offset;
  assert(!seen.has(key));seen.add(key);
  assert(e.source_offset>=0&&e.source_offset+8<=r.size&&e.target_offset>=0&&e.target_offset<t.size);
  assert.equal(restored[r.id].readBigUInt64LE(e.source_offset),0n);
  const target=BigInt(t.va)+BigInt(e.target_offset),slot=BigInt(r.va)+BigInt(e.source_offset);
  assert.equal(target,BigInt(e.target_va));assert.equal(slot,BigInt(e.slot_va));
  assert.equal(BigInt(j.input.image_base)+BigInt(proof.block_page_rva)+(BigInt(proof.entry_word)&4095n),slot);
  assert.equal(BigInt(proof.entry_word)>>12n,10n);
  assert.equal(Buffer.from(r.raw_hex,'hex').readBigUInt64LE(e.source_offset),target);
  restored[r.id].writeBigUInt64LE(target,e.source_offset);
}
for(const r of j.regions){assert.equal(restored[r.id].toString('hex'),r.raw_hex);if(r.backing.kind==='virtual-zero-fill'){assert.equal(r.backing.file_offset,null);assert(restored[r.id].every(x=>x===0));}}
assert.equal(j.regions.length,96);assert.equal(j.edges.length,116);
assert.deepEqual(j.edges.reduce((a,e)=>(a[e.source_region]=(a[e.source_region]||0)+1,a),{}),{0:11,1:18,2:87});
assert.equal(j.scalars.lconv_scalar_50.qword,'0x7f7f7f7f7f7f7f7f');
const report={result:'PASS',regions:96,relocations:116,bytes:j.regions.reduce((s,r)=>s+r.size,0),file_backed_regions:j.regions.filter(r=>r.backing.kind==='file').length,pe_zero_fill_regions:j.regions.filter(r=>r.backing.kind==='virtual-zero-fill').length,checks:['Region hashes','Every DIR64 proof reconstructs source slot','Every raw pointer resolves inside owned bounded graph','All normalized pointers zero','Applying evidence metadata recreates every original byte','11 locale/18 lconv/87 time edges','Zero-fill leaves expressly distinguished from file bytes'],original_execution:false};
fs.writeFileSync(path.join(__dirname,'extraction-validation.json'),JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report,null,2));
