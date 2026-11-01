'use strict';
// Read-only extractor. Only writes inside this script's output folder.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const dir = __dirname;
const sha = b => crypto.createHash('sha256').update(b).digest('hex');
const target = process.argv[2] || 'C:/Users/Skysect/Downloads/anadius64.dll';
const b = fs.readFileSync(target);
const expected = '9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';
if (sha(b) !== expected) throw Error('original SHA-256 mismatch');
if (b.readUInt16LE(0) !== 0x5a4d) throw Error('MZ');
const pe = b.readUInt32LE(0x3c);
if (b.readUInt32LE(pe) !== 0x4550) throw Error('PE');
const opt = pe + 24;
if (b.readUInt16LE(opt) !== 0x20b) throw Error('PE32+');
const base = Number(b.readBigUInt64LE(opt + 24));
const n = b.readUInt16LE(pe + 6), first = opt + b.readUInt16LE(pe + 20);
const sections = Array.from({length:n}, (_,i) => {
  const p = first + i*40;
  return {name:b.subarray(p,p+8).toString('ascii').replace(/\0.*$/,''),
    virtual_size:b.readUInt32LE(p+8),rva:b.readUInt32LE(p+12),
    raw_size:b.readUInt32LE(p+16),raw_offset:b.readUInt32LE(p+20)};
});
function offset(va, len) {
  const rva = va-base;
  const s = sections.find(s => rva>=s.rva && rva+len<=s.rva+s.raw_size);
  if (!s) throw Error('VA not fully raw-backed: '+va.toString(16));
  const p = s.raw_offset+rva-s.rva;
  if (p+len>b.length) throw Error('file range');
  return p;
}
const range = (va,len) => b.subarray(offset(va,len),offset(va,len)+len);
const hx = n => '0x'+n.toString(16);
const namesVA=0x18008ea10, lcidsVA=0x18008cfc0, rows=228, stride=16;
const namesRaw=range(namesVA,rows*stride), lcidsRaw=range(lcidsVA,rows*stride);
const names=Array.from({length:rows}, (_,i) => {
  const row=namesRaw.subarray(i*stride,(i+1)*stride);
  const va=Number(row.readBigUInt64LE(0)), index=row.readInt32LE(8);
  let bytes=0;
  for (;;) {
    if (bytes>=0x55*2) throw Error('name exceeds original comparison bound');
    if (range(va+bytes,2).readUInt16LE(0)===0) {bytes+=2;break;}
    bytes+=2;
  }
  const raw=range(va,bytes), name=raw.subarray(0,-2).toString('utf16le');
  if (!/^[\x20-\x7e]*$/.test(name)) throw Error('adapt literal escaping');
  if (!(index>=0&&index<rows)) throw Error('static index out of bounds');
  return {ordinal:i,row_va:hx(namesVA+i*stride),row_file_offset:offset(namesVA+i*stride,stride),
    row_hex:row.toString('hex'),name_address:hx(va),name_file_offset:offset(va,bytes),
    name_utf16le_hex:raw.toString('hex'),name_sha256:sha(raw),name,index,
    lcid:lcidsRaw.readUInt32LE(index*stride)};
});
const lcids=Array.from({length:rows}, (_,i) => ({ordinal:i,
  row_va:hx(lcidsVA+i*stride),row_file_offset:offset(lcidsVA+i*stride,stride),
  row_hex:lcidsRaw.subarray(i*stride,(i+1)*stride).toString('hex'),
  lcid:lcidsRaw.readUInt32LE(i*stride)}));
const ordering_inversions=[];
for (let i=1;i<rows;++i) if (names[i-1].name.toLowerCase()>names[i].name.toLowerCase()) ordering_inversions.push([i-1,i]);
const lookup = name => {
  let low=0,high=227;
  do {
    const mid=Math.trunc((low+high)/2), v=names[mid].name;
    if (name===v) return {row:mid,lcid:lcidsRaw.readUInt32LE(names[mid].index*16)};
    if (name<v) high=mid-1; else low=mid+1;
  } while (low<=high);
  return {row:null,lcid:0};
};
const lookup_results=names.map(r=>({ordinal:r.ordinal,name:r.name,...lookup(r.name)}));
const e={input:{path:target,size:b.length,sha256:sha(b),image_base:hx(base)},
  method:'PE32+ section raw-backed VA mapping, fixed 228 rows x16 bytes; strings individually bounded to 85 UTF16 code units including NUL',
  sections,names_table:{va:hx(namesVA),offset:offset(namesVA,namesRaw.length),bytes:namesRaw.length,sha256:sha(namesRaw)},
  lcids_table:{va:hx(lcidsVA),offset:offset(lcidsVA,lcidsRaw.length),bytes:lcidsRaw.length,sha256:sha(lcidsRaw)},
  names,lcids,ordering_inversions,lookup_results};
