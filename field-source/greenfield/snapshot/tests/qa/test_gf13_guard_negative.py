#!/usr/bin/env python3
"""GF13 guard tamper negatives with correct-reason assertions and byte restoration."""
import pathlib,sys,json,hashlib
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts'))
from verify_gf13 import verify
M=ROOT/'metadata/manifest.json'
scenarios=[
 ('fake_native_success','metadata/GF13_STATUS.json','FORGED_NATIVE_PASS:','windows_msvc_x64'),
 ('fake_link_success','metadata/GF13_STATUS.json','FORGED_NATIVE_PASS:','gf13_native_dispatch_final_link'),
 ('early_shipping_link','CMakeLists.txt','CMAKE_PARENT_OR_BLOCK_DRIFT',None),
 ('mutated_windows_avatar','adopted_hf19/src/runtime/avatar_path.cpp','SHIPPING_OR_SOURCE_IDENTITY:',None),
 ('altered_export','exports/preloader_l.def','SHIPPING_OR_SOURCE_IDENTITY:',None),
 ('disabled_native_guard','metadata/GF13_CMAKE_NATIVE_BLOCK.txt','CMAKE_PARENT_OR_BLOCK_DRIFT',None),
 ('new_unmanifested_file','BOGUS_GF13.txt','FILE_SET_DRIFT',None),
 ('fake_release','metadata/GF13_STATUS.json','FORGED_RELEASE','release'),
]
def sha(b):return hashlib.sha256(b).hexdigest()
def mutate(i,body,arg):
 if i=='fake_native_success' or i=='fake_link_success':
  d=json.loads(body);d[arg]='PASS';return (json.dumps(d,indent=2,sort_keys=True)+'\n').encode()
 if i=='fake_release':
  d=json.loads(body);d[arg]='FINAL_RELEASE';return (json.dumps(d,indent=2,sort_keys=True)+'\n').encode()
 if i=='altered_export':return body.replace(b'@2',b'@9')
 if i=='disabled_native_guard':return body.replace(b'NOT WIN32 OR NOT MSVC',b'FALSE AND FALSE')
 return body+b'\n# deliberate GF13 tamper\n'
passed=0
for label,n,expected,arg in scenarios:
 p=ROOT/n;original=p.read_bytes() if p.exists() else None;old_manifest=M.read_bytes()
 try:
  p.write_bytes(mutate(label,original or b'',arg))
  if original is not None:
   d=json.loads(old_manifest);d['files'][n]=sha(p.read_bytes());M.write_text(json.dumps(d,indent=2,sort_keys=True)+'\n')
  try:verify()
  except Exception as e:
   if not str(e).startswith(expected):raise RuntimeError('WRONG_REJECTION:'+label+':'+str(e))
   passed+=1
  else:raise RuntimeError('FALSE_ACCEPT:'+label)
 finally:
  if original is None:p.unlink(missing_ok=True)
  else:p.write_bytes(original)
  M.write_bytes(old_manifest)
print('GF13_GUARD_NEGATIVES_PASS',passed,'/',len(scenarios))
