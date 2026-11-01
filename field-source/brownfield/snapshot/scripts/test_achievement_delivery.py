#!/usr/bin/env python3
"""HF10 B: shipping-response transaction against exact current production units.

The fixture uses Windows heap stubs; only three implementation units are
compiled under the Windows conditional. No native/game acceptance implied.
"""
from pathlib import Path
import shutil,subprocess,sys,tempfile
r=Path(__file__).resolve().parents[1]
units=(r/'tests/production_source_units.txt').read_text(encoding='utf-8').splitlines()
if not units or len(units)!=68 or len(set(units))!=len(units):
 print('HF10_B: FAIL source closure inventory');sys.exit(1)
special=('src/runtime/event_queue.cpp','src/protocol/socket_exchange_transaction.cpp','src/protocol/socket_exchange_transaction_delivery.cpp','src/handlers/achievement_grant_route.cpp')
if 'src/handlers/achievement_grant_route.cpp' not in units or any(not (r/x).is_file() for x in [*units,*special]):
 print('HF10_B: FAIL missing production source');sys.exit(1)
flags=['-std=c++20','-O0','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-unused-function','-Wno-unused-variable','-Wno-unused-const-variable','-pthread']
inc=['-I'+str(r/'include'),'-I'+str(r/'src/protocol'),'-I'+str(r/'tests/support'),'-I'+str(r/'src/debug'),'-I'+str(r/'third_party/pegtl/upstream/include')]
with tempfile.TemporaryDirectory(prefix='squaleris_hf10_b_') as td:
 d=Path(td)
 for compiler in ('g++','clang++'):
  if not shutil.which(compiler):
   print('HF10_B: FAIL compiler missing',compiler);sys.exit(2)
  output=[]
  for x in special:
   o=d/(compiler.replace('+','p')+'_'+Path(x).stem+'.o')
   cmd=[compiler,*flags,*inc,'-D_WIN32','-D__declspec(x)=','-D__stdcall=','-c',str(r/x),'-o',str(o)]
   p=subprocess.run(cmd,capture_output=True,text=True,timeout=90)
   if p.returncode:print('HF10_B: FAIL special compiler',x,compiler,p.stderr[-5000:]);sys.exit(1)
   output.append(str(o))
  exe=d/('hf10_b_'+compiler.replace('+','p'))
  cmd=[compiler,*flags,*inc,str(r/'tests/unit/test_achievement_delivery.cpp'),
       *[str(r/x) for x in units if x not in special],*output,'-o',str(exe)]
  p=subprocess.run(cmd,capture_output=True,text=True,timeout=180)
  if p.returncode:print('HF10_B: FAIL source link',compiler,p.stderr[-8000:]);sys.exit(1)
  p=subprocess.run([str(exe)],capture_output=True,text=True,timeout=60)
  if p.returncode or 'HF10_B_REAL_SOURCE_ASSERTIONS_PASS 218 ' not in p.stdout:
   print('HF10_B: FAIL source runtime',compiler,p.stdout,p.stderr[-5000:]);sys.exit(1)
  print('HF10_B:',compiler,'PASS (218/218; source-linked; native/game NOT_RUN)')
print('HF10_B: PASS GCC + CLANG')
