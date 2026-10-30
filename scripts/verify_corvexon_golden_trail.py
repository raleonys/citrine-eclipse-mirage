#!/usr/bin/env python3
from __future__ import annotations
from pathlib import Path
import argparse, fnmatch, hashlib, json, struct, subprocess, sys
from golden_policy import load_target

class PEError(RuntimeError):
    pass

def u16(b,o):
    if o < 0 or o+2 > len(b): raise PEError(f'truncated u16 at 0x{o:X}')
    return struct.unpack_from('<H',b,o)[0]
def u32(b,o):
    if o < 0 or o+4 > len(b): raise PEError(f'truncated u32 at 0x{o:X}')
    return struct.unpack_from('<I',b,o)[0]
def u64(b,o):
    if o < 0 or o+8 > len(b): raise PEError(f'truncated u64 at 0x{o:X}')
    return struct.unpack_from('<Q',b,o)[0]

def require_optional_span(b,opt,optsz,rel,size,what):
    if rel < 0 or size < 0 or rel > optsz or size > optsz-rel:
        raise PEError(f'{what} outside declared optional header (SizeOfOptionalHeader=0x{optsz:X})')
    if opt+optsz > len(b): raise PEError('declared optional header exceeds file bounds')
    return opt+rel
def cstr(b,o):
    if o < 0 or o >= len(b): raise PEError(f'string offset outside file: 0x{o:X}')
    e=b.find(b'\0',o)
    if e < 0: raise PEError(f'unterminated string at 0x{o:X}')
    return b[o:e].decode('ascii','strict')
def sha256(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda:f.read(1024*1024),b''): h.update(chunk)
    return h.hexdigest()
def hx(v): return int(v,16) if isinstance(v,str) else int(v)

def compute_pe_checksum(data, checksum_offset):
    total=0
    n=len(data)
    for i in range(0,n,2):
        if checksum_offset <= i < checksum_offset+4:
            word=0
        else:
            lo=data[i]
            hi=data[i+1] if i+1<n else 0
            word=lo | (hi<<8)
        total += word
        total=(total & 0xFFFF) + (total >> 16)
    total=(total & 0xFFFF) + (total >> 16)
    return (total + n) & 0xFFFFFFFF

