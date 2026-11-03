# SPDX-License-Identifier: GPL-3.0-only

"""Bounded PE32+ import-descriptor permutation; all thunk/name/IAT bytes stay intact."""
import struct


def descriptors(data):
    def read(fmt, offset):
        if offset < 0 or offset + struct.calcsize(fmt) > len(data):
            raise ValueError('import-order PE field outside file')
        return struct.unpack_from(fmt, data, offset)
    if data[:2] != b'MZ':raise ValueError('import-order input has no MZ')
    pe,=read('<I',0x3c)
    if data[pe:pe+4]!=b'PE\0\0':raise ValueError('import-order input has no PE signature')
    coff=pe+4;opt=coff+20;optsz,=read('<H',coff+16)
    if optsz<128 or opt+optsz>len(data):raise ValueError('import directory outside declared optional header')
    if read('<H',opt)[0]!=0x20b or read('<I',opt+108)[0]<2:raise ValueError('import-order requires PE32+ import directory')
    sections=[]
    for i in range(read('<H',coff+2)[0]):
        vs,va,rs,rp=read('<IIII',opt+optsz+i*40+8)
        if rp+rs>len(data):raise ValueError('import-order section outside file')
        sections.append((va,rs,rp))
    def mapped(rva,size=1):
        matches=[(rp+rva-va,rp+rs) for va,rs,rp in sections if va<=rva and rva-va+size<=rs]
        if len(matches)!=1:raise ValueError('import-order RVA is not uniquely raw-backed')
        return matches[0]
    rva,size=read('<II',opt+120)
    if not rva or size<20:raise ValueError('import-order input has no complete import directory')
    offset,limit=mapped(rva,20);records=[];names=[]
    for i in range(size//20):
        current,end=mapped(rva+i*20,20)
        if current!=offset+i*20 or end!=limit:raise ValueError('import descriptors are not contiguous')
        record=bytes(data[current:current+20])
        if record==bytes(20):return offset,records,names
        name_rva=read('<IIIII',current)[3]
        if not name_rva:raise ValueError('import descriptor has no module name')
        name_offset,name_limit=mapped(name_rva)
        nul=data.find(b'\0',name_offset,name_limit)
        if nul<0:raise ValueError('unterminated import module name')
        name=bytes(data[name_offset:nul]).decode('ascii').upper()
        if name in names:raise ValueError('duplicate import module descriptor')
        names.append(name);records.append(record)
    raise ValueError('import directory has no bounded null terminator')


def reorder(data, expected):
    expected=[name.upper() for name in expected]
    if len(set(expected))!=len(expected):raise ValueError('duplicate target import module')
    offset,records,names=descriptors(data)
    if set(names)!=set(expected) or len(names)!=len(expected):
        raise ValueError('import-order module set differs: actual='+str(names)+' expected='+str(expected))
    by_name=dict(zip(names,records))
    data[offset:offset+20*len(records)]=b''.join(by_name[name] for name in expected)
    return {'before':names,'after':expected,'descriptor_offset':offset,'descriptor_bytes':20*len(records)}


def product_order(root):
    """One exact import contract for unsigned and Golden SURGE application DLLs."""
    import json
    from pathlib import Path
    root = Path(root)
    gold = json.loads((root / 'metadata/URSYNEX_GOLDEN_FINALIZER.json').read_text(encoding='utf-8'))
    cor = json.loads((root / 'metadata/CORVEXON_GOLDEN_TRAIL.json').read_text(encoding='utf-8'))
    dist = json.loads((root / 'metadata/DIST_CONTRACT.json').read_text(encoding='utf-8'))
    ordered = gold['import_module_order']
    if (not isinstance(ordered, list) or not ordered or
            len(set(ordered)) != len(ordered) or
            ordered != cor['import_policy']['module_order'] or
            ordered != cor['import_policy']['required_modules'] or
            ordered != cor['import_policy']['allowed_modules'] or
            ordered != dist['final_import_descriptor_order']):
        raise ValueError('Unsigned and Golden application import policies differ')
    return ordered


def normalize_application(path, expected, verify=False):
    """Change only complete PE import descriptors; fail without writes on a bad set."""
    import os
    import tempfile
    from pathlib import Path
    path = Path(path)
    original = path.read_bytes()
    data = bytearray(original)
    result = reorder(data, expected)
    if verify:
        if result['before'] != expected:
            raise ValueError('Unsigned application import order differs: ' + str(result['before']))
        return result
    start = result['descriptor_offset']
    end = start + result['descriptor_bytes']
    if original[:start] != data[:start] or original[end:] != data[end:] or len(original) != len(data):
        raise ValueError('Import normalizer modified bytes outside the descriptor table')
    if data != original:
        # A failed validation must never publish a partially rewritten DLL.
        temp = None
        try:
            with tempfile.NamedTemporaryFile(
                    mode='wb', prefix=path.name + '.order-', suffix='.tmp',
                    dir=path.parent, delete=False) as output:
                temp = Path(output.name)
                output.write(data)
                output.flush()
                os.fsync(output.fileno())
            os.replace(temp, path)
        finally:
            if temp is not None:
                temp.unlink(missing_ok=True)
    if descriptors(path.read_bytes())[2] != expected:
        raise ValueError('Unsigned application DLL was not normalized')
    return result


def main():
    import argparse
    from pathlib import Path
    parser = argparse.ArgumentParser(
        description='Normalize or verify the unsigned application DLL import descriptors')
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--dll', type=Path, required=True)
    parser.add_argument('--verify', action='store_true', help='Fail without modifying the DLL')
    args = parser.parse_args()
    try:
        result = normalize_application(args.dll, product_order(args.root), args.verify)
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print('UNSIGNED_IMPORT_ORDER: FAIL (' + str(exc) + ')')
        return 1
    print('UNSIGNED_IMPORT_ORDER: PASS (post-link unsigned image; before=' +
          str(result['before']) + '; after=' + str(result['after']) + ')')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
