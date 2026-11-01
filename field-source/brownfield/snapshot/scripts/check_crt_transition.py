#!/usr/bin/env python3
"""HF12 static CRT contract checker: source, CMake generated flags, PE imports.

On non-Windows, source checks are useful but do NOT prove native/link/PE acceptance.
"""
from pathlib import Path
import argparse,re,sys,struct
ROOT=Path(__file__).resolve().parents[1]
STATIC_TARGETS=('preloader_l','minhook','preloader_l_tinyxml2','preloader_l_native_acceptance_host','preloader_l_release_host')
DEBUG_TARGETS=('preloader_l_debug','minhook_debug','preloader_l_tinyxml2_debug','preloader_l_runtime_host','preloader_l_session_host','preloader_l_socket_host')
FORBIDDEN_PREFIXES=('MSVCP','MSVCR','VCRUNTIME','UCRTBASE','API-MS-WIN-CRT-')

def source_errors(cm):
    problems=[]
    for name in STATIC_TARGETS:
        expect=f'set_property(TARGET {name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")'
        if cm.count(expect)!=1:problems.append('static /MT property missing or duplicated: '+name)
    for name in DEBUG_TARGETS:
        # Debug executable hosts are created in a foreach host expansion, with one
        # CMake target property template for runtime/session/socket hosts.
        target='preloader_l_${host}_host' if name in ('preloader_l_runtime_host','preloader_l_session_host','preloader_l_socket_host') else name
        expect=f'set_property(TARGET {target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")'
        if expect not in cm:problems.append('static /MTd property missing: '+name)
    active='\n'.join(row for row in cm.splitlines() if not row.lstrip().startswith('#'))
    for token in ('MultiThreadedDLL','MultiThreadedDebugDLL','/NODEFAULTLIB','/ENTRY:DllMain','/Zl'):
        # Runtime CMake branches must not retain old custom linker behavior.
        if token in active:problems.append('legacy/dynamic runtime setting in CMake: '+token)
    if 'set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")' not in cm:problems.append('project default static runtime missing')
    if 'message(FATAL_ERROR "HF12 static CRT does not support the historical custom-entry comparison' not in cm:
        problems.append('obsolete custom entry does not fail closed')
    if 'target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)' not in cm:
        problems.append('shipping native and MinHook linkage missing')
    if 'target_link_libraries(preloader_l PRIVATE preloader_l_tinyxml2)' not in cm:
        problems.append('TinyXML2 shipping source linkage missing')
    return problems

def source_check():
    cm=(ROOT/'CMakeLists.txt').read_text()
    problems=source_errors(cm)
    import json
    c=json.loads((ROOT/'metadata/CORVEXON_GOLDEN_TRAIL.json').read_text())['import_policy']['crt_runtime_policy']
    if c.get('shipping_runtime')!='/MT (MultiThreaded)':problems.append('Golden static CRT source selection drift')
    if c.get('allowed_runtime_modules'):problems.append('Golden erroneously permits CRT runtime DLL modules')
    if set(c.get('forbidden_runtime_families',[]))!=set(FORBIDDEN_PREFIXES):problems.append('Golden runtime import denylist drift')
    if problems:return fail(problems)
    print('CRT_STATIC_SOURCE_POLICY: PASS (project owned targets, Golden policy)')
    return 0

def fail(msgs):
    print('CRT_STATIC: FAIL')
    for m in msgs:print(' - '+str(m))
    return 1