class PE:
    def __init__(self,path:Path):
        self.path=path; self.b=path.read_bytes(); b=self.b
        if len(b)<0x100 or b[:2]!=b'MZ': raise PEError('missing MZ')
        self.pe=u32(b,0x3c)
        if self.pe+24>len(b) or b[self.pe:self.pe+4]!=b'PE\0\0': raise PEError('missing PE signature')
        self.coff=self.pe+4; self.opt=self.coff+20
        self.nsec=u16(b,self.coff+2); self.optsz=u16(b,self.coff+16)
        require_optional_span(b,self.opt,self.optsz,0,2,'OptionalHeader.Magic')
        self.magic=u16(b,self.opt)
        if self.magic==0x20B: dd_rel=112
        elif self.magic==0x10B: dd_rel=96
        else: raise PEError(f'unsupported optional-header magic 0x{self.magic:04X}')
        require_optional_span(b,self.opt,self.optsz,dd_rel,16*8,'DataDirectory[16]')
        self.dd=self.opt+dd_rel
        self.sections=[]; so=self.opt+self.optsz
        for i in range(self.nsec):
            o=so+i*40
            if o+40>len(b): raise PEError('truncated section table')
            name=b[o:o+8].split(b'\0',1)[0].decode('ascii','replace')
            vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8)
            chars=u32(b,o+36)
            self.sections.append((name,va,max(vs,rs),rp,rs,chars))
    def directory(self,i): return u32(self.b,self.dd+i*8),u32(self.b,self.dd+i*8+4)
    def rva2off(self,rva):
        for _n,va,span,rp,rs,_chars in self.sections:
            if va<=rva<va+span:
                delta=rva-va
                if delta>=rs and rs!=0: raise PEError(f'RVA 0x{rva:X} maps into virtual-only bytes')
                o=rp+delta
                if o>=len(self.b): raise PEError(f'RVA 0x{rva:X} maps beyond EOF')
                return o
        # Header RVA.
        if rva < self.opt+self.optsz+self.nsec*40: return rva
        raise PEError(f'unmapped RVA 0x{rva:X}')
    def exports(self):
        rva,size=self.directory(0)
        if not rva or not size: raise PEError('export directory absent')
        eo=self.rva2off(rva)
        if eo+40>len(self.b): raise PEError('truncated export directory')
        base=u32(self.b,eo+16); nf=u32(self.b,eo+20); nn=u32(self.b,eo+24)
        funcs=self.rva2off(u32(self.b,eo+28)); names=self.rva2off(u32(self.b,eo+32)); ords=self.rva2off(u32(self.b,eo+36))
        if funcs+nf*4>len(self.b) or names+nn*4>len(self.b) or ords+nn*2>len(self.b): raise PEError('truncated export tables')
        self.export_counts=(nf,nn)
        self.export_function_rvas=[u32(self.b,funcs+i*4) for i in range(nf)]
        self.export_named_indices=[]
        out={}
        for i in range(nn):
            name=cstr(self.b,self.rva2off(u32(self.b,names+i*4)))
            idx=u16(self.b,ords+i*2)
            if idx>=nf: raise PEError(f'export ordinal index outside function table: {name}')
            self.export_named_indices.append(idx)
            frva=u32(self.b,funcs+idx*4); ordinal=base+idx
            forwarded=(rva <= frva < rva+size)
            out[name]=(ordinal,frva,forwarded)
        return out
    def import_names(self, module_name):
        """Read the exact PE import-name table for one module (no ordinal shortcuts)."""
        rva,size=self.directory(1)
        if not rva or size<20: return []
        wanted=module_name.upper(); result=[]; found=False; i=0
        while True:
            drva=rva+i*20
            if drva+20>rva+size: raise PEError('import descriptor extends outside declared directory')
            o=self.rva2off(drva)
            if o+20>len(self.b): raise PEError('truncated import descriptor')
            oft,ts,fc,nr,ft=struct.unpack_from('<IIIII',self.b,o)
            if (oft|ts|fc|nr|ft)==0: break
            if not nr: raise PEError('import descriptor has null Name RVA')
            if cstr(self.b,self.rva2off(nr)).upper()==wanted:
                found=True
                table_rva=oft or ft
                if not table_rva: raise PEError(wanted+' has no thunk table')
                entry_size=8 if self.magic==0x20B else 4
                high_bit=(1<<63) if entry_size==8 else (1<<31)
                for idx in range(4096):
                    thunk=self.rva2off(table_rva+idx*entry_size)
                    value=u64(self.b,thunk) if entry_size==8 else u32(self.b,thunk)
                    if value==0: break
                    if value & high_bit:
                        result.append('#'+str(value & 0xFFFF))
                    else:
                        nameoff=self.rva2off(value)+2  # IMAGE_IMPORT_BY_NAME.Hint
                        result.append(cstr(self.b,nameoff))
                else: raise PEError(wanted+' import thunk runaway')
            i+=1
            if i>4096: raise PEError('import descriptor runaway')
        return result if found else []

    def imports(self):
        rva,size=self.directory(1)
        if not rva and not size: return []
        if not rva or size<20: raise PEError('invalid import directory extent')
        start=self.rva2off(rva); end_rva=rva+size
        modules=[]; i=0
        while True:
            drva=rva+i*20
            if drva+20>end_rva: raise PEError('import descriptor extends outside declared directory')
            o=self.rva2off(drva)
            if o+20>len(self.b): raise PEError('truncated import descriptor')
            oft,ts,fc,nr,ft=struct.unpack_from('<IIIII',self.b,o)
            if (oft|ts|fc|nr|ft)==0: break
            if not nr: raise PEError('import descriptor has null Name RVA')
            modules.append(cstr(self.b,self.rva2off(nr)).upper())
            i+=1
            if i>4096: raise PEError('import descriptor runaway')
        return modules

