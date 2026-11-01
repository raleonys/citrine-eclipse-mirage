#!/usr/bin/env python3
"""Construct a structurally bounded synthetic PE fixture; never call it Windows native evidence."""
import pathlib,struct,sys
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[2]/'scripts'))
from gf10_pe_export_gate import check,PEError
s16=lambda b,o,v:struct.pack_into('<H',b,o,v)
s32=lambda b,o,v:struct.pack_into('<I',b,o,v)
b=bytearray(0x1600)
b[0:2]=b'MZ';s32(b,0x3c,0x80);b[0x80:0x84]=b'PE\0\0'
s16(b,0x84,0x8664);s16(b,0x86,2);s16(b,0x94,0xf0);s16(b,0x98,0x20b)
s32(b,0x98+60,0x200);s32(b,0x98+108,16);s32(b,0x98+112,0x1100);s32(b,0x98+116,0x120)
# rdata RVA 1000 -> raw 200, text RVA 2000 -> raw 1200
for o,va,ptr,sz in [(0x188,0x1000,0x200,0x1000),(0x1b0,0x2000,0x1200,0x200)]:
 s32(b,o+8,sz);s32(b,o+12,va);s32(b,o+16,sz);s32(b,o+20,ptr)
struct.pack_into('<IIHHIIIIIII',b,0x300,0,0,0,0,0x11b0,1,3,3,0x1180,0x1190,0x11a0)
for i,rv in enumerate((0x2000,0x2010,0x2020)):s32(b,0x380+i*4,rv)
for i,rv in enumerate((0x11c0,0x11d0,0x11e0)):s32(b,0x390+i*4,rv)
for i in range(3):s16(b,0x3a0+i*2,i)
b[0x3b0:0x3b5]=b'x.dll';b[0x3b5]=0
for off,name in [(0x3c0,b'DllMain'),(0x3d0,b'SURGE'),(0x3e0,b'DllShutdown')]:b[off:off+len(name)]=name;b[off+len(name)]=0
# the .text raw bytes
b[0x1200]=0xc3;b[0x1210]=0xc3;b[0x1220]=0xc3
assert [v['name'] for v in check(bytes(b))['dll_exports']]==['DllMain','SURGE','DllShutdown']
def neg(label,fn):
 x=bytearray(b);fn(x)
 try:check(bytes(x))
 except PEError:pass
 else:raise AssertionError('NOT_REJECTED_'+label)
neg('NO_MZ',lambda x:x.__setitem__(slice(0,2),b'ZZ'))
neg('MACHINE',lambda x:s16(x,0x84,0x14c))
neg('PE32',lambda x:s16(x,0x98,0x10b))
neg('NO_EXPORT',lambda x:s32(x,0x98+112,0))
neg('ORDINAL_ONLY',lambda x:s32(x,0x300+24,2))
neg('WRONG_BASE',lambda x:s32(x,0x300+16,2))
neg('BAD_FUNCTION_COUNT',lambda x:s32(x,0x300+20,4))
neg('DUPLICATE_ORD',lambda x:s16(x,0x3a2,0))
neg('UNKNOWN_NAME',lambda x:x.__setitem__(0x3d0,ord('Z')))
neg('NULL_FUNCTION',lambda x:s32(x,0x380,0))
neg('FORWARDED',lambda x:s32(x,0x380,0x1150))
neg('VIRTUAL_TAIL',lambda x:s32(x,0x380,0x3000))
neg('OVERLAY',lambda x:s32(x,0x380,0x3001))
neg('INVALID_SECTION_RAW',lambda x:s32(x,0x188+20,0x1500))
neg('NAME_PTR',lambda x:s32(x,0x390,0x90000000))
neg('DUPLICATE_SECTION',lambda x:s32(x,0x1b0+12,0x1000))
neg('TRUNCATED_PE',lambda x:x.__delitem__(slice(0x400,None)))
print('GF10_SYNTHETIC_PE_GATE_17_NEGATIVES_PLUS_VALID_PASS')