def flags_check(build_dir):
    target=build_dir/'CMakeFiles'/'preloader_l.dir'
    parts=[]
    for p in [target/'flags.make',target/'build.make']+list(target.glob('*.rsp')):
        if p.is_file():parts.append(p.read_text(errors='replace'))
    if not parts:return fail(['generated preloader_l compile flags missing '+str(build_dir)])
    txt='\n'.join(parts)
    flags=[v.upper() for v in re.findall(r'(?<!\S)[/-](MDd?|MTd?)(?=\s|$)',txt,flags=re.I)]
    problems=[]
    if 'MD' in flags or 'MDD' in flags:problems.append('dynamic CRT flag in generated shipping: '+repr(flags))
    if 'MTD' in flags:problems.append('Debug static CRT in shipping target: '+repr(flags))
    if '/GS' not in txt.upper() or '/GS-' in txt.upper():problems.append('shipping /GS security flag absent')
    if '/NODEFAULTLIB' in txt.upper() or '/ENTRY:DLLMAIN' in txt.upper() or '/ZL' in txt.upper():problems.append('legacy/manual startup in generated target')
    if problems:return fail(problems)
    print('CRT_STATIC_GENERATED_FLAGS: PASS (shipping /MT; no /MD, /MTd or custom entry)')
    return 0

def pe_info(path):
    b=path.read_bytes()
    if b[:2]!=b'MZ':raise ValueError('missing MZ')
    pe=struct.unpack_from('<I',b,0x3c)[0]
    if b[pe:pe+4]!=b'PE\0\0':raise ValueError('missing PE')
    coff=pe+4;nsec=struct.unpack_from('<H',b,coff+2)[0];opt=coff+20;magic=struct.unpack_from('<H',b,opt)[0]
    entry=struct.unpack_from('<I',b,opt+16)[0]; dd=opt+(112 if magic==0x20b else 96);optsz=struct.unpack_from('<H',b,coff+16)[0]
    dirs=[bool(r or s) for r,s in (struct.unpack_from('<II',b,dd+i*8) for i in range(16))]
    secs=[];ranges=[];so=opt+optsz
    for i in range(nsec):
        o=so+i*40;name=b[o:o+8].split(b'\0',1)[0].decode('ascii','replace');vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8)
        secs.append(name);ranges.append((name,va,max(vs,rs),rp,rs))
    def rva2off(rva):
        for _n,va,span,rp,rs in ranges:
            if va<=rva<va+span:
                d=rva-va
                if d>=rs:raise ValueError(f'RVA 0x{rva:X} virtual-only')
                return rp+d
        if rva<so+nsec*40:return rva
        raise ValueError(f'unmapped RVA 0x{rva:X}')
    imports=[]; irva,isize=struct.unpack_from('<II',b,dd+8)
    if irva and isize:
        pos=irva
        for _ in range(4096):
            o=rva2off(pos);vals=struct.unpack_from('<IIIII',b,o)
            if not any(vals):break
            no=rva2off(vals[3]);end=b.find(b'\0',no)
            if end<0:raise ValueError('unterminated import name')
            imports.append(b[no:end].decode('ascii','strict').upper());pos+=20
        else:raise ValueError('import descriptor runaway')
    return magic,secs,dirs,sorted(set(imports)),entry


def binary_check(path):
    try:
        magic,sections,dirs,imports,entry=pe_info(path)
    except Exception as exc:return fail(['invalid PE: '+str(exc)])
    errs=[]
    if magic!=0x20b:errs.append('not AMD64 PE32+')
    for mod in imports:
        if mod.startswith(FORBIDDEN_PREFIXES):errs.append('dynamic CRT import: '+mod)
    if errs:return fail(errs)
    print('CRT_STATIC_BINARY_IMPORTS: PASS (no forbidden CRT DLL import; CORVEXON remains final authority)')
    return 0

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--source',action='store_true')
    p.add_argument('--binary',nargs=1,metavar='STATIC_PE')
    p.add_argument('--build-dir',type=Path)
    p.add_argument('--expect-runtime',choices=['crt'])
    a=p.parse_args()
    if a.source:return source_check()
    if a.binary:return binary_check(Path(a.binary[0]))
    if a.build_dir:return flags_check(a.build_dir)
    return fail(['missing mode: --source / --build-dir / --binary'])
if __name__=='__main__':sys.exit(main())
