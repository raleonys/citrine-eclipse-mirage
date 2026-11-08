#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
from pathlib import Path
import json, struct, sys
from golden_policy import load_target

class PEError(RuntimeError): pass

def u16(b,o):
    if o < 0 or o+2 > len(b): raise PEError(f'truncated u16 at 0x{o:X}')
    return struct.unpack_from('<H',b,o)[0]
def u32(b,o):
    if o < 0 or o+4 > len(b): raise PEError(f'truncated u32 at 0x{o:X}')
    return struct.unpack_from('<I',b,o)[0]

def require_optional_span(b,opt,optsz,rel,size,what):
    if rel < 0 or size < 0 or rel > optsz or size > optsz-rel:
        raise PEError(f'{what} outside declared optional header (SizeOfOptionalHeader=0x{optsz:X})')
    if opt+optsz > len(b): raise PEError('declared optional header exceeds file bounds')
    return opt+rel

def inspect(dll, policy):
    b=dll.read_bytes(); errors=[]
    if len(b)<0x40 or b[:2]!=b'MZ': raise PEError('missing MZ')
    pe=u32(b,0x3c)
    if pe+24>len(b) or b[pe:pe+4]!=b'PE\0\0': raise PEError('missing PE signature')
    coff=pe+4; optsz=u16(b,coff+16); opt=coff+20
    require_optional_span(b,opt,optsz,0,2,'OptionalHeader.Magic')
    require_optional_span(b,opt,optsz,32,8,'SectionAlignment/FileAlignment')
    machine=u16(b,coff); magic=u16(b,opt); nsec=u16(b,coff+2)
    expected_machine=int(policy['machine'],16)
    expected_magic=int(policy['pe_format_magic'],16)
    legacy_sections=policy['sections_in_order']
    static_crt=policy['static_crt_fptable_profile']
    static_sections=static_crt['sections_in_order']
    if machine!=expected_machine: errors.append(f'machine=0x{machine:04X}')
    if magic!=expected_magic: errors.append(f'format_magic=0x{magic:04X}')
    expected_sections=static_sections if nsec==static_crt['section_count'] else legacy_sections
    expected_count=static_crt['section_count'] if nsec==static_crt['section_count'] else policy['section_count']
    if nsec!=expected_count: errors.append(f'section_count={nsec}')
    section_alignment=u32(b,opt+32); file_alignment=u32(b,opt+36)
    if not file_alignment or (file_alignment & (file_alignment-1))!=0 or not (0x200 <= file_alignment <= 0x10000):
        errors.append(f'FileAlignment=0x{file_alignment:X}')
    if not section_alignment:
        errors.append('SectionAlignment=0')
    elif section_alignment & (section_alignment - 1):
        errors.append(f'SectionAlignment=0x{section_alignment:X} is not a power of two')
    elif section_alignment < 0x1000 and section_alignment != file_alignment:
        errors.append(f'low SectionAlignment 0x{section_alignment:X} must equal FileAlignment 0x{file_alignment:X}')
    elif section_alignment >= 0x1000 and section_alignment < file_alignment:
        errors.append(f'SectionAlignment=0x{section_alignment:X} < FileAlignment=0x{file_alignment:X}')
    sec_off=opt+optsz; names=[]; virtual_extents=[]; raw_extents=[]; section_rows=[]
    fptable_seen=[]
    for i in range(nsec):
        o=sec_off+i*40
        if o+40>len(b): raise PEError('truncated section table')
        raw=b[o:o+8].split(b'\0',1)[0]
        try: name=raw.decode('ascii')
        except UnicodeDecodeError: name=raw.decode('ascii','replace')
        names.append(name)
        vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8)
        characteristics=u32(b,o+36)
        if name=='.fptable': fptable_seen.append((i,vs,rs,rp,characteristics))
        if section_alignment and va % section_alignment:
            errors.append(f'{name or "<unnamed>"} VirtualAddress 0x{va:X} is not SectionAlignment 0x{section_alignment:X} aligned')
        if rs:
            if file_alignment and rp % file_alignment:
                errors.append(f'{name or "<unnamed>"} PointerToRawData 0x{rp:X} is not FileAlignment 0x{file_alignment:X} aligned')
            if file_alignment and rs % file_alignment:
                errors.append(f'{name or "<unnamed>"} SizeOfRawData 0x{rs:X} is not FileAlignment 0x{file_alignment:X} aligned')
            if rp > len(b) or rp + rs > len(b):
                errors.append(f'{name or "<unnamed>"} raw extent [0x{rp:X},0x{rp+rs:X}) exceeds file size 0x{len(b):X}')
            raw_extents.append((rp,rp+rs,name or '<unnamed>'))
        section_rows.append((va,rp,rs,name or '<unnamed>'))
        span=max(vs,rs)
        if span:
            virtual_extents.append((va,va+span,name or '<unnamed>'))
    if names!=expected_sections:
        errors.append('sections='+' '.join(names))
    if nsec==static_crt['section_count'] and names==static_sections:
        spec=static_crt['fptable']
        if len(fptable_seen)!=1 or fptable_seen[0][0]!=spec['index']:
            errors.append('static CRT .fptable missing or misplaced')
        else:
            idx,vs,rs,rp,characteristics=fptable_seen[0]
            if 'virtual_size' in spec and vs!=int(spec['virtual_size'],16): errors.append(f'.fptable VirtualSize=0x{vs:X}')
            if 'raw_size' in spec and rs!=int(spec['raw_size'],16): errors.append(f'.fptable SizeOfRawData=0x{rs:X}')
            if spec.get('require_positive_sizes') and (not vs or not rs): errors.append('.fptable must have positive virtual and raw sizes')
            if characteristics!=int(spec['characteristics'],16): errors.append(f'.fptable Characteristics=0x{characteristics:08X}')
            if rp<=len(b) and rs<=len(b)-rp and any(b[rp:rp+rs]):
                errors.append('.fptable raw bytes not zero initialized')
    elif fptable_seen:
        errors.append('unapproved .fptable section profile')
    ordered=sorted(virtual_extents,key=lambda x:(x[0],x[1],x[2]))
    for prev,cur in zip(ordered,ordered[1:]):
        if cur[0] < prev[1]:
            errors.append(f'virtual section overlap: {prev[2]} [0x{prev[0]:X},0x{prev[1]:X}) with {cur[2]} [0x{cur[0]:X},0x{cur[1]:X})')
    raw_ordered=sorted(raw_extents,key=lambda x:(x[0],x[1],x[2]))
    for prev,cur in zip(raw_ordered,raw_ordered[1:]):
        if cur[0] < prev[1]:
            errors.append(f'raw section overlap: {prev[2]} [0x{prev[0]:X},0x{prev[1]:X}) with {cur[2]} [0x{cur[0]:X},0x{cur[1]:X})')
    va_order=[row for row in sorted(section_rows,key=lambda x:x[0]) if row[2]]
    for prev,cur in zip(va_order,va_order[1:]):
        if cur[1] < prev[1]:
            errors.append(f'raw-data order disagrees with RVA order: {prev[3]} raw=0x{prev[1]:X}, {cur[3]} raw=0x{cur[1]:X}')
    return errors,names

