#!/usr/bin/env python3
"""Fail-closed PE .data budget for the exact MSVC /MT Golden profile.

The historical /MD profile is read-only comparison mode, never a shipping
fallback for a malformed or unexpectedly linked /MT artifact.
"""
from pathlib import Path
import json
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
CRT_PREFIXES = ('MSVCP', 'MSVCR', 'VCRUNTIME', 'UCRTBASE', 'API-MS-WIN-CRT-')


def reject(message):
    raise SystemExit('DATA_FOOTPRINT: FAIL - ' + message)


def check(path, comparison_md=False):
    b = Path(path).read_bytes()

    def u16(o):
        if o < 0 or o + 2 > len(b): reject('truncated PE u16')
        return struct.unpack_from('<H', b, o)[0]

    def u32(o):
        if o < 0 or o + 4 > len(b): reject('truncated PE u32')
        return struct.unpack_from('<I', b, o)[0]

    if len(b) < 0x100 or b[:2] != b'MZ': reject('not PE')
    pe = u32(0x3c)
    if pe < 0x40 or pe + 24 > len(b) or b[pe:pe+4] != b'PE\0\0': reject('invalid NT header')
    coff = pe + 4
    machine, count, opt_size = u16(coff), u16(coff+2), u16(coff+16)
    opt = coff + 20
    if machine != 0x8664 or opt_size < 0xf0 or u16(opt) != 0x20b: reject('not x64 PE32+ with full optional header')
    sec_off = opt + opt_size
    if sec_off + count*40 > len(b): reject('section header outside file')
    if u32(opt+108) < 2: reject('missing import directory declaration')
    sections = []
    for i in range(count):
        o = sec_off + 40*i
        name_bytes = b[o:o+8].split(b'\0', 1)[0]
        try: name = name_bytes.decode('ascii')
        except UnicodeDecodeError: reject('non-ASCII PE section name')
        vs, va, raw_size, raw_ptr, flags = u32(o+8), u32(o+12), u32(o+16), u32(o+20), u32(o+36)
        if raw_size and (raw_ptr < sec_off + count*40 or raw_ptr + raw_size > len(b)):
            reject('invalid section raw extent: ' + name)
        sections.append({'name':name, 'vs':vs, 'va':va, 'raw':raw_size, 'ptr':raw_ptr, 'flags':flags})
    names = [x['name'] for x in sections]
    aryra = json.loads((ROOT/'metadata/ARYRA_GOLDEN_LAYOUT.json').read_text(encoding='utf-8'))
    crt = json.loads((ROOT/'metadata/CRT_STATIC_CONTRACT.json').read_text(encoding='utf-8'))
    fp = aryra['static_crt_fptable_profile']
    if crt['target_modes']['shipping'] != '/MT (MultiThreaded)' or fp['requires_shipping_runtime'] != '/MT (MultiThreaded)':
        reject('source CRT contract drift')
    if 'set_property(TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")' not in (ROOT/'CMakeLists.txt').read_text(encoding='utf-8'):
        reject('shipping target not pinned to static CRT')
    expected = aryra['sections_in_order'] if comparison_md else fp['sections_in_order']
    if names != expected: reject('PE section profile mismatch: '+repr(names))
    if len(sections) != (aryra['section_count'] if comparison_md else fp['section_count']):
        reject('PE section count mismatch')
    data = sections[names.index('.data')]
    if not comparison_md:
        f = sections[fp['fptable']['index']]
        required = fp['fptable']
        if (f['name'] != required['name'] or f['vs'] != int(required['virtual_size'], 16)
            or f['raw'] != int(required['raw_size'], 16)
            or f['flags'] != int(required['characteristics'], 16)):
            reject('incompatible MSVC .fptable size/flags/layout')
        if any(b[f['ptr']:f['ptr']+f['raw']]): reject('nonzero .fptable raw bytes')

    def rva_to_offset(rva):
        for s in sections:
            if s['va'] <= rva < s['va'] + s['raw']:
                return s['ptr'] + rva - s['va']
        reject('unbacked PE import RVA')

    import_rva, import_size = u32(opt+112+8), u32(opt+112+12)
    if not import_rva or import_size < 20: reject('missing import directory')
    imports = []
    for i in range(min(128, import_size//20)):
        row = rva_to_offset(import_rva + i*20)
        if row+20 > len(b): reject('truncated import descriptor')
        if b[row:row+20] == bytes(20): break
        name_off = rva_to_offset(u32(row+12))
        end = b.find(b'\0', name_off, min(len(b), name_off+260))
        if end < 0: reject('unterminated import DLL name')
        try: dll = b[name_off:end].decode('ascii').upper()
        except UnicodeDecodeError: reject('malformed import DLL name')
        imports.append(dll)
    else:
        reject('import table not terminated within declared extent')
    if not imports: reject('empty import descriptor table')
    dynamic_crt = any(v.startswith(CRT_PREFIXES) for v in imports)
    if comparison_md and not dynamic_crt: reject('legacy /MD comparison requires dynamic CRT import')
    if not comparison_md and dynamic_crt: reject('static /MT profile imports dynamic CRT')
    cap = 0x1000 if comparison_md else 0x1800
    print('DATA_FOOTPRINT: REPORT')
    print('profile:', 'HISTORICAL_MD_COMPARISON' if comparison_md else 'SHIPPING_MT_STATIC_FPTABLE')
    print(f'.data VirtualSize=0x{data["vs"]:X} ({data["vs"]/1048576:.3f} MiB)')
    print(f'.data RawSize=0x{data["raw"]:X} ({data["raw"]} bytes)')
    if data['vs'] >= 0x400000: reject('persistent .data exceeds 4 MiB storage-convergence ceiling')
    if data['raw'] > cap: reject(f'.data initialized raw payload exceeds bounded {hex(cap)} budget')
    print('DATA_FOOTPRINT: PASS - verified profile-specific raw ceiling and storage-convergence regression ceiling')
    print('NOTE: not a byte-parity target; separate structural, source, runtime and memory audits remain mandatory')


if __name__ == '__main__':
    if len(sys.argv) not in (2, 3) or (len(sys.argv)==3 and sys.argv[2] != '--comparison-md'):
        print('usage: check_data_footprint.py <preloader_l.dll> [--comparison-md]')
        sys.exit(2)
    check(sys.argv[1], len(sys.argv)==3)
