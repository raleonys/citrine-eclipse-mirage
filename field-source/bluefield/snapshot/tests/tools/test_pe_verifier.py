# SPDX-License-Identifier: GPL-3.0-only
"""Synthetic static-PE checker regressions; these bytes are never executed."""
import struct,sys,tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'scripts/qa'))
from check_pe import inspect
def fixture():
 b=bytearray(0x600);b[:2]=b'MZ';struct.pack_into('<I',b,0x3c,0x80);b[0x80:0x84]=b'PE\0\0'
 struct.pack_into('<HHIIIHH',b,0x84,0x8664,1,0,0,0,240,0x2002);opt=0x98
 struct.pack_into('<H',b,opt,0x20b);struct.pack_into('<I',b,opt+16,0x1000);struct.pack_into('<H',b,opt+70,0x4160);struct.pack_into('<II',b,opt+112,0x1100,0x100)
 section=opt+240;b[section:section+5]=b'.text';struct.pack_into('<IIII',b,section+8,0x400,0x1000,0x400,0x200);struct.pack_into('<I',b,section+36,0x60000020)
 struct.pack_into('<IIHHIIIIIII',b,0x300,0,0,0,0,0x11d0,1,3,3,0x1140,0x1150,0x1160)
 struct.pack_into('<III',b,0x340,0x1010,0x1020,0x1030);struct.pack_into('<III',b,0x350,0x1180,0x1190,0x11a0);struct.pack_into('<HHH',b,0x360,0,1,2)
 for offset,s in [(0x380,b'DllMain\0'),(0x390,b'SURGE\0'),(0x3a0,b'DllShutdown\0'),(0x3d0,b'fixture.dll\0')]:b[offset:offset+len(s)]=s
 return b
def main():
 with tempfile.TemporaryDirectory(prefix='preloader_l_pe_') as temp:
  p=Path(temp)/'fixture.dll';valid=fixture();p.write_bytes(valid);assert inspect(p)['status']=='PASS'
  mutations=[]
  b=bytearray(valid);b[0]=0;mutations.append(b)
  b=bytearray(valid);struct.pack_into('<H',b,0x84,0x14c);mutations.append(b)
  b=bytearray(valid);struct.pack_into('<I',b,0x310,2);mutations.append(b)
  b=bytearray(valid);struct.pack_into('<H',b,0x98+70,0x4060);mutations.append(b)
  b=bytearray(valid);struct.pack_into('<II',b,0x98+120,0x1200,40);struct.pack_into('<IIIII',b,0x400,1,0,0,0x1250,1);b[0x450:0x450+13]=b'ucrtbase.dll\0';mutations.append(b)
  b=bytearray(valid);struct.pack_into('<I',b,0x98+240+36,0x40000040);mutations.append(b)
  mutations.append(valid[:100])
  b=bytearray(valid);struct.pack_into('<I',b,0x340,0x1101);mutations.append(b)
  for i,b in enumerate(mutations):
   p.write_bytes(b)
   try:inspect(p)
   except Exception:continue
   raise AssertionError('negative fixture accepted: '+str(i))
 print('PASS PE verifier: baseline + 8 negative fixtures; none executed');return 0
if __name__=='__main__':sys.exit(main())
