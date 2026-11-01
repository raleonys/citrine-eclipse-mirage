// Read-only, bounded MSVC x64 RTTI recovery. Never loads the input DLL.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '..', '..');
const input = 'C:\\Users\\Skysect\\Downloads\\anadius64.dll';
const out = path.join(root, 'outputs', 'anadius-type-recovery');
const b = fs.readFileSync(input);
const sha = crypto.createHash('sha256').update(b).digest('hex');
if (sha !== '9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c') throw Error('Target identity changed');
const pe = b.readUInt32LE(0x3c), opt = pe + 24;
if (b.readUInt32LE(pe) !== 0x4550 || b.readUInt16LE(opt) !== 0x20b) throw Error('Expected PE32+');
const image = b.readBigUInt64LE(opt + 24), sections = [];
for (let i = 0; i < b.readUInt16LE(pe + 6); i++) {
  const o = opt + b.readUInt16LE(pe + 20) + i * 40;
  sections.push({name:b.toString('ascii',o,o+8).replace(/\0.*$/,''),rva:b.readUInt32LE(o+12),size:b.readUInt32LE(o+8),raw:b.readUInt32LE(o+20),rawSize:b.readUInt32LE(o+16),flags:b.readUInt32LE(o+36)});
}
const hx = x => '0x' + BigInt(x).toString(16), va = r => hx(image+BigInt(r));
function fo(r,n=1) {
  if (!Number.isSafeInteger(r)||r<0||!Number.isSafeInteger(n)||n<1) throw Error('Invalid range');
  if (r+n <= b.readUInt32LE(opt+60)) return r;
  const s=sections.find(s=>r>=s.rva&&r+n<=s.rva+s.rawSize);
  if (!s||s.raw+r-s.rva+n>b.length) throw Error('Unmapped source range');
  return s.raw+r-s.rva;
}
const u32=r=>b.readUInt32LE(fo(r,4)), i32=r=>b.readInt32LE(fo(r,4)), u64=r=>b.readBigUInt64LE(fo(r,8));
function z(r,max=4096) {const o=fo(r),end=b.indexOf(0,o);if(end<o||end-o>max)throw Error('Unbounded name');return b.toString('ascii',o,end);}
const index=JSON.parse(fs.readFileSync(path.join(root,'outputs','anadius-recovery-index.json'),'utf8').replace(/^\uFEFF/,''));
const functions=new Map(index.procedures.normalized_result.map(p=>[p.address,p]));
const codeSections=sections.filter(s=>(s.flags&0x20000000)!==0);
const isCode=p=>{const r=Number(p-image);return codeSections.some(s=>r>=s.rva&&r<s.rva+s.size);};
const types=new Map(), locators=[];
function type(r) {
  if(types.has(r))return types.get(r);
  const name=z(r+16);
  if(!/^\.\?A[UV]/.test(name))throw Error('Not a class/struct RTTI name');
  const t={rva:hx(r),address:va(r),fileOffset:hx(fo(r,17)),mangledName:name,descriptorVftable: hx(u64(r)),spare:hx(u64(r+8))};
  types.set(r,t);return t;
}
function hierarchy(r) {
  const count=u32(r+8),array=u32(r+12);
  if(count<1||count>256)throw Error('Invalid base count');
  fo(array,count*4);
  const bases=[];
  for(let i=0;i<count;i++) {
    const p=u32(array+4*i),t=type(u32(p));
    bases.push({descriptorRVA:hx(p),descriptorFileOffset:hx(fo(p,28)),type:t.mangledName,typeRVA:t.rva,numContainedBases:u32(p+4),pmd:{memberDisplacement:i32(p+8),vbtableDisplacement:i32(p+12),displacementInsideVBTable:i32(p+16)},attributes:hx(u32(p+20)),classHierarchyRVA:hx(u32(p+24))});
  }
  return {rva:hx(r),fileOffset:hx(fo(r,16)),signature:u32(r),attributes:hx(u32(r+4)),baseCount:count,bases};
}
for(const s of sections.filter(s=>(s.flags&0x20000000)===0))for(let delta=0;delta+24<=s.rawSize;delta+=4) {
  const r=s.rva+delta;
  try {
    if(u32(r)!==1||u32(r+20)!==r)continue;
    const td=type(u32(r+12)),h=hierarchy(u32(r+16));
    if(h.bases[0].type!==td.mangledName)continue;
    locators.push({rva:hx(r),address:va(r),fileOffset:hx(fo(r,24)),offsetFromTop:u32(r+4),constructorDisplacement:u32(r+8),type:td.mangledName,typeRVA:td.rva,hierarchy:h,vftables:[]});
  }catch{}
}
const colMap=new Map(locators.map(x=>[x.address,x]));
for(const s of sections.filter(s=>(s.flags&0x20000000)===0))for(let delta=0;delta+16<=s.rawSize;delta+=8) {
  const r=s.rva+delta;
  try {
    const col=colMap.get(hx(u64(r)));if(!col)continue;
    const slots=[];let termination='entry-cap';
    for(let i=0;i<256;i++) {
      let p;try{p=u64(r+8+8*i);}catch{termination='unmapped';break;}
      if(!isCode(p)){termination='next-non-code-pointer';break;}
      const address=hx(p),fn=functions.get(address);
      slots.push({index:i,pointerAddress:va(r+8+8*i),pointerFileOffset:hx(fo(r+8+8*i,8)),address,functionName:fn?.value??null,functionRecovered:!!fn,thunk:fn?.procedure?.thunk??null});
    }
    if(slots.length)col.vftables.push({address:va(r+8),rva:hx(r+8),fileOffset:hx(fo(r+8,8)),locatorPointerAddress:va(r),observedCodeSlots:slots.length,termination,slots,slotBoundaryStatus:'heuristic-contiguous-code-pointers-not-authoritative-length'});
  }catch{}
}
const result={provenance:'Independent read-only parser of original PE bytes; not a new REA Evidence record',input,sha256:sha,format:'MSVC x64 image-relative RTTI with signature1 and exact self RVA',sourceEvidence:{rdata:'ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082',functionInventory:index.procedures.evidence_id,definedStrings:index.strings.evidence_id},reference:'https://github.com/llvm/llvm-project/blob/main/clang/lib/CodeGen/MicrosoftCXXABI.cpp',limitations:['COL identity and hierarchy structures are directly parsed from file bytes. Vftable trailing length uses contiguous pointers into executable source ranges and is a heuristic.','Base descriptors include the most-derived class and possibly repeated virtual bases; baseCount is not a direct-base count.','Member displacement describes an inherited subobject, not a full class size or complete member layout.','Original function names/signatures, nonvirtual members and missing RTTI remain unknown. No binary or analysis database was changed.'],summary:{typeDescriptors:types.size,completeObjectLocators:locators.length,locatorsWithTables:locators.filter(x=>x.vftables.length).length,vftables:locators.reduce((n,x)=>n+x.vftables.length,0),slotReferences:locators.reduce((n,x)=>n+x.vftables.reduce((m,v)=>m+v.slots.length,0),0)},types:[...types.values()],locators};
fs.mkdirSync(out,{recursive:true});fs.writeFileSync(path.join(out,'msvc-rtti.json'),JSON.stringify(result,null,2));
const lines=['# anadius64.dll: RTTI ve sanal çağrı eşlemesi','','Bu kayıt orijinal PE baytlarının salt okunur ayrıştırmasıdır; özgün C++ sınıf tanımları veya derlenebilir kaynak değildir.','',`SHA-256: ${sha}`,``,`${result.summary.typeDescriptors} type descriptor; ${result.summary.completeObjectLocators} complete-object locator; ${result.summary.vftables} vftable adayı; ${result.summary.slotReferences} sanal slot başvurusu.`, '', 'Vftable bitişi ardışık kod işaretçilerinden tahmin edilmiştir. Yer değiştirmeler tam sınıf boyutu/member yerleşimi sayılmamalıdır. Her yapı ve slot için kaynak dosya ofseti JSON içinde korunur.','', '| RTTI adı | COL adresi | Vftable adresi | Kod slotu |','|---|---|---|---:|'];
for(const c of locators)for(const v of c.vftables)lines.push(`| ${c.type.replace(/\|/g,'&#124;')} | ${c.address} | ${v.address} | ${v.slots.length} |`);
lines.push('','Ayrıntılı kalıtım grafiği, alt nesne ofsetleri ve slot-fonksiyon eşlemeleri: [msvc-rtti.json](msvc-rtti.json).','','Ayrıştırma düzeni için birincil kaynak: [LLVM MicrosoftCXXABI.cpp](https://github.com/llvm/llvm-project/blob/main/clang/lib/CodeGen/MicrosoftCXXABI.cpp).');
fs.writeFileSync(path.join(out,'README.md'),lines.join('\n'));
console.log(JSON.stringify(result.summary));

