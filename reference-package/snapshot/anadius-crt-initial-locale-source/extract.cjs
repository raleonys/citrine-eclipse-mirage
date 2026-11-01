'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const dir=__dirname,target=process.argv.slice(2).find(x=>!x.startsWith('--'))||'C:/Users/Skysect/Downloads/anadius64.dll';
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),hx=n=>'0x'+n.toString(16);
const b=fs.readFileSync(target),expected='9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c';
if(sha(b)!==expected)throw Error('Target identity mismatch');
const retainedPath=path.join(dir,'..','anadius-all-functions.pseudo.c'),retained=fs.readFileSync(retainedPath),retainedSha='26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb';
if(sha(retained)!==retainedSha)throw Error('Retained pseudocode identity mismatch');
const retainedText=retained.toString('utf8'),start=retainedText.indexOf('/* 0x180062a80 FUN_180062a80;'),end=retainedText.indexOf('/* 0x1800630e4 FUN_1800630e4;',start);
if(start<0||end<=start)throw Error('Retained constructor missing');
const pe=b.readUInt32LE(0x3c),opt=pe+24;
if(b.readUInt16LE(0)!==0x5a4d||b.readUInt32LE(pe)!==0x4550||b.readUInt16LE(opt)!==0x20b)throw Error('Expected PE32+');
const base=Number(b.readBigUInt64LE(opt+24)),sections=[];
for(let i=0;i<b.readUInt16LE(pe+6);i++){const p=opt+b.readUInt16LE(pe+20)+i*40;sections.push({name:b.toString('ascii',p,p+8).replace(/\0.*$/,''),rva:b.readUInt32LE(p+12),virtual_size:b.readUInt32LE(p+8),raw_size:b.readUInt32LE(p+16),raw_offset:b.readUInt32LE(p+20),characteristics:hx(b.readUInt32LE(p+36)),section_header_file_offset:hx(p)});}
function offset(va,n=1){const r=va-base,s=sections.filter(s=>r>=s.rva&&r+n<=s.rva+s.raw_size);if(s.length!==1)throw Error('Not uniquely file-backed: '+hx(va)+'+'+n);const p=s[0].raw_offset+r-s[0].rva;if(p+n>b.length)throw Error('EOF');return p;}
function backing(va,n){const r=va-base,ss=sections.filter(s=>r>=s.rva&&r+n<=s.rva+Math.max(s.virtual_size,s.raw_size));if(ss.length!==1)throw Error('Not uniquely image mapped '+hx(va));const s=ss[0],delta=r-s.rva;if(Number(s.characteristics)&0x20000000)throw Error('Code excluded');return {section:s.name,section_header_file_offset:s.section_header_file_offset,section_rva:hx(s.rva),virtual_size:s.virtual_size,raw_size:s.raw_size,section_offset:delta,kind:delta+n<=s.raw_size?'file':delta>=s.raw_size?'virtual-zero-fill':'mixed',file_offset:delta+n<=s.raw_size?hx(s.raw_offset+delta):null};}
function data(a,n){const m=backing(a,n);if(m.kind==='file')return b.subarray(offset(a,n),offset(a,n)+n);if(m.kind==='virtual-zero-fill')return Buffer.alloc(n);throw Error('Mixed raw/zero region excluded');}
const u64=a=>{const value=b.readBigUInt64LE(offset(a,8));if(value>BigInt(Number.MAX_SAFE_INTEGER))throw Error('Pointer exceeds exact Number domain '+hx(a));return Number(value);};
const relocRva=b.readUInt32LE(opt+112+5*8),relocSize=b.readUInt32LE(opt+116+5*8),relocs=new Map();
for(let cursor=0;cursor<relocSize;){const p=offset(base+relocRva+cursor,8),page=b.readUInt32LE(p),size=b.readUInt32LE(p+4);if(size<8||size%2||cursor+size>relocSize)throw Error('Reloc block');for(let i=8;i<size;i+=2){const v=b.readUInt16LE(p+i);if(v>>>12===10){const slot=base+page+(v&4095);if(relocs.has(slot))throw Error('Duplicate DIR64');relocs.set(slot,{block_file_offset:hx(p),block_page_rva:hx(page),entry_file_offset:hx(p+i),entry_word:hx(v),type:'IMAGE_REL_BASED_DIR64'});}else if(v>>>12!==0)throw Error('Non DIR64 relocation');}cursor+=size;}
const roots=[['locale',0x1800a17e0,0x160],['lconv',0x1800a1700,0x98],['time',0x18008ad20,0x2c0],['category',0x1800a1948,4]];
const summary=roots.map(([name,va,size])=>({name,va:hx(va),offset:hx(offset(va,size)),size,sha256:sha(data(va,size)),nonzero_qwords:Array.from({length:Math.floor(size/8)},(_,i)=>{const address=va+i*8,value=b.readBigUInt64LE(offset(address,8));return value!==0n?{offset:hx(i*8),value:hx(value),dir64:relocs.has(address)}:null;}).filter(Boolean)}));
if(process.argv.includes('--summary')){console.log(JSON.stringify(summary,null,2));process.exit(0);}
const regions=[],edges=[];
function region(name,va,size,kind,extent_basis){
  if(regions.some(r=>va<r.va+r.size&&r.va<va+size))throw Error('Region overlap '+name);
  const bytes=Buffer.from(data(va,size));
  const r={id:regions.length,name,va,size,kind,extent_basis,raw:bytes,normalized:Buffer.from(bytes)};regions.push(r);return r;
}
for(const [name,va,size] of roots)region(name,va,size,'root','Retained consumer field accesses; exact bounded requested extent');
const ctype=region('ctype_signed_domain',0x18008a120,0x300,'u16_table','FUN_180062a80 allocates 0x180 WORDs and exposes +0x100 view; snapshot of signed-index domain [-128,255], not a symbol-size claim');
const lower=region('lower_signed_domain',0x18008a420,0x180,'u8_table','FUN_180062a80 allocates 0x180 bytes and exposes +0x80 view; bounded signed-index domain [-128,255], not a symbol-size claim');
const upper=region('upper_signed_domain',0x18008a5a0,0x180,'u8_table','Same constructor allocation/view schema as lower');
function stringRegion(va,encoding){
  const unit=encoding==='utf16le'?2:1,existing=regions.find(r=>va===r.va&&r.kind===encoding);
  if(existing)return existing;
  const container=regions.find(r=>va>=r.va&&va<r.va+r.size);
  const cap=256;let units=0;
  for(;units<cap;units++){const raw=data(va+units*unit,unit);if(unit===1?raw[0]===0:raw.readUInt16LE(0)===0)break;}
  if(units===cap)throw Error('String bound exceeded '+hx(va));
  const size=(units+1)*unit;
  if(container){if(va+size>container.va+container.size)throw Error('String exceeds container');return container;}
  const r=region('string_'+va.toString(16).slice(3)+'_'+encoding,va,size,encoding,'First NUL within 256 code units; terminator included');
  r.text=r.raw.subarray(0,size-unit).toString(encoding);if(encoding==='ascii'&&r.raw.some(x=>x>127))throw Error('Non-ASCII');return r;
}
function edge(source,slot,kind,targetRegion){
  const address=source.va+slot,target=u64(address),proof=relocs.get(address);
  if(!target||!proof)throw Error('Expected nonnull DIR64 pointer '+hx(address));
  const dest=targetRegion||stringRegion(target,kind);
  if(target<dest.va||target>=dest.va+dest.size)throw Error('Target not in bounded region');
  source.normalized.fill(0,slot,slot+8);
  edges.push({source_region:source.id,source_offset:slot,slot_va:hx(address),slot_file_offset:hx(offset(address,8)),target_region:dest.id,target_offset:target-dest.va,target_va:hx(target),kind,relocation_proof:proof});
}
const [locale,lconv,time,category]=regions;category.text=category.raw.subarray(0,2).toString('utf16le');
edge(locale,0,'ctype_view',ctype);
for(let c=1;c<=5;c++)edge(locale,0x28+c*0x20,'category_name',category);
edge(locale,0xf8,'lconv',lconv);edge(locale,0x110,'lower_view',lower);edge(locale,0x118,'upper_view',upper);edge(locale,0x120,'initial_time',time);edge(locale,0x158,'self',locale);
for(let i=0;i<=0x48;i+=8)edge(lconv,i,'ascii');
for(let i=0x58;i<=0x90;i+=8)edge(lconv,i,'utf16le');
for(let i=0;i<=0x150;i+=8)edge(time,i,'ascii');
for(let i=0x160;i<=0x2b8;i+=8)edge(time,i,'utf16le');
// Reject any image relocation inside the graph which is not an explicit schema edge.
for(const r of regions)for(const slot of relocs.keys())if(slot>=r.va&&slot<r.va+r.size&&!edges.some(e=>e.slot_va===hx(slot)))throw Error('Unexplained graph DIR64 '+hx(slot));
if(category.raw.toString('hex')!=='43000000')throw Error('Category sentinel changed');
const json={schema:1,provenance:'local-file-derived; no new REA evidence IDs',input:{path:target,sha256:sha(b),bytes:b.length,image_base:hx(base)},pe:{sections,relocation_directory:{rva:hx(relocRva),size:relocSize}},retained_evidence:{constructor:{function_va:'0x180062a80',evidence_id:'ev_50614aa9857141802f47ef16f2118e4167da2b050bd76658ee8a7a4107a6b417',file:'../anadius-all-functions.pseudo.c'},root_consumers:['../anadius-crt-locale-ref-source','../anadius-crt-locale-free-source','../anadius-crt-ptd-data-source']},scalars:{locale_mb_cur_max:{offset:8,u32:locale.raw.readUInt32LE(8)},locale_refcount:{offset:16,i32:locale.raw.readInt32LE(16)},locale_u32_18:{offset:24,u32:locale.raw.readUInt32LE(24)},locale_u32_1c:{offset:28,u32:locale.raw.readUInt32LE(28)},time_u32_158:{offset:0x158,u32:time.raw.readUInt32LE(0x158)},time_refcount:{offset:0x15c,i32:time.raw.readInt32LE(0x15c)},lconv_scalar_50:{offset:0x50,hex:lconv.raw.subarray(0x50,0x58).toString('hex'),qword:hx(lconv.raw.readBigUInt64LE(0x50)),dir64:relocs.has(lconv.va+0x50)}},regions:regions.map(r=>({id:r.id,name:r.name,va:hx(r.va),backing:backing(r.va,r.size),size:r.size,kind:r.kind,extent_basis:r.extent_basis,...(r.text!==undefined?{text:r.text}:{}),raw_sha256:sha(r.raw),raw_hex:r.raw.toString('hex'),normalized_sha256:sha(r.normalized),normalized_hex:r.normalized.toString('hex')})),edges,limits:['No original binary execution, LoadLibrary, writes, or arbitrary pointer/code traversal.','No claim that bounded table snapshots equal linker symbol extents.','Locale +0x108 is NULL in static initial state; dynamic ownership-base rule -0xfe does not identify an initial owned allocation.','Unknown wider process/global initialization graph is excluded.','Virtual-zero-fill leaf bytes are PE mapping-derived zeros, not file-backed bytes.']};
fs.writeFileSync(path.join(dir,'initial_locale.extraction.json'),JSON.stringify(json,null,2)+'\n');
fs.writeFileSync(path.join(dir,'constructor.selected.pseudo.c'),retainedText.slice(start,end));
const cppArray=(r)=>`constexpr std::uint8_t data_${r.id}[] = {\n${Array.from({length:Math.ceil(r.size/16)},(_,i)=>'    '+Array.from(r.normalized.subarray(i*16,i*16+16),v=>'0x'+v.toString(16).padStart(2,'0')).join(', ')+',').join('\n')}\n};\n`;
const header=`#pragma once\n#include <cstdint>\n#include <span>\n#include <string_view>\n\nnamespace anadius::crt::initial_locale_data {\nstruct Region { std::string_view name; std::uint64_t preferred_va; std::span<const std::uint8_t> bytes; };\nstruct Relocation { std::uint32_t source_region, source_offset, target_region, target_offset; };\nenum Root : std::uint32_t { Locale = 0, Lconv = 1, Time = 2, Category = 3 };\n// Bytes contain zero in every pointer slot. Allocate/copy each region then apply relocations.\n// preferred_va is evidence metadata only; never dereference or install it as a live pointer.\nstd::span<const Region> regions() noexcept;\nstd::span<const Relocation> relocations() noexcept;\n}\n`;
fs.writeFileSync(path.join(dir,'initial_locale_data.hpp'),header);
fs.writeFileSync(path.join(dir,'initial_locale_data.cpp'),`// Generated by extract.cjs from unchanged SHA-256 ${expected}.\n#include "initial_locale_data.hpp"\n\nnamespace anadius::crt::initial_locale_data {\nnamespace {\n${regions.map(cppArray).join('\n')}constexpr Region all_regions[] = {\n${regions.map(r=>`    {"${r.name}", ${hx(r.va)}ULL, data_${r.id}},`).join('\n')}\n};\nconstexpr Relocation all_relocations[] = {\n${edges.map(e=>`    {${e.source_region}, ${e.source_offset}, ${e.target_region}, ${e.target_offset}},`).join('\n')}\n};\n}\nstd::span<const Region> regions() noexcept { return all_regions; }\nstd::span<const Relocation> relocations() noexcept { return all_relocations; }\n}\n`);
console.log(JSON.stringify({regions:regions.length,relocations:edges.length,raw_bytes:regions.reduce((s,r)=>s+r.size,0),outputs:['initial_locale.extraction.json','initial_locale_data.hpp','initial_locale_data.cpp'],scalars:json.scalars},null,2));