def validate_restricted_named_imports(pe, import_policy, modules):
    errors=[]
    for module,expected in import_policy.get('restricted_named_imports',{}).items():
        if module.upper() in modules:
            actual=pe.import_names(module)
            if sorted(actual)!=sorted(expected):
                errors.append(f'{module.upper()} import names={actual!r}, expected={expected!r}')
    return errors

def inspect_shipping_abi(path:Path,policy:dict):
    pe=PE(path); errors=[]; req=policy['required_core']
    machine=u16(pe.b,pe.coff); subsystem=u16(pe.b,pe.opt+68); entry=u32(pe.b,pe.opt+16)
    integrity=policy.get('integrity_policy',{})
    stored_checksum=u32(pe.b,pe.opt+64)
    computed_checksum=compute_pe_checksum(pe.b,pe.opt+64)
    if integrity.get('require_pe_checksum_valid') and stored_checksum != computed_checksum:
        errors.append(f'PE CheckSum=0x{stored_checksum:08X}, recomputed=0x{computed_checksum:08X}')
    if machine!=hx(req['machine']): errors.append(f'machine=0x{machine:04X}')
    if pe.magic!=hx(req['pe_format_magic']): errors.append(f'PE magic=0x{pe.magic:04X}')
    if subsystem!=int(req['subsystem']): errors.append(f'subsystem={subsystem}')
    ex=pe.exports(); wanted={x['name']:x for x in policy['shipping_exports']}
    if policy.get('exact_named_export_set'):
        nf,nn=pe.export_counts
        if nf!=nn or nf!=len(wanted):
            errors.append(f'export function/name counts NumberOfFunctions={nf} NumberOfNames={nn} expected={len(wanted)}')
        if set(pe.export_named_indices)!=set(range(nf)):
            errors.append(f'ordinal-only or unnamed export slot(s) present: named_indices={sorted(set(pe.export_named_indices))} functions={nf}')
        if any(rva==0 for rva in pe.export_function_rvas):
            errors.append('null export function slot present')
        if set(ex)!=set(wanted):
            errors.append(f'export name set={sorted(ex)} expected={sorted(wanted)}')
    for name,spec in wanted.items():
        got=ex.get(name)
        if got is None:
            errors.append(f'missing export {name}'); continue
        ordinal,rva,forwarded=got
        if ordinal!=int(spec['ordinal']): errors.append(f'{name} ordinal={ordinal}, expected={spec["ordinal"]}')
        if policy.get('forbid_forwarded_exports') and forwarded: errors.append(f'{name} is forwarded')
        if spec.get('must_equal_entrypoint') and rva!=entry:
            errors.append(f'{name} RVA 0x{rva:X} != AddressOfEntryPoint 0x{entry:X}')
        if spec.get('must_not_equal_entrypoint') and rva==entry:
            errors.append(f'{name} RVA 0x{rva:X} unexpectedly equals AddressOfEntryPoint; normalized CRT startup was bypassed')
    entry_section=None; entry_characteristics=None
    for sec_name,va,span,_rp,_rs,chars in pe.sections:
        if va <= entry < va+span:
            entry_section=sec_name; entry_characteristics=chars; break
    if entry_section != '.text':
        errors.append(f'AddressOfEntryPoint 0x{entry:X} is not in .text (section={entry_section!r})')
    elif policy.get('require_entrypoint_executable_section',True) and not (entry_characteristics & 0x20000000):
        errors.append(f'AddressOfEntryPoint 0x{entry:X} resides in non-executable .text (Characteristics=0x{entry_characteristics:08X})')
    modules=pe.imports(); modset=set(modules); ip=policy['import_policy']
    if ip.get('module_order') is not None and modules!=[x.upper() for x in ip['module_order']]:
        errors.append('import module order differs: '+str(modules))
    missing=[x for x in ip['required_modules'] if x.upper() not in modset]
    if missing: errors.append('missing required import modules: '+', '.join(missing))
    allowed={x.upper() for x in ip['allowed_modules']}
    runtime_globs=[x.upper() for x in ip.get('crt_runtime_policy',{}).get('allowed_runtime_modules',[]) if '*' in x]
    unexpected=sorted(mod for mod in modset if mod not in allowed and not any(fnmatch.fnmatch(mod,g) for g in runtime_globs))
    if unexpected: errors.append('unexpected import modules: '+', '.join(unexpected))
    # HF13: Allow MSWSOCK only for the exact Asio IOCP AcceptEx surface.
    errors.extend(validate_restricted_named_imports(pe,ip,modset))
    for mod in sorted(modset):
        for prefix in ip.get('forbidden_module_prefixes',[]):
            if mod.startswith(prefix.upper()): errors.append(f'forbidden import module: {mod}')
    crt=ip.get('crt_runtime_policy',{})
    # HF12: static CRT makes the *absence* of runtime imports authoritative.
    for mod in sorted(modset):
        if any(mod.startswith(p.upper()) for p in crt.get('forbidden_runtime_families',[])):
            errors.append('static CRT forbidden runtime import: '+mod)

    gs=ip.get('gs_runtime_policy',{})
    cookie_info={'required':False,'present':False,'rva':None,'load_config_present':False}
    load_rva,load_size=pe.directory(10)
    cookie_info['required']=bool(gs.get('require_pe_load_config_security_cookie'))
    if load_rva:
        cookie_info['load_config_present']=True
        if load_size < 96:
            errors.append(f'/GS Load Config present but truncated: RVA=0x{load_rva:X} Size=0x{load_size:X}')
        else:
            lo=pe.rva2off(load_rva)
            declared_size=u32(pe.b,lo)
            if declared_size >= 96:
                image_base=u64(pe.b,pe.opt+24) if pe.magic==0x20B else u32(pe.b,pe.opt+28)
                image_size=u32(pe.b,pe.opt+56)
                cookie_va=u64(pe.b,lo+88) if pe.magic==0x20B else u32(pe.b,lo+60)
                if cookie_va:
                    if cookie_va < image_base or cookie_va >= image_base+image_size:
                        errors.append(f'/GS SecurityCookie VA 0x{cookie_va:X} outside image [0x{image_base:X},0x{image_base+image_size:X})')
                    else:
                        cookie_rva=cookie_va-image_base
                        try:
                            pe.rva2off(cookie_rva)
                        except PEError as e:
                            errors.append('/GS SecurityCookie is not backed by mapped file storage: '+str(e))
                        else:
                            cookie_section=None; cookie_characteristics=None
                            for sec_name,va,span,_rp,_rs,chars in pe.sections:
                                if va <= cookie_rva < va+span:
                                    cookie_section=sec_name; cookie_characteristics=chars; break
                            if cookie_section is None:
                                errors.append(f'/GS SecurityCookie RVA 0x{cookie_rva:X} is not owned by a PE section')
                            elif gs.get('require_security_cookie_writable_section',True) and not (cookie_characteristics & 0x80000000):
                                errors.append(f'/GS SecurityCookie RVA 0x{cookie_rva:X} resides in non-writable section {cookie_section} (Characteristics=0x{cookie_characteristics:08X})')
                            else:
                                cookie_info.update({'present':True,'rva':cookie_rva,'section':cookie_section,'characteristics':cookie_characteristics})
    if cookie_info['required'] and not cookie_info['present']:
        errors.append('normalized /GS image lacks required nonzero image-backed Load Config SecurityCookie')
    return errors, {'exports':ex,'imports':sorted(modset),'entrypoint_rva':entry,'entrypoint_section':entry_section,'entrypoint_characteristics':entry_characteristics,'machine':machine,'magic':pe.magic,'subsystem':subsystem,'gs_cookie':cookie_info,'pe_checksum':stored_checksum}

