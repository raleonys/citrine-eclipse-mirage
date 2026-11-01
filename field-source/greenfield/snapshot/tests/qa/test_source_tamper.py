#!/usr/bin/env python3
"""Negative tests refuse forged source, native status and shipping link even if rehashed."""
import hashlib,json,pathlib,shutil,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts/qa'))
from check_source_provenance import verify

def seal(folder):
 m=folder/'metadata/manifest.json'
 data=json.loads(m.read_text('utf-8'))
 for n in data['files']:
  data['files'][n]=hashlib.sha256((folder/n).read_bytes()).hexdigest()
 m.write_text(json.dumps(data,indent=2,sort_keys=True)+'\n')

def check_mutation(what):
 with tempfile.TemporaryDirectory(prefix='gf14_tamper_') as tmp:
  d=pathlib.Path(tmp)/'source'
  shutil.copytree(ROOT,d,ignore=shutil.ignore_patterns('__pycache__','*.pyc'))
  if what=='source_bytes':
   p=d/'adopted_hf19/src/protocol/socket_exchange_wire.cpp'
   p.write_bytes(p.read_bytes()+b'\n// intentional negative mutation\n')
  elif what=='forged_native':
   p=d/'metadata/WORK_STATE.json';data=json.loads(p.read_text());data['windows_msvc_x64']='PASS';p.write_text(json.dumps(data)+'\n');seal(d)
  elif what=='forged_canonical':
   p=d/'metadata/WORK_STATE.json';data=json.loads(p.read_text());data['canonical_modified']=True;p.write_text(json.dumps(data)+'\n');seal(d)
  elif what=='second_shipping_link':
   p=d/'CMakeLists.txt';p.write_text(p.read_text()+'\ntarget_link_libraries(gf_preloader PRIVATE gf_hf19_dispatch_units)\n');seal(d)
  elif what=='unmanifested':
   (d/'src/hidden_source.cpp').write_text('int hidden() { return 7; }\n')
  else:raise ValueError(what)
  try:verify(d)
  except ValueError as e:
   msg=str(e)
   wanted={'source_bytes':'MANIFEST_HASH_MISMATCH', 'forged_native':'GF14_STATUS_DRIFT',
           'forged_canonical':'GF14_UNAUTHORIZED_MUTATION',
           'second_shipping_link':'GF14_SHIPPING_LINK_DRIFT','unmanifested':'MANIFEST_FILE_SET_DRIFT'}[what]
   if wanted not in msg:raise AssertionError(what+' WRONG_REJECTION '+msg)
   return
  raise AssertionError(what+' WAS_ACCEPTED')
for x in ('source_bytes','forged_native','forged_canonical','second_shipping_link','unmanifested'):
 check_mutation(x)
print('GF14_SOURCE_TAMPER_NEGATIVES_PASS 5/5')
