#!/usr/bin/env python3
"""GF10 independent PE32+ x64 export-table shape gate, NOT native execution.
No third-party parsing library; fail closed on ordinal-only / forwarded exports.
"""
import argparse,hashlib,json,pathlib,struct,sys
class PEError(ValueError): pass
def fail(msg): raise PEError(msg)
def check(data):
    def span(off, n):
        if off<0 or n<0 or off>len(data) or n>len(data)-off: fail('TRUNCATED')
        return data[off:off+n]
    def u16(o):return struct.unpack('<H',span(o,2))[0]
    def u32(o):return struct.unpack('<I',span(o,4))[0]
    if span(0,2)!=b'MZ':fail('NO_MZ')
    pe=u32(0x3c)
    if pe<0x40 or pe+24>len(data) or span(pe,4)!=b'PE\0\0':fail('NO_PE')
    coff=pe+4
    if u16(coff)!=0x8664:fail('NOT_AMD64')
    nsec=u16(coff+2)
    if not 1<=nsec<=96:fail('SECTION_COUNT')
    optsize=u16(coff+16); opt=coff+20
    if optsize<120 or u16(opt)!=0x20b:fail('NOT_PE32_PLUS')
    span(opt,optsize)
    size_hdr=u32(opt+60)
    if size_hdr>len(data):fail('BAD_HEADERS')
    if u32(opt+108)<1:fail('EXPORT_DIRECTORY_MISSING')
    exr,exlen=u32(opt+112),u32(opt+116)
    if not exr or exlen<40:fail('EXPORT_DIRECTORY_EMPTY')
    secstart=opt+optsize
    sections=[]
    for i in range(nsec):
        o=secstart+40*i;span(o,40)
        vsize,va,rawsize,rawptr=struct.unpack('<4I',span(o+8,16))
        span(rawptr,rawsize)
        sections.append((va,vsize,rawsize,rawptr))
    def rva(rv,n):
        if rv<0 or n<0:fail('RVA_RANGE')
        if rv<size_hdr:
            if rv+n>size_hdr:fail('RVA_BAD_HEADERS')
            return rv
        matches=[(ptr+(rv-va)) for va,vs,rs,ptr in sections if rv>=va and rv-va<=rs and n<=rs-(rv-va)]
        if len(matches)!=1:fail('RVA_NOT_UNIQUE_FILE_BACKED')
        span(matches[0],n)
        return matches[0]
    def name(rv):
        start=rva(rv,1)
        end=data.find(b'\0',start,min(len(data),start+512))
        if end<0:fail('NAME_NO_NUL')
        # Protect section boundary: every source byte must stay file-backed.
        rva(rv,end-start+1)
        try:return data[start:end].decode('ascii')
        except UnicodeDecodeError:fail('NAME_ENCODING')
    edir=rva(exr,40)
    (_flags,_date,_maj,_min,dllname,base,nfunc,nnames,func_rva,names_rva,ord_rva)=struct.unpack('<IIHHIIIIIII',span(edir,40))
    if base!=1 or nfunc!=3 or nnames!=3:fail('EXPORT_COUNT_ORDINAL_ONLY_OR_WRONG_BASE')
    if not all((func_rva,names_rva,ord_rva,dllname)):fail('EXPORT_POINTER_NULL')
    name(dllname)
    funcs=struct.unpack('<3I',span(rva(func_rva,12),12))
    names=struct.unpack('<3I',span(rva(names_rva,12),12))
    ords=struct.unpack('<3H',span(rva(ord_rva,6),6))
    expected={0:'DllMain',1:'SURGE',2:'DllShutdown'}
    observed={}
    for nr,oi in zip(names,ords):
        if oi not in expected or oi in observed:fail('DUPLICATE_OR_BAD_ORDINAL')
        observed[oi]=name(nr)
    if observed!=expected:fail('EXPORT_NAMES_MISMATCH')
    for f in funcs:
        if not f:fail('EXPORT_NULL_FUNCTION')
        if exr<=f<exr+exlen:fail('FORWARDED_EXPORT')
        rva(f,1)
    return {'machine':'AMD64','pe':'PE32+','dll_exports':[{'ordinal':i+1,'name':expected[i], 'rva':hex(funcs[i])} for i in range(3)]}

def main():
    a=argparse.ArgumentParser();a.add_argument('dll',type=pathlib.Path);a.add_argument('--json',type=pathlib.Path);args=a.parse_args()
    try:
        data=args.dll.read_bytes();result=check(data);result['dll_sha256']=hashlib.sha256(data).hexdigest()
        if args.json:args.json.write_text(json.dumps(result,indent=2)+'\n')
        print('GF10_PE_EXPORT_GATE_PASS',result['dll_sha256']);return 0
    except (PEError,IOError,ValueError,struct.error) as e:
        print('GF10_PE_EXPORT_GATE_FAIL',type(e).__name__,str(e));return 1
if __name__=='__main__':sys.exit(main())
