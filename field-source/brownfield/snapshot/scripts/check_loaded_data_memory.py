#!/usr/bin/env python3
"""Read-only audit of the loaded preloader_l.dll .data mapping in a live Windows process."""
from __future__ import annotations
import argparse, ctypes, json, os, struct, sys
from ctypes import wintypes
from pathlib import Path

TH32CS_SNAPMODULE=0x8
TH32CS_SNAPMODULE32=0x10
PROCESS_QUERY_INFORMATION=0x0400
PROCESS_VM_READ=0x0010
INVALID_HANDLE_VALUE=ctypes.c_void_p(-1).value
MEM_COMMIT=0x1000
MEM_IMAGE=0x1000000

class MODULEENTRY32W(ctypes.Structure):
    _fields_=[
        ('dwSize',wintypes.DWORD),('th32ModuleID',wintypes.DWORD),('th32ProcessID',wintypes.DWORD),
        ('GlblcntUsage',wintypes.DWORD),('ProccntUsage',wintypes.DWORD),('modBaseAddr',ctypes.POINTER(ctypes.c_ubyte)),
        ('modBaseSize',wintypes.DWORD),('hModule',wintypes.HMODULE),('szModule',wintypes.WCHAR*256),('szExePath',wintypes.WCHAR*260),
    ]

class MEMORY_BASIC_INFORMATION(ctypes.Structure):
    _fields_=[
        ('BaseAddress',ctypes.c_void_p),('AllocationBase',ctypes.c_void_p),
        ('AllocationProtect',wintypes.DWORD),('PartitionId',wintypes.WORD),
        ('RegionSize',ctypes.c_size_t),('State',wintypes.DWORD),('Protect',wintypes.DWORD),('Type',wintypes.DWORD),
    ]

def die(msg):
    print('LOADED_DATA_MEMORY: FAIL:',msg,file=sys.stderr); raise SystemExit(2)

def locate_module(k32,pid,name):
    snap=k32.CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid)
    if snap==INVALID_HANDLE_VALUE: die(f'CreateToolhelp32Snapshot winerr={ctypes.get_last_error()}')
    try:
        e=MODULEENTRY32W(); e.dwSize=ctypes.sizeof(e)
        if not k32.Module32FirstW(snap,ctypes.byref(e)): die(f'Module32FirstW winerr={ctypes.get_last_error()}')
        wanted=name.lower()
        while True:
            if e.szModule.lower()==wanted or Path(e.szExePath).name.lower()==wanted:
                return ctypes.cast(e.modBaseAddr,ctypes.c_void_p).value,int(e.modBaseSize),e.szExePath
            if not k32.Module32NextW(snap,ctypes.byref(e)): break
        die(f'module {name!r} not found in pid {pid}')
    finally:
        k32.CloseHandle(snap)

def read(k32,process,address,size):
    buf=(ctypes.c_ubyte*size)(); got=ctypes.c_size_t()
    if not k32.ReadProcessMemory(process,ctypes.c_void_p(address),buf,size,ctypes.byref(got)) or got.value!=size:
        die(f'ReadProcessMemory 0x{address:x}/0x{size:x} got=0x{got.value:x} winerr={ctypes.get_last_error()}')
    return bytes(buf)

def data_section(headers):
    if headers[:2]!=b'MZ': die('missing MZ')
    pe=struct.unpack_from('<I',headers,0x3c)[0]
    if headers[pe:pe+4]!=b'PE\0\0': die('bad PE signature')
    coff=pe+4; nsec=struct.unpack_from('<H',headers,coff+2)[0]; opt_size=struct.unpack_from('<H',headers,coff+16)[0]
    sec=coff+20+opt_size
    for i in range(nsec):
        o=sec+i*40; name=headers[o:o+8].split(b'\0',1)[0].decode('ascii','replace')
        if name=='.data':
            vs,va,raw=struct.unpack_from('<III',headers,o+8)
            return va,vs,raw
    die('.data section not found')

def main():
    if os.name!='nt': die('run this tool on Windows')
    ap=argparse.ArgumentParser(); ap.add_argument('--pid',required=True,type=int); ap.add_argument('--module',default='preloader_l.dll'); ap.add_argument('--out',type=Path,required=True)
    a=ap.parse_args()
    k32=ctypes.WinDLL('kernel32',use_last_error=True)
    k32.CreateToolhelp32Snapshot.restype=wintypes.HANDLE; k32.OpenProcess.restype=wintypes.HANDLE
    k32.Module32FirstW.restype=wintypes.BOOL; k32.Module32NextW.restype=wintypes.BOOL; k32.ReadProcessMemory.restype=wintypes.BOOL
    k32.VirtualQueryEx.restype=ctypes.c_size_t
    base,module_size,path=locate_module(k32,a.pid,a.module)
    proc=k32.OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,False,a.pid)
    if not proc: die(f'OpenProcess winerr={ctypes.get_last_error()}')
    try:
        hdr=read(k32,proc,base,min(max(module_size,0x10000),0x10000))
        rva,vs,raw=data_section(hdr)
        begin=base+rva; end=begin+vs; cur=begin; regions=[]; all_commit=True; all_image=True
        while cur<end:
            mbi=MEMORY_BASIC_INFORMATION()
            n=k32.VirtualQueryEx(proc,ctypes.c_void_p(cur),ctypes.byref(mbi),ctypes.sizeof(mbi))
            if not n: die(f'VirtualQueryEx 0x{cur:x} winerr={ctypes.get_last_error()}')
            rb=mbi.BaseAddress or 0; re=rb+mbi.RegionSize
            clip_b=max(cur,rb); clip_e=min(end,re)
            regions.append({'base':f'0x{rb:x}','region_size':f'0x{mbi.RegionSize:x}','covered_begin':f'0x{clip_b:x}','covered_end':f'0x{clip_e:x}','state':f'0x{mbi.State:x}','protect':f'0x{mbi.Protect:x}','type':f'0x{mbi.Type:x}'})
            all_commit &= mbi.State==MEM_COMMIT
            all_image &= mbi.Type==MEM_IMAGE
            if re<=cur: die('VirtualQueryEx returned non-advancing region')
            cur=re
    finally:
        k32.CloseHandle(proc)
    result={'schema':1,'pid':a.pid,'module':a.module,'module_path':path,'module_base':f'0x{base:x}','module_size':f'0x{module_size:x}',
            'data_rva':f'0x{rva:x}','data_virtual_size':f'0x{vs:x}','data_raw_size':f'0x{raw:x}','data_begin':f'0x{begin:x}','data_end':f'0x{end:x}',
            'all_regions_mem_commit':all_commit,'all_regions_mem_image':all_image,'regions':regions,'semantics_modified':False}
    a.out.parent.mkdir(parents=True,exist_ok=True); a.out.write_text(json.dumps(result,indent=2)+'\n')
    print('LOADED_DATA_MEMORY: REPORT')
    print(f'.data RVA=0x{rva:X} VirtualSize=0x{vs:X} RawSize=0x{raw:X}')
    for x in regions: print(' -',x['covered_begin'],x['covered_end'],'state='+x['state'],'protect='+x['protect'],'type='+x['type'])
    if not all_commit or not all_image:
        print('LOADED_DATA_MEMORY: FAIL - .data is not entirely MEM_COMMIT/MEM_IMAGE under the live loader mapping')
        return 1
    print('LOADED_DATA_MEMORY: PASS - .data is mapped as committed image memory; physical residency remains demand-paged by Windows')
    return 0

if __name__=='__main__': raise SystemExit(main())
