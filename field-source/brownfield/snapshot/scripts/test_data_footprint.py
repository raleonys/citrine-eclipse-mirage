#!/usr/bin/env python3
"""Synthetic positive/negative PE fixtures for the profile-aware Golden budget."""
from pathlib import Path
import importlib.util
import io
import struct
import tempfile
from contextlib import redirect_stdout

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('footprint',ROOT/'scripts/check_data_footprint.py')
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)


def fixture(md=False):
    names=['.text','.rdata','.data','.pdata','.rsrc','.reloc'] if md else ['.text','.rdata','.data','.pdata','.fptable','.rsrc','.reloc']
    raws={'.text':0x200,'.rdata':0x400,'.data':0x800 if md else 0x1200,'.pdata':0x200,'.fptable':0x200,'.rsrc':0x200,'.reloc':0x200}
    vaddr={'.text':0x1000,'.rdata':0x2000,'.data':0x3000,'.pdata':0x5000,'.fptable':0x6000,'.rsrc':0x7000,'.reloc':0x8000}
    size=0x400+sum(raws[n] for n in names)
    b=bytearray(size);b[0:2]=b'MZ';struct.pack_into('<I',b,0x3c,0x80);b[0x80:0x84]=b'PE\0\0'
    struct.pack_into('<HHIIIHH',b,0x84,0x8664,len(names),0,0,0,0xf0,0x2022)
    opt=0x98;struct.pack_into('<H',b,opt,0x20b);struct.pack_into('<I',b,opt+108,16)
    struct.pack_into('<II',b,opt+112+8,0x2020,0x28)
    raw_ptr=0x400
    for i,n in enumerate(names):
        o=opt+0xf0+i*40;b[o:o+8]=n.encode().ljust(8,b'\0')
        raw=raws[n];vs=0x100 if n=='.fptable' else raw
        flags=0xC0000040 if n=='.fptable' else 0x40000040
        struct.pack_into('<IIII',b,o+8,vs,vaddr[n],raw,raw_ptr)
        struct.pack_into('<I',b,o+36,flags)
        if n=='.rdata': rdata_ptr=raw_ptr
        raw_ptr+=raw
    dll=b'VCRUNTIME140.DLL\0' if md else b'KERNEL32.DLL\0'
    struct.pack_into('<I',b,rdata_ptr+0x20+12,0x2100)
    b[rdata_ptr+0x100:rdata_ptr+0x100+len(dll)]=dll
    return b,names,raws


def check(data,expect,md=False):
    with tempfile.TemporaryDirectory() as tmp:
        p=Path(tmp)/'fixture.dll';p.write_bytes(data)
        try:
            with redirect_stdout(io.StringIO()):mod.check(p,md)
            ok=True
        except (SystemExit,ValueError,IndexError,KeyError,struct.error):ok=False
        if ok!=expect:raise RuntimeError('unexpected result: '+('accepted' if ok else 'rejected'))

b,names,raws=fixture();check(b,True)
# Golden must reject profile confusion, payload inflation and .fptable tampering.
for case in range(8):
    z=bytearray(b);base=0x98+0xf0;data_o=base+2*40;fp_o=base+4*40
    if case==0:struct.pack_into('<I',z,data_o+16,0x1a00) # raw budget exceeded
    if case==1:struct.pack_into('<I',z,data_o+8,0x400000) # persistent pool regression
    if case==2:z[fp_o:fp_o+8]=b'.bad\0\0\0\0' # section identity drift
    if case==3:struct.pack_into('<I',z,fp_o+36,0xe0000040) # executable fptable
    if case==4:struct.pack_into('<I',z,fp_o+8,0x200) # wrong virtual size
    if case==5:z[0x400+0x200+0x400+0x1200+0x200]=1 # nonzero fptable
    if case==6:struct.pack_into('<H',z,0x84,0x14c) # x86 model mismatch
    if case==7:struct.pack_into('<I',z,data_o+20,len(z)+0x400) # unbacked section
    check(z,False)
legacy,*_=fixture(md=True)
check(legacy,True,md=True);check(legacy,False,md=False)
check(b,False,md=True)
# Dynamic CRT must never masquerade as shipping static CRT.
z=bytearray(b);off=0x400+0x200+0x100
z[off:off+16]=b'VCRUNTIME140.DLL';check(z,False)
print('DATA_FOOTPRINT_PROFILE_REGRESSION: PASS (13/13, current /MT and historical /MD comparison, adversarial PE)')
