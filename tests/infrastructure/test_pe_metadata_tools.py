#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
from pathlib import Path
import importlib.util, json, struct, tempfile, sys
sys.dont_write_bytecode = True
r=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(r/'scripts'))
fixture_root=r/'tests/support/golden'

def load(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    mod=importlib.util.module_from_spec(spec); spec.loader.exec_module(mod); return mod

fin=load('fin',r/'scripts/ursynex_golden_finalizer.py')
fin.ROOT=fixture_root
# Synthetic parser fixtures, not an activated product contract.
fin.load_target=lambda root,name: json.loads((fixture_root/'metadata'/name).read_text())
ary=load('ary',r/'scripts/verify_aryra_golden_layout.py')
mus=load('mus',r/'scripts/verify_mustavex_golden_metadata.py')
ary_policy=json.loads((fixture_root/'metadata/ARYRA_GOLDEN_LAYOUT.json').read_text())
mustavex=json.loads((fixture_root/'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())
assert ary_policy['sections_in_order']==['.text','.rdata','.data','.pdata','.rsrc','.reloc']
assert mustavex['required']['timestamp']=='0xCD268EB1'
assert mustavex['required']['export_internal_name']=='citrine_eclipse_mirage.unsigned.dll'
fin.validate_policy_bindings()
assert fin.FINALIZER_POLICY['integrity_finalization']['pe_checksum']=='RECOMPUTE_AFTER_APPROVED_MUTATIONS'

# F1538-002: VersionInfo StringTable key is executable policy, not metadata-only prose.
fixed=mustavex['required']['versioninfo']['fixed']
fixed_bytes=struct.pack('<13I',*(int(v,16) if isinstance(v,str) else int(v) for v in fixed.values()))
strings=[{'key':k,'value':v,'value_bytes':b'','children':[]} for k,v in mustavex['required']['versioninfo']['strings'].items()]
translation=struct.pack('<HH',int(mustavex['required']['versioninfo']['translation_language'],16),mustavex['required']['versioninfo']['translation_codepage'])
good_vi={'key':'VS_VERSION_INFO','value':None,'value_bytes':fixed_bytes,'children':[
    {'key':'StringFileInfo','value':None,'value_bytes':b'','children':[{'key':mustavex['required']['versioninfo']['string_table'],'value':None,'value_bytes':b'','children':strings}]},
    {'key':'VarFileInfo','value':None,'value_bytes':b'','children':[{'key':'Translation','value':None,'value_bytes':translation,'children':[]}]}
]}
assert not mus.validate_versioninfo_root(good_vi,mustavex['required'])
bad_vi=json.loads(json.dumps(good_vi,default=lambda x: None)) if False else None
wrong_table={'key':'VS_VERSION_INFO','value':None,'value_bytes':fixed_bytes,'children':[
    {'key':'StringFileInfo','value':None,'value_bytes':b'','children':[{'key':'000004b0','value':None,'value_bytes':b'','children':strings}]},
    {'key':'VarFileInfo','value':None,'value_bytes':b'','children':[{'key':'Translation','value':None,'value_bytes':translation,'children':[]}]}
]}
assert any(x.startswith('StringTable=') for x in mus.validate_versioninfo_root(wrong_table,mustavex['required']))

# Minimal PE32+ fixture: sufficient for finalizer core + ARYRA structural gate.
pe=0x100; coff=pe+4; opt=pe+24; optsz=0xF0; sec=opt+optsz
data=bytearray(0x400+6*0x200)
data[:2]=b'MZ'; struct.pack_into('<I',data,0x3c,pe); data[pe:pe+4]=b'PE\0\0'
struct.pack_into('<H',data,coff,0x8664); struct.pack_into('<H',data,coff+2,6); struct.pack_into('<H',data,coff+16,optsz)
struct.pack_into('<H',data,opt,0x20B); data[opt+2]=99; data[opt+3]=99
struct.pack_into('<I',data,opt+32,0x1000); struct.pack_into('<I',data,opt+36,0x200)
struct.pack_into('<H',data,opt+40,6); struct.pack_into('<H',data,opt+42,0)
struct.pack_into('<H',data,opt+48,6); struct.pack_into('<H',data,opt+50,0)
struct.pack_into('<H',data,opt+68,2)
for i,name in enumerate(ary_policy['sections_in_order']):
    raw=name.encode('ascii').ljust(8,b'\0')
    o=sec+i*40; data[o:o+8]=raw
    # Structurally valid, aligned non-overlapping geometry; exact parity remains intentionally unpinned.
    struct.pack_into('<IIII',data,o+8,0x400,0x1000+i*0x1000,0x200,0x400+i*0x200)
# Synthetic encoded Rich block before PE.
start=0x80; rp=0xA0; key=0x12345678
vals=[0x536e6144,0,0,0,0x01020304,5]
for i,v in enumerate(vals): struct.pack_into('<I',data,start+i*4,v^key)
data[rp:rp+4]=b'Rich'; struct.pack_into('<I',data,rp+4,key)

with tempfile.TemporaryDirectory() as td:
    p=Path(td)/'x.dll'; p.write_bytes(data)
    b=bytearray(p.read_bytes()); pe2,coff2,opt2=fin.parse_core(b)
    struct.pack_into('<I',b,coff2+4,fin.TARGET_TIMESTAMP); b[opt2+2]=fin.TARGET_LINKER[0]; b[opt2+3]=fin.TARGET_LINKER[1]
    assert fin.strip_rich_in_place(b,pe2)
    checksum_offset=opt2+64
    struct.pack_into('<I',b,checksum_offset,0)
    checksum=fin.compute_pe_checksum(b,checksum_offset)
    struct.pack_into('<I',b,checksum_offset,checksum)
    p.write_bytes(b)
    assert b.find(b'Rich',0,pe2)==-1
    assert struct.unpack_from('<I',b,checksum_offset)[0]==fin.compute_pe_checksum(b,checksum_offset)
    assert struct.unpack_from('<I',b,coff2+4)[0]==0xCD268EB1
    assert (b[opt2+2],b[opt2+3])==(14,0)
    errors,names=ary.inspect(p,ary_policy)
    assert not errors and names==ary_policy['sections_in_order']
    # Audit3 F323-S0106-001: file bytes beyond SizeOfOptionalHeader do not authorize reads.
    short=bytearray(p.read_bytes()); struct.pack_into('<H',short,coff2+16,0x20); shortp=Path(td,'bad_declared_optional_header.dll'); shortp.write_bytes(short)
    for label,fn in [('URSYNEX',lambda: fin.parse_core(bytearray(shortp.read_bytes()))),('ARYRA',lambda: ary.inspect(shortp,ary_policy)),('MUSTAVEX',lambda: mus.inspect(shortp,mustavex))]:
        try:
            fn(); raise AssertionError(label+' accepted fields outside declared optional header')
        except (fin.PEError,ary.PEError,mus.PEError) as e:
            assert 'declared optional header' in str(e), (label,e)
    # Wrong section order must fail closed.
    bad=bytearray(p.read_bytes()); o=sec; bad[o:o+8]=b'.rdata\0\0'; Path(td,'bad.dll').write_bytes(bad)
    errors,_=ary.inspect(Path(td,'bad.dll'),ary_policy)
    assert errors
    # F1537-001: raw section extent beyond EOF must fail closed.
    bad=bytearray(p.read_bytes()); struct.pack_into('<I',bad,sec+20,len(bad)+0x100); Path(td,'bad_raw_eof.dll').write_bytes(bad)
    errors,_=ary.inspect(Path(td,'bad_raw_eof.dll'),ary_policy)
    assert any('raw extent' in x and 'exceeds file size' in x for x in errors), errors
    # F1537-001: overlapping loaded virtual section extents must fail closed.
    bad=bytearray(p.read_bytes()); first_va=struct.unpack_from('<I',bad,sec+12)[0]; struct.pack_into('<I',bad,sec+40+12,first_va); Path(td,'bad_virtual_overlap.dll').write_bytes(bad)
    errors,_=ary.inspect(Path(td,'bad_virtual_overlap.dll'),ary_policy)
    assert any('virtual section overlap' in x for x in errors), errors
    # F321-S052-001: mandatory PE alignment and raw/RVA-order validity.
    bad=bytearray(p.read_bytes()); struct.pack_into('<I',bad,sec+20,0x401); Path(td,'bad_raw_alignment.dll').write_bytes(bad)
    errors,_=ary.inspect(Path(td,'bad_raw_alignment.dll'),ary_policy); assert any('PointerToRawData' in x for x in errors), errors
    bad=bytearray(p.read_bytes()); struct.pack_into('<I',bad,sec+16,0x201); Path(td,'bad_raw_size_alignment.dll').write_bytes(bad)
    errors,_=ary.inspect(Path(td,'bad_raw_size_alignment.dll'),ary_policy); assert any('SizeOfRawData' in x for x in errors), errors
    bad=bytearray(p.read_bytes()); struct.pack_into('<I',bad,sec+12,0x1001); Path(td,'bad_va_alignment.dll').write_bytes(bad)
    errors,_=ary.inspect(Path(td,'bad_va_alignment.dll'),ary_policy); assert any('VirtualAddress' in x for x in errors), errors
    bad=bytearray(p.read_bytes()); struct.pack_into('<I',bad,sec+20,0x600); struct.pack_into('<I',bad,sec+40+20,0x400); Path(td,'bad_raw_order.dll').write_bytes(bad)
    errors,_=ary.inspect(Path(td,'bad_raw_order.dll'),ary_policy); assert any('raw-data order' in x for x in errors), errors
    # HF12 /MT MSVC UCRT .fptable: the seventh section is accepted only with
    # exact section order, size, RW non-exec flags and zero-initialized raw data.
    fp_spec=ary_policy['static_crt_fptable_profile']
    assert fp_spec['requires_shipping_runtime']=='/MT (MultiThreaded)'
    assert fp_spec['sections_in_order']==['.text','.rdata','.data','.pdata','.fptable','.rsrc','.reloc']
    seven=bytearray(p.read_bytes()); seven.extend(b'\0'*0x200)
    struct.pack_into('<H',seven,coff+2,7)
    for i,name in enumerate(fp_spec['sections_in_order']):
        o=sec+i*40; seven[o:o+40]=b'\0'*40; seven[o:o+8]=name.encode('ascii').ljust(8,b'\0')
        struct.pack_into('<IIII',seven,o+8,
            0x100 if name=='.fptable' else 0x400,
            0x1000+i*0x1000,0x200,0x400+i*0x200)
        struct.pack_into('<I',seven,o+36,0xC0000040 if name=='.fptable' else 0x40000040)
    sp=Path(td,'aryra_static7_good.dll'); sp.write_bytes(seven)
    errors,names=ary.inspect(sp,ary_policy)
    assert not errors,(errors,names)
    assert names==fp_spec['sections_in_order']
    fp_header=sec+4*40; fp_raw=0x400+4*0x200
    def reject_fptable(label,mutate,expected):
        bad=bytearray(seven); mutate(bad)
        q=Path(td,'aryra_fp_'+label+'.dll'); q.write_bytes(bad)
        errors,_=ary.inspect(q,ary_policy)
        assert any(expected in e for e in errors),(label,expected,errors)
    reject_fptable('renamed',lambda b: b.__setitem__(slice(fp_header,fp_header+8),b'.junk\0\0\0'), 'sections=')
    reject_fptable('wrong_order',lambda b: b.__setitem__(slice(sec+4*40,sec+4*40+8),b'.rsrc\0\0\0'), 'sections=')
    reject_fptable('wrong_virtual_size',lambda b: struct.pack_into('<I',b,fp_header+8,0x101),'.fptable VirtualSize=')
    reject_fptable('wrong_raw_size',lambda b: struct.pack_into('<I',b,fp_header+16,0x400),'.fptable SizeOfRawData=')
    reject_fptable('exec_flag',lambda b: struct.pack_into('<I',b,fp_header+36,0xE0000040),'.fptable Characteristics=')
    reject_fptable('read_only',lambda b: struct.pack_into('<I',b,fp_header+36,0x40000040),'.fptable Characteristics=')
    reject_fptable('code_flag',lambda b: struct.pack_into('<I',b,fp_header+36,0xC0000020),'.fptable Characteristics=')
    reject_fptable('nonzero_raw',lambda b: b.__setitem__(fp_raw,0x42),'.fptable raw bytes not zero initialized')
    reject_fptable('bad_raw_pointer',lambda b: struct.pack_into('<I',b,fp_header+20,len(b)+0x200),'raw extent')
    reject_fptable('misaligned_rva',lambda b: struct.pack_into('<I',b,fp_header+12,0x5001),'VirtualAddress')
    reject_fptable('overlap',lambda b: struct.pack_into('<I',b,fp_header+12,0x4000),'virtual section overlap')
    # The whitelist must not expand to arbitrary eighth sections.
    eight=bytearray(seven); struct.pack_into('<H',eight,coff+2,8)
    eight[sec+7*40:sec+7*40+8]=b'.extra\0\0'; qp=Path(td,'aryra_fp_eighth.dll'); qp.write_bytes(eight)
    errors,_=ary.inspect(qp,ary_policy)
    assert any('section_count=8' in e for e in errors),errors
    # F1538-001/F321-S053-001: current identity detection is case-insensitive and covers UTF-16LE.
    current=mustavex['forbidden_project_identity_tokens'][1]
    ident=Path(td,'bad_identity.dll'); ident.write_bytes(p.read_bytes()+current.encode('ascii'))
    errors=mus.inspect(ident,mustavex)
    assert any(x=='project release identity embedded: '+current for x in errors), errors
    ident=Path(td,'bad_identity_mixed.dll'); ident.write_bytes(p.read_bytes()+current.title().encode('ascii'))
    errors=mus.inspect(ident,mustavex); assert any(x=='project release identity embedded: '+current for x in errors), errors
    ident=Path(td,'bad_identity_utf16.dll'); ident.write_bytes(p.read_bytes()+current.encode('utf-16le'))
    errors=mus.inspect(ident,mustavex); assert any(x=='project release identity embedded: '+current for x in errors), errors
    # F321-S063-001: virtual-only section bytes are never treated as file-backed RVA data.
    bad=bytearray(p.read_bytes()); first_va=struct.unpack_from('<I',bad,sec+12)[0]; dd=opt2+112; struct.pack_into('<II',bad,dd,first_va+0x300,0x20); vp=Path(td,'bad_virtual_rva.dll'); vp.write_bytes(bad)
    try:
        mus.inspect(vp,mustavex)
        raise AssertionError('MUSTAVEX accepted virtual-only export RVA')
    except mus.PEError as e:
        assert 'virtual-only' in str(e), e

    # S0108: malformed resource graphs are bounded rather than recursively trusted.
    dd=opt2+112
    # Self-cycle: root directory points back to itself.
    bad=bytearray(p.read_bytes()); struct.pack_into('<II',bad,dd+2*8,0x1000,0x80)
    roff=0x400; bad[roff:roff+0x80]=b'\0'*0x80
    struct.pack_into('<HH',bad,roff+12,0,1); struct.pack_into('<II',bad,roff+16,16,0x80000000)
    cyc=Path(td,'bad_resource_cycle.dll'); cyc.write_bytes(bad)
    try:
        mus.inspect(cyc,mustavex); raise AssertionError('MUSTAVEX accepted resource cycle')
    except mus.PEError as e:
        assert 'cycle' in str(e),e
    # Depth budget: nine nested directories exceed the bounded depth of eight.
    bad=bytearray(p.read_bytes()); struct.pack_into('<II',bad,dd+2*8,0x1000,0x1F0)
    bad[roff:roff+0x1F0]=b'\0'*0x1F0
    for level in range(10):
        rel=level*24; struct.pack_into('<HH',bad,roff+rel+12,0,1)
        nxt=(level+1)*24
        struct.pack_into('<II',bad,roff+rel+16,16+level,0x80000000|nxt)
    deep=Path(td,'bad_resource_depth.dll'); deep.write_bytes(bad)
    try:
        mus.inspect(deep,mustavex); raise AssertionError('MUSTAVEX accepted excessive resource depth')
    except mus.PEError as e:
        assert 'depth budget' in str(e),e
    # Entry budget: fail before iterating attacker-controlled declared fanout.
    bad=bytearray(p.read_bytes()); struct.pack_into('<II',bad,dd+2*8,0x1000,0x80)
    bad[roff:roff+0x80]=b'\0'*0x80; struct.pack_into('<HH',bad,roff+12,0,1025)
    fan=Path(td,'bad_resource_fanout.dll'); fan.write_bytes(bad)
    try:
        mus.inspect(fan,mustavex); raise AssertionError('MUSTAVEX accepted excessive resource fanout')
    except mus.PEError as e:
        assert 'entry budget' in str(e),e


# CORVEXON Golden Trail binary-side ABI self-test.
cor=load('cor',r/'scripts/verify_corvexon_golden_trail.py')
cor_policy=json.loads((fixture_root/'metadata/CORVEXON_GOLDEN_TRAIL.json').read_text())

def build_corvexon_fixture(imports=('CRYPT32.DLL','WS2_32.DLL','KERNEL32.DLL')):
    pe=0x100; coff=pe+4; opt=coff+20; optsz=0xF0; sec=opt+optsz
    raw_base=0x400; va_base=0x1000; raw_size=0x1600
    data=bytearray(raw_base+raw_size)
    data[:2]=b'MZ'; struct.pack_into('<I',data,0x3c,pe); data[pe:pe+4]=b'PE\0\0'
    struct.pack_into('<H',data,coff,0x8664); struct.pack_into('<H',data,coff+2,1); struct.pack_into('<H',data,coff+16,optsz)
    struct.pack_into('<H',data,opt,0x20B); struct.pack_into('<I',data,opt+16,0x1500); struct.pack_into('<H',data,opt+68,2)
    struct.pack_into('<Q',data,opt+24,0x180000000); struct.pack_into('<I',data,opt+56,0x3000)
    struct.pack_into('<I',data,opt+108,16)
    # one mapped fixture section spanning all directories/code. It is executable for
    # the entrypoint and writable for the synthetic /GS SecurityCookie.
    data[sec:sec+8]=b'.text\0\0\0'; struct.pack_into('<IIII',data,sec+8,raw_size,va_base,raw_size,raw_base)
    struct.pack_into('<I',data,sec+36,0xE0000020)  # CODE | EXECUTE | READ | WRITE
    def off(rva): return raw_base+(rva-va_base)
    dd=opt+112
    struct.pack_into('<II',data,dd+0*8,0x1100,0x100)
    struct.pack_into('<II',data,dd+1*8,0x1300,0x100)
    # normalized shipping /GS proof: PE32+ Load Config with mapped SecurityCookie pointer
    struct.pack_into('<II',data,dd+10*8,0x1600,0x100)
    lo=off(0x1600); struct.pack_into('<I',data,lo,0x100); struct.pack_into('<Q',data,lo+88,0x180000000+0x1700)
    # export directory
    eo=off(0x1100)
    struct.pack_into('<IIHHIIIIIII',data,eo,0,0,0,0,0x1200,1,3,3,0x1190,0x11A0,0x11B0)
    module_name=b'citrine_eclipse_mirage.unsigned.dll\0'
    data[off(0x1200):off(0x1200)+len(module_name)]=module_name
    for i,rva in enumerate((0x1510,0x1520,0x1530)): struct.pack_into('<I',data,off(0x1190)+i*4,rva)
    names=('DllMain','SURGE','DllShutdown'); nrvas=(0x11C0,0x11D0,0x11E0)
    for i,rva in enumerate(nrvas): struct.pack_into('<I',data,off(0x11A0)+i*4,rva)
    for i,idx in enumerate((0,1,2)): struct.pack_into('<H',data,off(0x11B0)+i*2,idx)
    for name,rva in zip(names,nrvas):
        z=name.encode()+b'\0'; data[off(rva):off(rva)+len(z)]=z
    # import descriptors, names only (sufficient for module-policy inspection)
    name_rva=0x1900
    for i,mod in enumerate(imports):
        mrva=name_rva+i*0x30; struct.pack_into('<IIIII',data,off(0x1300)+i*20,0x1C00+i*0x20,0,0,mrva,0x1D00+i*0x20)
        z=mod.encode()+b'\0'; data[off(mrva):off(mrva)+len(z)]=z
    struct.pack_into('<I',data,opt+64,0)
    struct.pack_into('<I',data,opt+64,cor.compute_pe_checksum(data,opt+64))
    return data,off(0x11B0),opt

with tempfile.TemporaryDirectory() as td:
    td=Path(td)
    good,ords_off,opt_off=build_corvexon_fixture(); gp=td/'cor_good.dll'; gp.write_bytes(good)
    bad=bytearray(good); struct.pack_into('<H',bad,(0x100+4)+16,0x70); bp=td/'cor_bad_declared_optional_header.dll'; bp.write_bytes(bad)
    try:
        cor.PE(bp); raise AssertionError('CORVEXON accepted data directories outside declared optional header')
    except cor.PEError as e:
        assert 'declared optional header' in str(e), e
    # CORVEXON must not treat zero-raw PE sections as file-backed storage:
    # a virtual-only RVA cannot alias the DOS header or unrelated file data.
    section_header=opt_off+0xF0
    for raw_pointer in (0,0x400):
        bad=bytearray(good)
        struct.pack_into('<II',bad,section_header+16,0,raw_pointer)
        zero_raw=td/('cor_zero_raw_'+str(raw_pointer)+'.dll')
        zero_raw.write_bytes(bad)
        zero_pe=cor.PE(zero_raw)
        for offset in (0,0x100):
            try:
                zero_pe.rva2off(0x1000+offset)
            except cor.PEError as error:
                assert 'virtual-only' in str(error),error
            else:
                raise AssertionError('CORVEXON mapped zero-raw virtual section into file bytes')
    assert cor.PE(gp).rva2off(0x1050)==0x450
    assert cor.PE(gp).rva2off(0x100)==0x100
    errors,info=cor.inspect_shipping_abi(gp,cor_policy)
    assert not errors, errors
    assert info['exports']['DllMain'][0]==1 and info['exports']['SURGE'][0]==2 and info['exports']['DllShutdown'][0]==3
    assert info['entrypoint_rva']==0x1500
    assert info['pe_checksum']==cor.compute_pe_checksum(good,opt_off+64)
    # F1536-002: downstream final-binary acceptance must reject a stale PE checksum.
    bad=bytearray(good); bad[-1]^=0x01; bp=td/'bad_checksum.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('PE CheckSum=' in x for x in errors), errors
    # Wrong ordinal must fail.
    bad=bytearray(good); struct.pack_into('<H',bad,ords_off+2,2); bp=td/'bad_ordinal.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('SURGE ordinal=' in x for x in errors)
    # F321-S061-001: an ordinal-only fourth function is not invisible to the exact ABI gate.
    bad=bytearray(good); eo=0x400+(0x1100-0x1000); funcs=0x400+(0x1190-0x1000); struct.pack_into('<I',bad,eo+20,4); struct.pack_into('<I',bad,funcs+12,0x1540); struct.pack_into('<I',bad,opt_off+64,0); struct.pack_into('<I',bad,opt_off+64,cor.compute_pe_checksum(bad,opt_off+64)); bp=td/'bad_ordinal_only.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('NumberOfFunctions=4' in x or 'ordinal-only' in x for x in errors), errors
    # CRT entry point must remain distinct from exported DllMain.
    bad=bytearray(good); struct.pack_into('<I',bad,opt_off+16,0x1510); bp=td/'bad_entry.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('AddressOfEntryPoint' in x for x in errors)
    # F1539-001: .text ownership is insufficient when the entrypoint section is non-executable.
    sec_off=opt_off+0xF0
    bad=bytearray(good); chars=struct.unpack_from('<I',bad,sec_off+36)[0]; struct.pack_into('<I',bad,sec_off+36,chars & ~0x20000000); bp=td/'bad_entry_nonexec.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('non-executable .text' in x for x in errors), errors
    # Forbidden legacy CRT import must fail closed.
    bad,_,_=build_corvexon_fixture(('CRYPT32.DLL','WS2_32.DLL','KERNEL32.DLL','MSVCRT.DLL')); bp=td/'bad_import.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('MSVCRT.DLL' in x for x in errors)
    # HF12 static CRT: both MSVC runtime and Universal CRT dynamic imports must fail closed.
    for forbidden in ('MSVCP140.DLL','VCRUNTIME140.DLL','VCRUNTIME140_1.DLL','UCRTBASE.DLL','API-MS-WIN-CRT-HEAP-L1-1-0.DLL'):
        bad,_,_=build_corvexon_fixture(('CRYPT32.DLL','WS2_32.DLL','KERNEL32.DLL',forbidden))
        bp=td/('bad_static_'+forbidden.replace('.','_')+'.dll');bp.write_bytes(bad)
        errors,_=cor.inspect_shipping_abi(bp,cor_policy)
        assert any(forbidden in e for e in errors),(forbidden,errors)
    # Normalized CRT /GS shipping requires a valid SecurityCookie pointer.
    bad,_,opt_bad=build_corvexon_fixture(); dd_bad=opt_bad+112; load_rva=struct.unpack_from('<I',bad,dd_bad+10*8)[0]
    raw_base=0x400; va_base=0x1000; lo=raw_base+(load_rva-va_base); image_base=struct.unpack_from('<Q',bad,opt_bad+24)[0]
    struct.pack_into('<Q',bad,lo+88,image_base+0x7fffffff)
    bp=td/'bad_cookie.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('SecurityCookie VA' in x and 'outside image' in x for x in errors)
    # F1539-002: image-backed SecurityCookie storage must also reside in a writable section.
    bad,_,opt_bad=build_corvexon_fixture(); sec_bad=opt_bad+0xF0; chars=struct.unpack_from('<I',bad,sec_bad+36)[0]; struct.pack_into('<I',bad,sec_bad+36,chars & ~0x80000000)
    bp=td/'bad_cookie_readonly.dll'; bp.write_bytes(bad)
    errors,_=cor.inspect_shipping_abi(bp,cor_policy); assert any('SecurityCookie RVA' in x and 'non-writable section' in x for x in errors), errors

# Owner-selected metadata and import descriptor order on a synthetic PE fixture.
import subprocess
from import_descriptor_order import descriptors
order=json.loads((r/'metadata/URSYNEX_GOLDEN_FINALIZER.json').read_text())['import_module_order']
canonical=['CRYPT32.DLL','WS2_32.DLL','KERNEL32.DLL','USER32.DLL','SHELL32.DLL','MSWSOCK.DLL']
assert order==canonical, (order,canonical)
active_import_policy=json.loads((r/'metadata/CORVEXON_GOLDEN_TRAIL.json').read_text())['import_policy']
assert active_import_policy['module_order']==canonical
assert active_import_policy['required_modules']==canonical
assert active_import_policy['allowed_modules']==canonical
assert list(active_import_policy['restricted_named_imports'])==['CRYPT32.DLL','MSWSOCK.DLL']
dist=json.loads((r/'metadata/DIST_CONTRACT.json').read_text())
assert dist['final_import_descriptor_order']==canonical
fixture_import_policy=cor_policy['import_policy']
assert fixture_import_policy['required_modules']==['CRYPT32.DLL','WS2_32.DLL','KERNEL32.DLL']
assert [module for module in fixture_import_policy['allowed_modules'] if module in canonical]==canonical
source_link=(r/'CMakeLists.txt').read_text()
assert '  crypt32 ws2_32 kernel32 user32 shell32)' in source_link
assert 'target_link_libraries(citrine_eclipse_mirage_asio INTERFACE ws2_32 mswsock)' in (r/'cmake/VendoredDependencies.cmake').read_text()
assert 'const wchar_t* modules[]={L"crypt32.dll",L"ws2_32.dll",L"kernel32.dll",L"shell32.dll",L"KernelBase.dll",L"advapi32.dll"};' in (r/'tests/native/application_runtime_tests.cpp').read_text()

with tempfile.TemporaryDirectory() as td:
    path=Path(td)/'normalized.dll'
    original,_,opt2=build_corvexon_fixture(tuple(reversed(order)))
    pe2=0x100;coff2=pe2+4
    struct.pack_into('<HH',original,opt2+40,6,0)
    struct.pack_into('<HH',original,opt2+48,6,0)
    struct.pack_into('<I',original,coff2+4,0x67543210)
    path.write_bytes(original)
    cp=subprocess.run([sys.executable,'-B',str(r/'scripts/ursynex_golden_finalizer.py'),str(path)],capture_output=True,text=True)
    assert cp.returncode==0,cp.stdout+cp.stderr
    result=path.read_bytes()
    assert struct.unpack_from('<I',result,coff2+4)[0]==int(json.loads((r/'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())['required']['timestamp'],16)
    assert tuple(result[opt2+2:opt2+4])==(14,0)
    assert b'Rich' not in result[:pe2]
    assert struct.unpack_from('<I',result,opt2+64)[0]==cor.compute_pe_checksum(result,opt2+64)
    assert descriptors(result)[2]==order
    shipping_policy=json.loads((r/'metadata/CORVEXON_GOLDEN_TRAIL.json').read_text())
    errors,report=cor.inspect_shipping_abi(path,shipping_policy)
    assert report['imports']==canonical, report['imports']
    assert not any('import module order differs' in x for x in errors),errors
    path.write_bytes(original)
    errors,report=cor.inspect_shipping_abi(path,shipping_policy)
    assert report['imports']==list(reversed(canonical)), report['imports']
    assert any('import module order differs' in x for x in errors),errors
    path.write_bytes(result)
    fin.validate_approved_mutation(original,result,pe2,coff2,opt2,order)
    table,records,_=descriptors(original)
    # Only complete descriptors move. Everything outside the table and approved
    # header/checksum spans must remain byte-identical, including thunk/name data.
    bad_offsets=(0x70,0x400,table+16,table+20*len(records)+32)
    for offset in bad_offsets:
        bad=bytearray(result);bad[offset]^=1
        try:
            fin.validate_approved_mutation(original,bad,pe2,coff2,opt2,order)
            raise AssertionError('accepted a mutation outside the exact approved fields/records')
        except (fin.PEError,ValueError):pass
    cp=subprocess.run([sys.executable,'-B',str(r/'scripts/ursynex_golden_finalizer.py'),str(path)],capture_output=True,text=True)
    assert cp.returncode==0 and path.read_bytes()==result
    for label,mutate in [('wrong_os',lambda b:struct.pack_into('<HH',b,opt2+40,10,3)),
                         ('unknown_module',lambda b:b.__setitem__(slice(0x400+0x900,0x400+0x900+6),b'OTHERX'))]:
        bad=bytearray(original);mutate(bad);path.write_bytes(bad)
        cp=subprocess.run([sys.executable,'-B',str(r/'scripts/ursynex_golden_finalizer.py'),str(path)],capture_output=True,text=True)
        assert cp.returncode!=0 and path.read_bytes()==bad,(label,cp.stdout,cp.stderr)
    # Every MSVC application profile must yield a canonical unsigned DLL,
    # BEFORE URSYNEX. The normalizer is fail-closed and idempotent.
    raw_path = Path(td) / 'citrine_eclipse_mirage.unsigned.dll'
    raw, _, _ = build_corvexon_fixture(tuple(reversed(order)))
    raw_path.write_bytes(raw)
    call = [sys.executable, '-B', str(r / 'scripts/import_descriptor_order.py'),
            '--root', str(r), '--dll', str(raw_path)]
    unnormalized = raw_path.read_bytes()
    cp = subprocess.run(call + ['--verify'], capture_output=True, text=True)
    assert cp.returncode != 0 and raw_path.read_bytes() == unnormalized, (cp.stdout, cp.stderr)
    cp = subprocess.run(call, capture_output=True, text=True)
    assert cp.returncode == 0, (cp.stdout, cp.stderr)
    normalized = raw_path.read_bytes()
    table, records, _ = descriptors(unnormalized)
    stop = table + 20 * len(records)
    assert descriptors(normalized)[2] == canonical
    assert normalized[:table] == unnormalized[:table] and normalized[stop:] == unnormalized[stop:]
    assert len(normalized) == len(unnormalized)
    cp = subprocess.run(call + ['--verify'], capture_output=True, text=True)
    assert cp.returncode == 0 and raw_path.read_bytes() == normalized, (cp.stdout, cp.stderr)
    cp = subprocess.run(call, capture_output=True, text=True)
    assert cp.returncode == 0 and raw_path.read_bytes() == normalized, (cp.stdout, cp.stderr)
    # An unexpected module is never silently removed or renamed.
    unexpected, _, _ = build_corvexon_fixture(('CRYPT32.DLL', 'KERNEL32.DLL', 'EXTRA.DLL'))
    raw_path.write_bytes(unexpected)
    cp = subprocess.run(call, capture_output=True, text=True)
    assert cp.returncode != 0 and raw_path.read_bytes() == unexpected, (cp.stdout, cp.stderr)
    cmake_text = (r / 'cmake/BuildPartition.cmake').read_text()
    profile_text = (r / 'scripts/build_orchestration/build_profile.ps1').read_text()
    assert 'Normalize unsigned DLL import descriptors before validation' in cmake_text
    assert 'Unsigned DLL import descriptor order' in profile_text

    # The earlier ARYRA fixture remains an independently tested geometry fixture.
    path.write_bytes(data);errors,_=ary.inspect(path,json.loads((r/'metadata/ARYRA_GOLDEN_LAYOUT.json').read_text()))
    assert not errors,errors

print('GOLDEN_PROTOCOL_TOOL_SELFTEST: PASS')
