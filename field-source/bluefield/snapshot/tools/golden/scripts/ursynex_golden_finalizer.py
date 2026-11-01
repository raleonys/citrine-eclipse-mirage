#!/usr/bin/env python3
from pathlib import Path
import json, os, struct, sys, tempfile

ROOT = Path(__file__).resolve().parents[1]
FINALIZER_POLICY = None
METADATA_POLICY = None
REQ = None
MUTATION_VALUES = None
TARGET_TIMESTAMP = None
TARGET_LINKER = None
AMD64 = None
PE32_PLUS = None
SUBSYSTEM_WINDOWS_GUI = None

class PEError(RuntimeError): pass

def load_policies():
    global FINALIZER_POLICY, METADATA_POLICY, REQ, MUTATION_VALUES
    global TARGET_TIMESTAMP, TARGET_LINKER, AMD64, PE32_PLUS, SUBSYSTEM_WINDOWS_GUI
    if FINALIZER_POLICY is not None and METADATA_POLICY is not None:
        return
    try:
        finalizer=json.loads((ROOT/'metadata/URSYNEX_GOLDEN_FINALIZER.json').read_text(encoding='utf-8'))
        metadata=json.loads((ROOT/'metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text(encoding='utf-8'))
        req=metadata['required']; mutation=finalizer['mutation_values']
        target_timestamp=int(mutation['timestamp'],16)
        target_linker=tuple(mutation['linker_version'])
        amd64=int(req['machine'],16); pe32_plus=int(req['pe_format_magic'],16)
        subsystem=req['subsystem']
    except (OSError, UnicodeError, json.JSONDecodeError, KeyError, TypeError, ValueError) as e:
        raise PEError(f'policy load failed: {e}') from e
    FINALIZER_POLICY=finalizer; METADATA_POLICY=metadata; REQ=req; MUTATION_VALUES=mutation
    TARGET_TIMESTAMP=target_timestamp; TARGET_LINKER=target_linker
    AMD64=amd64; PE32_PLUS=pe32_plus; SUBSYSTEM_WINDOWS_GUI=subsystem

def _write_file_fully(handle, data):
    written=handle.write(data)
    if written is not None and written != len(data):
        raise OSError(f'short write: {written}/{len(data)}')
    handle.flush()
    os.fsync(handle.fileno())

def atomic_replace_bytes(path, data):
    path=Path(path)
    tmp_path=None
    fd=None
    try:
        mode=path.stat().st_mode & 0o777
        fd,tmp=tempfile.mkstemp(prefix='.'+path.name+'.',suffix='.tmp',dir=str(path.parent))
        tmp_path=Path(tmp)
        try:
            os.fchmod(fd,mode)
        except (AttributeError,OSError):
            pass
        with os.fdopen(fd,'wb',closefd=True) as handle:
            fd=None
            _write_file_fully(handle, data)
        os.replace(tmp_path,path)
        tmp_path=None
    finally:
        if fd is not None:
            try: os.close(fd)
            except OSError: pass
        if tmp_path is not None:
            try: tmp_path.unlink()
            except FileNotFoundError: pass

DEBUG_TYPE_NAMES = {
    0: 'UNKNOWN', 1: 'COFF', 2: 'CODEVIEW', 3: 'FPO', 4: 'MISC',
    5: 'EXCEPTION', 6: 'FIXUP', 7: 'OMAP_TO_SRC', 8: 'OMAP_FROM_SRC',
    9: 'BORLAND', 10: 'RESERVED10', 11: 'CLSID', 12: 'VC_FEATURE',
    13: 'POGO', 14: 'ILTCG', 15: 'MPX', 16: 'REPRO',
    17: 'EMBEDDED_PORTABLE_PDB', 19: 'PDB_CHECKSUM', 20: 'EX_DLLCHARACTERISTICS',
}

def debug_directory_types(b, coff, opt, rva, size):
    if not rva or not size:
        return []
    section_count=u16(b,coff+2); optional_size=u16(b,coff+16)
    sec=opt+optional_size; raw=None
    for i in range(section_count):
        o=sec+i*40
        if o+40>len(b): break
        virtual_size=u32(b,o+8); virtual_address=u32(b,o+12)
        raw_size=u32(b,o+16); raw_ptr=u32(b,o+20)
        span=max(virtual_size,raw_size)
        if virtual_address <= rva < virtual_address+span:
            raw=raw_ptr+(rva-virtual_address); break
    if raw is None or raw<0 or raw+min(size,28)>len(b):
        return ['UNMAPPED']
    out=[]
    count=size//28
    for i in range(count):
        o=raw+i*28
        if o+28>len(b): break
        t=u32(b,o+12)
        out.append(f"{DEBUG_TYPE_NAMES.get(t,'TYPE_'+str(t))}({t})")
    return out

def u16(b,o):
    if o < 0 or o+2 > len(b): raise PEError(f'truncated u16 at 0x{o:X}')
    return struct.unpack_from('<H', b, o)[0]
def u32(b,o):
    if o < 0 or o+4 > len(b): raise PEError(f'truncated u32 at 0x{o:X}')
    return struct.unpack_from('<I', b, o)[0]

def require_optional_span(b,opt,optsz,rel,size,what):
    if optsz < 0 or rel < 0 or size < 0 or rel > optsz or size > optsz-rel:
        raise PEError(f'{what} outside declared optional header (SizeOfOptionalHeader=0x{optsz:X})')
    if opt+optsz > len(b): raise PEError('declared optional header exceeds file bounds')
    return opt+rel

def parse_core(b):
    if len(b) < 0x40 or b[:2] != b'MZ': raise PEError('missing MZ')
    pe=u32(b,0x3c)
    if pe+24 > len(b) or b[pe:pe+4] != b'PE\0\0': raise PEError('missing PE signature')
    coff=pe+4; opt=coff+20; optsz=u16(b,coff+16)
    require_optional_span(b,opt,optsz,0,2,'OptionalHeader.Magic')
    machine=u16(b,coff); magic=u16(b,opt)
    if machine != AMD64: raise PEError(f'machine 0x{machine:04X} != AMD64')
    if magic != PE32_PLUS: raise PEError(f'optional magic 0x{magic:04X} != PE32+')
    require_optional_span(b,opt,optsz,2,2,'LinkerVersion')
    require_optional_span(b,opt,optsz,40,4,'OSVersion')
    require_optional_span(b,opt,optsz,48,4,'SubsystemVersion')
    require_optional_span(b,opt,optsz,64,4,'CheckSum')
    require_optional_span(b,opt,optsz,68,2,'Subsystem')
    if u16(b,opt+68) != SUBSYSTEM_WINDOWS_GUI: raise PEError('subsystem is not Windows GUI')
    if (u16(b,opt+40),u16(b,opt+42)) != tuple(REQ['os_version']): raise PEError('OS version does not match MUSTAVEX policy')
    if (u16(b,opt+48),u16(b,opt+50)) != tuple(REQ['subsystem_version']): raise PEError('subsystem version does not match MUSTAVEX policy')
    # Debug directory must be absent in the shipping binary. Do not rewrite its RVA/size here.
    require_optional_span(b,opt,optsz,112+6*8,8,'DebugDirectory')
    dd=opt+112
    debug_rva=u32(b,dd+6*8); debug_size=u32(b,dd+6*8+4)
    if debug_rva or debug_size:
        types=debug_directory_types(b,coff,opt,debug_rva,debug_size)
        detail=', '.join(types) if types else 'unknown entries'
        raise PEError(f'debug directory present (RVA=0x{debug_rva:X}, size=0x{debug_size:X}, types={detail}); normalized shipping build must emit no Debug Directory')
    return pe,coff,opt

def strip_rich_in_place(b, pe):
    rp=bytes(b).find(b'Rich', 0, pe)
    if rp == -1:
        return False
    if rp+8 > pe: raise PEError('truncated Rich header')
    key=u32(b,rp+4)
    dans=0x536e6144
    start=None
    # Rich records are DWORD-aligned; decode backwards using the Rich XOR key.
    for o in range(rp-4,0x3f,-4):
        if u32(b,o) ^ key == dans:
            start=o; break
    if start is None:
        raise PEError('Rich marker present but encoded DanS start not found')
    for i in range(start, rp+8): b[i]=0
    return True


def validate_policy_bindings():
    load_policies()
    bindings=FINALIZER_POLICY.get('policy_bindings',{})
    if bindings.get('metadata_policy')!='metadata/MUSTAVEX_GOLDEN_METADATA.json':
        raise PEError('URSYNEX metadata-policy binding drift')
    if TARGET_TIMESTAMP != int(REQ['timestamp'],16):
        raise PEError('URSYNEX/MUSTAVEX TimeDateStamp policy drift')
    if TARGET_LINKER != tuple(REQ['linker_version']):
        raise PEError('URSYNEX/MUSTAVEX LinkerVersion policy drift')
    if MUTATION_VALUES.get('rich_header') != REQ.get('rich_header'):
        raise PEError('URSYNEX/MUSTAVEX RichHeader policy drift')
    integ=FINALIZER_POLICY.get('integrity_finalization',{})
    if integ.get('pe_checksum')!='RECOMPUTE_AFTER_APPROVED_MUTATIONS':
        raise PEError('URSYNEX PE checksum ownership drift')
    if integ.get('downstream_validation')!='CORVEXON_READ_ONLY_REQUIRED':
        raise PEError('URSYNEX checksum downstream policy drift')

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
    total=(total + n) & 0xFFFFFFFF
    return total

def main():
    if len(sys.argv)!=2:
        print('usage: ursynex_golden_finalizer.py <dll>', file=sys.stderr); return 2
    p=Path(sys.argv[1])
    try:
        load_policies()
        b=bytearray(p.read_bytes())
        validate_policy_bindings()
        pe,coff,opt=parse_core(b)
        struct.pack_into('<I',b,coff+4,TARGET_TIMESTAMP)
        b[opt+2]=TARGET_LINKER[0]; b[opt+3]=TARGET_LINKER[1]
        stripped=strip_rich_in_place(b,pe)
        checksum_offset=opt+64
        struct.pack_into('<I',b,checksum_offset,0)
        checksum=compute_pe_checksum(b,checksum_offset)
        struct.pack_into('<I',b,checksum_offset,checksum)
        atomic_replace_bytes(p,b)
    except (OSError,PEError,struct.error,UnicodeError,ValueError) as e:
        print(f'URSYNEX_GOLDEN_FINALIZER: FAIL: {e}',file=sys.stderr); return 1
    print('URSYNEX_GOLDEN_FINALIZER: PASS')
    print(f' TimeDateStamp=0x{TARGET_TIMESTAMP:08X}')
    print(f' LinkerVersion={TARGET_LINKER[0]}.{TARGET_LINKER[1]}')
    print(' RichHeader=' + ('STRIPPED' if stripped else 'ALREADY_ABSENT'))
    print(f' PECheckSum=0x{checksum:08X}')
    print(' RSDS/REPRO=ABSENT (precondition)')
    return 0
if __name__=='__main__': raise SystemExit(main())