def run_gate(cmd,label):
    cp=subprocess.run(cmd,text=True,capture_output=True)
    out=(cp.stdout or '')+(cp.stderr or '')
    if cp.returncode!=0:
        raise RuntimeError(label+' failed\n'+out.rstrip())
    return out.rstrip()

def main(argv=None):
    ap=argparse.ArgumentParser(description='CORVEXON Golden Trail: read-only final shipping-DLL acceptance gate.')
    ap.add_argument('dll',type=Path)
    ap.add_argument('--build-dir',type=Path,required=True,help='CMake build directory used by the shipping link-policy gate')
    args=ap.parse_args(argv)
    root=Path(__file__).resolve().parents[1]
    try:
        policy=load_target(root,'CORVEXON_GOLDEN_TRAIL.json')
        before=sha256(args.dll)
        aryra=run_gate([sys.executable,'-B',str(root/'scripts/verify_aryra_golden_layout.py'),str(args.dll)],'ARYRA')
        mustavex=run_gate([sys.executable,'-B',str(root/'scripts/verify_mustavex_golden_metadata.py'),str(args.dll)],'MUSTAVEX')
        link=run_gate([sys.executable,'-B',str(root/'scripts/check_msvc_shipping_link_command.py'),str(args.build_dir)],'MSVC shipping link policy')
        errors,info=inspect_shipping_abi(args.dll,policy)
        if errors: raise RuntimeError('\n'.join(errors))
    except (OSError,PEError,struct.error,UnicodeError,json.JSONDecodeError,RuntimeError,KeyError,TypeError,ValueError) as e:
        print('CORVEXON_GOLDEN_TRAIL: FAIL',file=sys.stderr)
        for line in str(e).splitlines(): print(' - '+line,file=sys.stderr)
        return 1
    try:
        after=sha256(args.dll)
    except OSError as e:
        print('CORVEXON_GOLDEN_TRAIL: FAIL',file=sys.stderr)
        print(' - '+str(e),file=sys.stderr)
        return 1
    if before!=after:
        print('CORVEXON_GOLDEN_TRAIL: FAIL',file=sys.stderr)
        print(' - verifier changed final DLL bytes',file=sys.stderr)
        print(f' - before={before}',file=sys.stderr); print(f' - after ={after}',file=sys.stderr)
        return 1
    print('CORVEXON_GOLDEN_TRAIL: PASS')
    print(' mode: READ_ONLY_FAIL_CLOSED_POST_FINALIZATION_ACCEPTANCE')
    print(' F1411-001: CLOSED_BY_FINAL_BINARY_ACCEPTANCE_GATE')
    print(' sha256_immutable: '+before)
    print(' exports: '+' '.join(f'{n}@{info["exports"][n][0]}' for n in sorted(info['exports'],key=lambda x:info['exports'][x][0])))
    print(' entrypoint: executable CRT startup in .text; distinct from exported DllMain@1')
    print(' imports: '+(' '.join(info['imports']) if info['imports'] else '<none>'))
    print('/GS LoadConfig SecurityCookie: VALID WRITABLE %s RVA=0x%X' % (info['gs_cookie'].get('section','<unknown>'),info['gs_cookie']['rva']))
    print(' PE CheckSum: VALID 0x%08X (URSYNEX-owned; CORVEXON read-only verified)' % info['pe_checksum'])
    print(' upstream: ARYRA PASS; MUSTAVEX PASS; MSVC shipping link policy PASS')
    return 0

if __name__=='__main__': raise SystemExit(main())