def main():
    if len(sys.argv)!=2:
        print('usage: verify_aryra_golden_layout.py <dll>',file=sys.stderr); return 2
    root=Path(__file__).resolve().parents[1]
    try:
        policy=load_target(root,'ARYRA_GOLDEN_LAYOUT.json')
        crt=json.loads((root/'metadata/CRT_STATIC_CONTRACT.json').read_text(encoding='utf-8'))
        if crt['target_modes']['shipping']!=policy['static_crt_fptable_profile']['requires_shipping_runtime']:
            raise PEError('static CRT .fptable profile is not bound to shipping /MT contract')
        errors,names=inspect(Path(sys.argv[1]),policy)
    except (OSError,UnicodeError,json.JSONDecodeError,PEError,struct.error,KeyError,TypeError,ValueError) as e:
        print(f'ARYRA_GOLDEN_LAYOUT: FAIL: {e}',file=sys.stderr); return 1
    if errors:
        print('ARYRA_GOLDEN_LAYOUT: FAIL')
        for e in errors: print(' -',e)
        return 1
    print('ARYRA_GOLDEN_LAYOUT: PASS')
    print(' machine: AMD64')
    print(' format: PE32+')
    print(' sections: '+' '.join(names))
    print(' exact RVA/size/offset parity: NOT ENFORCED')
    return 0
if __name__=='__main__': raise SystemExit(main())
