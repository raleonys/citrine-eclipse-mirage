#!/usr/bin/env python3
"""Fail-closed mutation suite. Reverts all edits in finally; never changes canonical HF19."""
import pathlib,sys,json,hashlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from verify_gf12 import verify
cases=[
 ('raw_hf19_source_modified','adopted_hf19/src/protocol/challenge_handshake.cpp',b'\n// tamper\n',False,'HASH_MISMATCH:'),
 ('rehashed_hf19_source_tamper','adopted_hf19/src/protocol/socket_exchange_wire.cpp',b'\n// tamper\n',True,'ADOPTED_SOURCE_MUTATED:'),
 ('rehashed_premature_shipping_link','CMakeLists.txt',b'\ntarget_link_libraries(gf_preloader PRIVATE gf12_exchange_transitive_units)\n',True,'PREMATURE_SHIPPING_LINK'),
 ('rehashed_forged_native_pass','metadata/GF12_STATUS.json',b'',True,'FORGED_NATIVE_OR_FINAL_LINK_PASS:'),
 ('rehashed_export_drift','exports/preloader_l.def',b'',True,'EXPORT_DRIFT'),
 ('unmanifested_file','bogus_GF12_unowned.txt',b'hello',False,'FILE_SET_DRIFT'),
]
sha=lambda b:hashlib.sha256(b).hexdigest()
manifest=ROOT/'metadata/manifest.json'
passed=0
for label,name,suffix,rehashed,expected in cases:
 p=ROOT/name
 original=p.read_bytes() if p.exists() else None
 mf_original=manifest.read_bytes()
 try:
  if label=='rehashed_forged_native_pass':
   status=json.loads(original);status['msvc_x64']='PASS';p.write_text(json.dumps(status,indent=2,sort_keys=True)+'\n')
  elif label=='rehashed_export_drift':p.write_bytes(original.replace(b'@2',b'@7'))
  else:p.write_bytes((original or b'')+suffix)
  if rehashed:
   m=json.loads(mf_original);m['files'][name]=sha(p.read_bytes());manifest.write_text(json.dumps(m,sort_keys=True,indent=2)+'\n')
  try:verify()
  except Exception as e:
   if not str(e).startswith(expected):raise AssertionError(label+' WRONG_REJECTION '+str(e)) from e
   passed+=1
  else:raise AssertionError(label+' FALSE_ACCEPT')
 finally:
  if original is None:p.unlink(missing_ok=True)
  else:p.write_bytes(original)
  manifest.write_bytes(mf_original)
print(f'GF12_NEGATIVE_PASS {passed}/{len(cases)} CORRECT_REJECTION')