fs.writeFileSync(path.join(dir,'extracted-tables.json'),JSON.stringify(e,null,2)+'\n');
const rowsCpp=names.map(r => `    {u${JSON.stringify(r.name)}, ${r.index}, ${r.name_address}ULL},`).join('\n');
const lcidCpp=lcids.map(r => `    0x${r.lcid.toString(16)}U,`).join('\n');
const cpp=`// Generated static data: extract-tables.cjs; see extracted-tables.json.\n#include "locale_names.hpp"\n#include <array>\n\nnamespace anadius::crt_locale_names_recovered {\nnamespace {\nconstexpr std::array<NameRow,228> names{{\n${rowsCpp}\n}};\nconstexpr std::array<std::uint32_t,228> lcids{{\n${lcidCpp}\n}};\nconstexpr std::uint32_t fold(std::uint16_t c) {\n    return c >= 0x41U && c <= 0x5aU ? c + 0x20U : c;\n}\n}\n\nstd::int32_t ascii_wcsnicmp_0736d4(Address left, std::u16string_view right,\n                                 std::uint64_t count, Effects& effects) {\n    if (count == 0) return 0;\n    std::size_t index = 0;\n    for (;;) {\n        const auto a = fold(effects.read_u16(left));\n        left += 2;\n        const auto b = fold(static_cast<std::uint16_t>(right.data()[index]));\n        ++index;\n        const auto difference = static_cast<std::int32_t>(a) - static_cast<std::int32_t>(b);\n        if (difference != 0 || a == 0) return difference;\n        if (--count == 0) return difference;\n    }\n}\n\nstd::uint32_t locale_name_to_lcid_07310c(Address name, Effects& effects) {\n    if (name == 0) return 0;\n    std::int32_t low = 0, high = 0xe3;\n    do {\n        const auto middle = (high + low) / 2;\n        const auto& row = names[static_cast<std::size_t>(middle)];\n        const auto result = ascii_wcsnicmp_0736d4(name, row.name, 0x55, effects);\n        if (result == 0) {\n            const auto index = row.lcid_index;\n            if (index < 0 || index >= 0xe4) return 0;\n            return lcids[static_cast<std::size_t>(index)];\n        }\n        if (result < 0) high = middle - 1;\n        else low = middle + 1;\n    } while (low <= high);\n    return 0;\n}\n\nstd::span<const NameRow,228> shipped_name_rows() { return names; }\nstd::span<const std::uint32_t,228> shipped_lcid_values() { return lcids; }\n} // namespace anadius::crt_locale_names_recovered\n`;
fs.writeFileSync(path.join(dir,'locale_names.cpp'),cpp);
const evidenceDir=path.resolve(dir,'..');
const selections = {};
const inputs=[['anadius-all-functions.pseudo.c','26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb','^/\\* 0x'],
  ['anadius-all-functions.asm.txt','9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42','^; 0x']];
for (const [file,hash,start] of inputs) {
  const raw=fs.readFileSync(path.join(evidenceDir,file));
  if (sha(raw)!==hash) throw Error('retained artifact identity mismatch: '+file);
  const t=raw.toString('utf8');
  const selected=['18007310c','1800736d4'].map(a => {
    // An end-of-line $ is not an end-of-input sentinel with m enabled.
    const begin=new RegExp(start+a+' .*','m').exec(t);
    if (!begin) throw Error('missing function');
    const tail=t.slice(begin.index+begin[0].length);
    const next=new RegExp(start,'m').exec(tail);
    return t.slice(begin.index,next ? begin.index+begin[0].length+next.index : t.length);
  }).join('\n');
  selections[file.endsWith('.c')?'pseudocode':'assembly'] = selected;
  fs.writeFileSync(path.join(dir,file.endsWith('.c')?'selected.pseudo.c.txt':'selected.asm.txt'),selected);
}
fs.writeFileSync(path.join(dir,'selected-evidence.json'),JSON.stringify({
  target:'anadius64.dll',target_sha256:expected,
  source_pseudocode_sha256:inputs[0][1],source_assembly_sha256:inputs[1][1],
  provider:'Ghidra 12.1.4',REA:'6.5.0',
  mode:'Retained exact-digest static evidence; aligned closed session verified by root; no target execution',
  selected_functions:['0x18007310c','0x1800736d4'],...selections,
  tables:{names:e.names_table,lcids:e.lcids_table,complete_raw_records:'extracted-tables.json'},
  observed_static_ordering_inversions:ordering_inversions,
  statically_unreachable_name_rows:lookup_results.filter(r=>r.row===null),
  limits:['Original x64 ABI equality unproven; typed logical effects/view interface.',
    'Original static data mutation outside supported domain.',
    'Static data has valid 0..227 indices, so rejection branches are retained but unreachable without data mutation.',
    'No original DLL or native Windows locale API execution.']
},null,2)+'\n');
console.log(JSON.stringify({names:rows,names_table:e.names_table,lcids_table:e.lcids_table,
  first:names[0],last:names.at(-1),unique_indices:new Set(names.map(x=>x.index)).size,
  ordering_inversions,unreachable:lookup_results.filter(r=>r.row===null)},null,2));
