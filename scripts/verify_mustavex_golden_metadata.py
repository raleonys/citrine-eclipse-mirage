#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
from pathlib import Path
import hashlib, struct, sys
from golden_policy import load_target

class PEError(RuntimeError): pass

def u16(b,o):
    if o<0 or o+2>len(b): raise PEError(f'truncated u16 at 0x{o:X}')
    return struct.unpack_from('<H',b,o)[0]
def u32(b,o):
    if o<0 or o+4>len(b): raise PEError(f'truncated u32 at 0x{o:X}')
    return struct.unpack_from('<I',b,o)[0]

def require_optional_span(b,opt,optsz,rel,size,what):
    if rel < 0 or size < 0 or rel > optsz or size > optsz-rel:
        raise PEError(f'{what} outside declared optional header (SizeOfOptionalHeader=0x{optsz:X})')
    if opt+optsz > len(b): raise PEError('declared optional header exceeds file bounds')
    return opt+rel
def align4(x): return (x+3)&~3

def read_wstr(data,pos):
    chars=[]
    while pos+2<=len(data):
        c=struct.unpack_from('<H',data,pos)[0]; pos+=2
        if c==0: break
        chars.append(chr(c))
    return ''.join(chars),pos

def parse_ver_node(data,off=0,end=None):
    if end is None: end=len(data)
    if off+6>end: return None,off
    wlen,wvallen,wtype=struct.unpack_from('<HHH',data,off)
    if not wlen or off+wlen>end: return None,off
    key,pos=read_wstr(data,off+6); pos=align4(pos)
    nbytes=wvallen*2 if wtype==1 else wvallen
    vb=data[pos:pos+nbytes] if wvallen else b''
    value=vb.decode('utf-16le','replace').rstrip('\x00') if wvallen and wtype==1 else None
    pos=align4(pos+nbytes)
    children=[]
    while pos<off+wlen:
        child,nxt=parse_ver_node(data,pos,off+wlen)
        if not child or nxt<=pos: break
        children.append(child); pos=align4(nxt)
    return {'key':key,'value':value,'value_bytes':vb,'children':children},off+wlen

def hx(v): return int(v,16) if isinstance(v,str) else int(v)


def validate_resource_leaves(leaves, req):
    """Keep VERSIONINFO strict; additionally allow only explicitly byte-pinned assets."""
    errors=[]
    version_path=[16,1,hx(req['versioninfo']['translation_language'])]
    assets=req.get('additional_resource_assets',[])
    expected=[version_path]+[x['path'] for x in assets]
    paths=[x[0] for x in leaves]
    if len(paths)!=len(expected) or {tuple(p) for p in paths}!={tuple(p) for p in expected}:
        errors.append(f'resource allowlist mismatch: {paths}')
    for path,data,_cp in leaves:
        if path==version_path:
            root,_=parse_ver_node(data)
            errors.extend(validate_versioninfo_root(root,req))
        else:
            spec=next((x for x in assets if x['path']==path),None)
            if spec and hashlib.sha256(data).hexdigest()!=spec['sha256']:
                errors.append(f'resource byte pin mismatch: {path}')
    return errors


def validate_versioninfo_root(root, req):
    errors=[]
    if not root or root.get('key')!='VS_VERSION_INFO':
        return ['invalid VERSIONINFO root']
    vb=root.get('value_bytes',b''); fixed=req['versioninfo']['fixed']
    if len(vb)<52:
        errors.append('truncated VS_FIXEDFILEINFO')
    else:
        vals=struct.unpack_from('<13I',vb,0); keys=list(fixed)
        got=dict(zip(keys,vals))
        for k,v in fixed.items():
            if got.get(k)!=hx(v): errors.append(f'{k}=0x{got.get(k,0):08X}')
    strings={}; translations=[]; string_tables=[]
    def recur(n,parents=()):
        k=n.get('key')
        if parents and parents[-1]=='StringFileInfo': string_tables.append(k)
        if len(parents)>=2 and parents[-2]=='StringFileInfo':
            # A dict comparison must not silently collapse ambiguous resource
            # records, even when duplicate values happen to be identical.
            if k in strings:
                errors.append(f'Duplicate VERSIONINFO StringFileInfo key: {k}')
            strings[k]=n.get('value')
        if k=='Translation':
            z=n.get('value_bytes',b'')
            for i in range(0,len(z)-3,4): translations.append(struct.unpack_from('<HH',z,i))
        for c in n.get('children',[]): recur(c,parents+(k,))
    recur(root)
    expected_table=req['versioninfo']['string_table']
    if string_tables!=[expected_table]: errors.append(f'StringTable={string_tables!r}')
    if strings!=req['versioninfo']['strings']: errors.append(f'StringFileInfo={strings!r}')
    expected_translation=(hx(req['versioninfo']['translation_language']),req['versioninfo']['translation_codepage'])
    if translations!=[expected_translation]: errors.append(f'Translation={translations!r}')
    return errors

