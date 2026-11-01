#!/usr/bin/env python3
from __future__ import annotations
import struct,sys
from pathlib import Path

REQ={'DllMain','SURGE','DllShutdown','preloader_l_debug_wait_ready','preloader_l_debug_controlled_teardown','preloader_l_native_acceptance_quiesce','preloader_l_native_acceptance_worker_metric','preloader_l_native_acceptance_connect_metric','preloader_l_native_acceptance_f02_control','preloader_l_native_acceptance_wave4_control','preloader_l_native_acceptance_campaign18_scenario'}

def u16(d,o): return struct.unpack_from('<H',d,o)[0]
def u32(d,o): return struct.unpack_from('<I',d,o)[0]

def main():
    if len(sys.argv)!=2:
        print('usage: check_native_exports.py <dll>',file=sys.stderr); return 2
    p=Path(sys.argv[1]); d=p.read_bytes()
    if d[:2]!=b'MZ': print('NATIVE_ACCEPTANCE_EXPORTS: FAIL: no MZ',file=sys.stderr); return 1
    pe=u32(d,0x3c)
    if d[pe:pe+4]!=b'PE\0\0': print('NATIVE_ACCEPTANCE_EXPORTS: FAIL: no PE',file=sys.stderr); return 1
    nsec=u16(d,pe+6); optsz=u16(d,pe+20); opt=pe+24
    if u16(d,opt)!=0x20b: print('NATIVE_ACCEPTANCE_EXPORTS: FAIL: not PE32+',file=sys.stderr); return 1
    dd=opt+112
    erva=u32(d,dd); esz=u32(d,dd+4)
    st=opt+optsz
    secs=[]
    for i in range(nsec):
        o=st+i*40; vs=u32(d,o+8); va=u32(d,o+12); rs=u32(d,o+16); rp=u32(d,o+20)
        secs.append((va,max(vs,rs),rp))
    def off(rva):
        for va,span,rp in secs:
            if va<=rva<va+span: return rp+(rva-va)
        raise ValueError(f'RVA 0x{rva:X} outside sections')
    if not erva or not esz: print('NATIVE_ACCEPTANCE_EXPORTS: FAIL: no export dir',file=sys.stderr); return 1
    eo=off(erva); n=u32(d,eo+24); names_rva=u32(d,eo+32); no=off(names_rva)
    names=set()
    for i in range(n):
        nrva=u32(d,no+i*4); pos=off(nrva); end=d.find(b'\0',pos)
        if end<0: raise ValueError('unterminated export')
        names.add(d[pos:end].decode('ascii','strict'))
    miss=sorted(REQ-names)
    if miss:
        print('NATIVE_ACCEPTANCE_EXPORTS: FAIL: missing '+', '.join(miss),file=sys.stderr); return 1
    print('NATIVE_ACCEPTANCE_EXPORTS: PASS')
    print('exports: '+ ' '.join(sorted(names)))
    return 0
if __name__=='__main__': raise SystemExit(main())
