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