def inspect(path,policy):
    b=path.read_bytes(); errors=[]; req=policy['required']
    if len(b)<0x40 or b[:2]!=b'MZ': raise PEError('missing MZ')
    pe=u32(b,0x3c)
    if pe+24>len(b) or b[pe:pe+4]!=b'PE\0\0': raise PEError('missing PE signature')
    coff=pe+4; nsec=u16(b,coff+2); optsz=u16(b,coff+16); opt=coff+20
    require_optional_span(b,opt,optsz,0,2,'OptionalHeader.Magic')
    magic=u16(b,opt)
    require_optional_span(b,opt,optsz,2,2,'LinkerVersion')
    require_optional_span(b,opt,optsz,40,4,'OSVersion')
    require_optional_span(b,opt,optsz,48,4,'SubsystemVersion')
    require_optional_span(b,opt,optsz,68,2,'Subsystem')
    checks=[
      ('Machine',u16(b,coff)==hx(req['machine']),f'0x{u16(b,coff):04X}'),
      ('PE format',magic==hx(req['pe_format_magic']),f'0x{magic:04X}'),
      ('LinkerVersion',req['linker_version']=='PRESERVE_INPUT' or (b[opt+2],b[opt+3])==tuple(req['linker_version']),f'{b[opt+2]}.{b[opt+3]}'),
      ('TimeDateStamp',req['timestamp']=='PRESERVE_INPUT' or u32(b,coff+4)==hx(req['timestamp']),f'0x{u32(b,coff+4):08X}'),
      ('OS version',req['os_version']=='PRESERVE_INPUT' or (u16(b,opt+40),u16(b,opt+42))==tuple(req['os_version']),f'{u16(b,opt+40)}.{u16(b,opt+42)}'),
      ('Subsystem version',req['subsystem_version']=='PRESERVE_INPUT' or (u16(b,opt+48),u16(b,opt+50))==tuple(req['subsystem_version']),f'{u16(b,opt+48)}.{u16(b,opt+50)}'),
      ('Subsystem',u16(b,opt+68)==req['subsystem'],str(u16(b,opt+68))),
    ]
    for name,ok,got in checks:
        if not ok: errors.append(f'{name}: {got}')
    # Shipping debug/provenance policy.
    if u32(b,coff+8)!=0 or u32(b,coff+12)!=0:
        errors.append('COFF symbol table present')
    if magic==0x20B: dd_rel=112
    elif magic==0x10B: dd_rel=96
    else: raise PEError(f'unsupported optional-header magic 0x{magic:04X}')
    require_optional_span(b,opt,optsz,dd_rel,16*8,'DataDirectory[16]')
    dd=opt+dd_rel
    dirs=[(u32(b,dd+i*8),u32(b,dd+i*8+4)) for i in range(16)]
    if dirs[6] != (0,0): errors.append(f'debug directory not stripped: {dirs[6]}')
    if dirs[4] != (0,0): errors.append('Authenticode/security directory present')
    if req.get('rich_header')!='PRESERVE_INPUT' and b.find(b'Rich',0,pe)!=-1: errors.append('Rich Header present')
    forbidden_identity_tokens=[]
    for key in ('project_identity_forbidden_in_shipping_dll','forbidden_project_identity_tokens'):
        forbidden_identity_tokens.extend(policy.get(key,[]))
    ascii_folded=b.lower()
    wide_folded=[]
    for parity in (0,1):
        chunk=b[parity:len(b)-((len(b)-parity)%2)]
        try: wide_folded.append(chunk.decode('utf-16le','ignore').casefold())
        except UnicodeError: wide_folded.append('')
    for token in dict.fromkeys(forbidden_identity_tokens):
        folded=token.casefold()
        hit=folded.encode('ascii') in ascii_folded
        if not hit: hit=any(folded in view for view in wide_folded)
        if hit: errors.append('project release identity embedded: '+token)
    # Section table is used only for RVA mapping. Section names/order/layout are ARYRA's policy.
    sec_off=opt+optsz; secs=[]
    for i in range(nsec):
        o=sec_off+i*40
        if o+40>len(b): raise PEError('truncated section table')
        vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8)
        secs.append((va,vs,rp,rs))
    def rva2off(rva):
        for va,vs,rp,rs in secs:
            if va<=rva<va+max(vs,rs):
                delta=rva-va
                if delta>=rs: raise PEError(f'RVA 0x{rva:X} maps into virtual-only section bytes')
                off=rp+delta
                if off>=len(b): raise PEError(f'RVA 0x{rva:X} maps beyond EOF')
                return off
        if rva<sec_off:return rva
        raise PEError(f'unmapped RVA 0x{rva:X}')
    # #18 internal export name.
    er,_=dirs[0]
    if not er: errors.append('export directory absent')
    else:
        eo=rva2off(er); nr=u32(b,eo+12); no=rva2off(nr); end=b.find(b'\0',no)
        name=b[no:end].decode('ascii','replace') if end!=-1 else ''
        if name!=req['export_internal_name']: errors.append(f'export internal name={name!r}')
    # Strict resource allowlist: exactly VERSIONINFO / 1 / en-US and no other resource class.
    # Audit2 S0108: resource-directory traversal is explicitly bounded and range checked.
    rr,rsize=dirs[2]
    if not rr: errors.append('VERSIONINFO resource absent')
    else:
        if rsize < 16: raise PEError('resource directory too small')
        base=rva2off(rr); leaves=[]
        max_depth=8
        max_directories=256
        max_entries=1024
        visited=set()
        budget={'directories':0,'entries':0}
        def resource_span(rel,size,what):
            if rel<0 or size<0 or rel>rsize or size>rsize-rel:
                raise PEError(f'{what} outside resource directory range')
            o=base+rel
            if o<0 or o>len(b) or size>len(b)-o:
                raise PEError(f'{what} exceeds file bounds')
            return o
        def name_or_id(v):
            if v&0x80000000:
                rel=v&0x7fffffff
                no=resource_span(rel,2,'resource name header'); ln=u16(b,no)
                size=2+ln*2
                no=resource_span(rel,size,'resource name')
                return b[no+2:no+size].decode('utf-16le','replace')
            return v&0xffff
        def walk(rel,path,depth=0):
            if depth>max_depth: raise PEError('resource directory depth budget exceeded')
            if rel in visited: raise PEError('resource directory cycle detected')
            if budget['directories']>=max_directories: raise PEError('resource directory node budget exceeded')
            o=resource_span(rel,16,'resource directory header')
            visited.add(rel); budget['directories']+=1
            try:
                count=u16(b,o+12)+u16(b,o+14)
                if count>max_entries-budget['entries']:
                    raise PEError('resource directory entry budget exceeded')
                resource_span(rel+16,count*8,'resource directory entries')
                budget['entries']+=count
                for i in range(count):
                    e=base+rel+16+i*8; n=name_or_id(u32(b,e)); v=u32(b,e+4); np=path+[n]
                    if v&0x80000000:
                        walk(v&0x7fffffff,np,depth+1)
                    else:
                        de_rel=v&0x7fffffff
                        de=resource_span(de_rel,16,'resource data entry')
                        dr,sz,cp,_=struct.unpack_from('<IIII',b,de)
                        off=rva2off(dr)
                        if off+sz>len(b): raise PEError(f'resource extent for RVA 0x{dr:X} exceeds EOF')
                        leaves.append((np,b[off:off+sz],cp))
            finally:
                visited.remove(rel)
        walk(0,[])
        errors.extend(validate_resource_leaves(leaves,req))
    return errors

def main():
    if len(sys.argv)!=2:
        print('usage: verify_mustavex_golden_metadata.py <dll>',file=sys.stderr); return 2
    root=Path(__file__).resolve().parents[1]
    try:
        policy=load_target(root,'MUSTAVEX_GOLDEN_METADATA.json')
        errors=inspect(Path(sys.argv[1]),policy)
    except (OSError,PEError,struct.error,KeyError,ValueError) as e:
        print(f'MUSTAVEX_GOLDEN_METADATA: FAIL: {e}',file=sys.stderr); return 1
    if errors:
        print('MUSTAVEX_GOLDEN_METADATA: FAIL')
        for e in errors: print(' -',e)
        return 1
    print('MUSTAVEX_GOLDEN_METADATA: PASS')
    print(' curio: #1 #2 #3 #5 #6 #7 #10 #11 #18 #36')
    print(' stripped: #13 RSDS, #14 REPRO; COFF symbols absent')
    print(' excluded: Authenticode/certificate and non-allowlisted recovery targets')
    return 0
if __name__=='__main__': raise SystemExit(main())
