#!/usr/bin/env python3
"""Exact inherited GF13 custody and new GF14 source manifest, no trust in status claims."""
import hashlib,json,pathlib,zipfile,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
GF13_SHA='e67305952326ef7ecd4db77cf503f9f38cc0cd381d92362a0dadcfff8380e17d'
HF19_SHA='c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab'
EXTENSIONS={
 'docs/NATIVE_EVIDENCE_DESIGN_TR.md','metadata/WORK_STATE.json',
 'scripts/build/BUILD_NATIVE_ACCEPTANCE.cmd','scripts/qa/check_source_provenance.py',
 'scripts/build/build_native_acceptance.py','scripts/qa/test_native_evidence.py',
 'scripts/build/build_source_package.py','tests/qa/test_native_evidence_negative.py',
 'tests/qa/test_source_provenance.py','tests/qa/test_source_tamper.py'
}

def sha(b):return hashlib.sha256(b).hexdigest()
def verify(root=ROOT, gf13=None, hf19=None):
 root=pathlib.Path(root)
 mpath=root/'metadata/manifest.json'
 m=json.loads(mpath.read_text('utf-8'))['files']
 present={x.relative_to(root).as_posix() for x in root.rglob('*') if x.is_file()}
 if any('__pycache__' in pathlib.PurePosixPath(n).parts or n.endswith('.pyc') for n in present):raise ValueError('TRANSIENT_CACHE_FORBIDDEN')
 if present!=set(m)|{'metadata/manifest.json'}:raise ValueError('MANIFEST_FILE_SET_DRIFT')
 for n,v in m.items():
  npath=pathlib.PurePosixPath(n)
  if npath.is_absolute() or '..' in npath.parts or npath.as_posix()!=n:raise ValueError('UNSAFE_MANIFEST_PATH')
  if sha((root/n).read_bytes())!=v:raise ValueError('MANIFEST_HASH_MISMATCH:'+n)
 state=json.loads((root/'metadata/WORK_STATE.json').read_text('utf-8'))
 expected={
  'classification':'NON_RELEASE_WORK','release':'NOT_RELEASE',
  'gf13_parent_sha256':GF13_SHA,'canonical_hf19_source_sha256':HF19_SHA,
  'scope':'TEST_ONLY_NATIVE_RUNNER_AND_EVIDENCE_CHECK_NO_PRODUCTION_SOURCE_CHANGE',
  'shipping_surge':'NOOP','shipping_dllshutdown':'ZERO_OWNER',
  'windows_msvc_x64':'NOT_RUN','windows_dispatch_final_link':'NOT_RUN',
  'windows_export_load':'NOT_RUN','windows_io_socket':'NOT_RUN',
  'live_lsx':'NOT_RUN','consumer_game':'NOT_RUN',
  'gf09_get_config':'OPEN_EXIT14','production_dispatch_shipping_link':'NOT_IMPLEMENTED'
 }
 for k,v in expected.items():
  if state.get(k)!=v:raise ValueError('GF14_STATUS_DRIFT:'+k)
 for k in ('source_changed','canonical_modified','current_artifact_modified'):
  if state.get(k) is not False:raise ValueError('GF14_UNAUTHORIZED_MUTATION:'+k)
 # Exact inherited GF13 source members anchored independently by its package SHA.
 if gf13:
  gf13=pathlib.Path(gf13)
  if sha(gf13.read_bytes())!=GF13_SHA:raise ValueError('GF13_PARENT_HASH_MISMATCH')
  with zipfile.ZipFile(gf13) as z:
   if z.testzip():raise ValueError('GF13_PARENT_CRC')
   prev={i.filename for i in z.infolist()}
   for n in prev-{'metadata/manifest.json'}:
    if (root/n).read_bytes()!=z.read(n):raise ValueError('GF13_PARENT_BYTE_DRIFT:'+n)
   if present-prev-{'metadata/manifest.json'} != EXTENSIONS:raise ValueError('GF14_ADDED_SCOPE_DRIFT')
 # Unconditional QA protection even when parent ZIP unavailable.
 cm=(root/'CMakeLists.txt').read_text('utf-8')
 import re
 links=re.findall(r'\btarget_link_libraries\s*\(\s*gf_preloader\b([^)]*)\)',cm,re.I|re.S)
 if len(links)!=1 or links[0].split()!=['PRIVATE','gf_core','gf_adopted']:
  raise ValueError('GF14_SHIPPING_LINK_DRIFT')
 if 'GF13_ENABLE_ORIGINAL_DISPATCH_NATIVE_LINK' not in cm:
  raise ValueError('GF13_NATIVE_DISPATCH_GATE_MISSING')
 if hf19:
  hf19=pathlib.Path(hf19)
  if sha(hf19.read_bytes())!=HF19_SHA:raise ValueError('OFFICIAL_HF19_HASH_MISMATCH')
  with zipfile.ZipFile(hf19) as z:
   if z.testzip():raise ValueError('HF19_PARENT_CRC')
   for n in ('src/runtime/avatar_path.cpp','tests/unit/test_dispatch.cpp'):
    if z.read(n)!=(root/'adopted_hf19'/n).read_bytes():raise ValueError('HF19_SOURCE_DRIFT:'+n)
 return {'source_member_count':len(m),'source_manifest_sha256':sha(mpath.read_bytes()),
         'parent_gf13_sha256':GF13_SHA,'official_hf19_sha256':HF19_SHA}

if __name__=='__main__':
 import argparse
 ap=argparse.ArgumentParser();ap.add_argument('--parent',type=pathlib.Path);ap.add_argument('--hf19',type=pathlib.Path);a=ap.parse_args()
 try:print('GF14_SOURCE_VERIFY_PASS',json.dumps(verify(gf13=a.parent,hf19=a.hf19),sort_keys=True))
 except Exception as e:print('GF14_SOURCE_VERIFY_FAIL',str(e));sys.exit(2)
