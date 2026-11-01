#!/usr/bin/env python3
"""GF13 exact cumulative source + immutable parent/HF19 + dormant native gate verifier."""
import argparse,hashlib,json,pathlib,re,sys,zipfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
PARENT='6be3fc0c6266684395047cf1a142032b94a993198b07314517306cbffa8243ab'
HF19='c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab'
CM_PARENT='553617001872987927d64b70f506cf5e15bec1c2241bedf2a3bb4a0184a907d6'
ENTRY='9133ea8a9d028f25399665acddd590bebbd57eaad418ac0b341dee2992f176fa'
EXPORT='cae880291e7ef717edef36a50a5cdd032686203ad9cfa21a75c555df5ab5ac43'
AVATAR='a2fa3aba3412096e658de21d9e8d62f470e1fd56f484cd40f3d69981723eef0a'
TEST='667b6f4272713d56fec11f8902468d54f6d7bd9144820f9717d29a703854c9e9'
EXTRA={
 'metadata/GF13_CMAKE_NATIVE_BLOCK.txt', 'metadata/GF13_STATUS.json',
 'docs/GF13_NATIVE_FINAL_LINK_GATE_TR.md', 'scripts/RUN_WINDOWS_GF13.cmd',
 'scripts/gf13_record_native.py','scripts/verify_gf13.py',
 'tests/qa/test_gf13_guard_negative.py','tests/qa/test_gf13_native_receipt_negative.py',
 'scripts/repack_gf13.py'
}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def fail(e):raise ValueError(e)
def verify(parent=None,hf19=None):
 manifest=json.loads((ROOT/'metadata/manifest.json').read_text())['files']
 got={p.relative_to(ROOT).as_posix() for p in ROOT.rglob('*') if p.is_file()}
 if any('__pycache__' in pathlib.PurePosixPath(n).parts or n.endswith('.pyc') for n in got):fail('CACHE_FILE_FORBIDDEN')
 if got!=set(manifest)|{'metadata/manifest.json'}:fail('FILE_SET_DRIFT')
 for n,v in manifest.items():
  if pathlib.PurePosixPath(n).is_absolute() or '..' in pathlib.PurePosixPath(n).parts:fail('PATH_TRAVERSAL')
  if sha(ROOT/n)!=v:fail('HASH_MISMATCH:'+n)
 s=json.loads((ROOT/'metadata/GF13_STATUS.json').read_text())
 if s['release']!='NOT_RELEASE' or s['classification']!='NON_RELEASE_WORK':fail('FORGED_RELEASE')
 for k in ['gf13_native_dispatch_final_link','windows_msvc_x64','windows_export_load','native_iocp','live_lsx','consumer_game']:
  if s[k]!='NOT_RUN':fail('FORGED_NATIVE_PASS:'+k)
 if s['gf13_scope']!='TEST_TOOLING_ONLY_NO_PRODUCTION_SOURCE_CHANGE':fail('PRODUCTION_SCOPE_DRIFT')
 if s['gf12_fix01_parent_sha256']!=PARENT or s['official_hf19_sha256']!=HF19:fail('PARENT_IDENTITY')
 if s['new_hf19_implementation_files']!=0 or s['cumulative_exact_hf19_files']!=232:fail('HF19_REUSE_COUNT')
 if s['shipping_surge']!='NOOP' or s['shipping_dllshutdown']!='ZERO_OWNER':fail('SHIPPING_STATUS')
 if s['get_config_malformed_entity']!='OPEN_EXIT14_NOT_COUNTED_PASS':fail('HIDDEN_FAILURE')
 if any(s[k] for k in ['publication_authorized','canonical_hf19_modified','current_artifact_modified']):fail('CANONICAL_OR_PUBLICATION')
 for n,h in [('src/win/dll_entry.cpp',ENTRY),('exports/preloader_l.def',EXPORT),('adopted_hf19/src/runtime/avatar_path.cpp',AVATAR),('adopted_hf19/tests/unit/test_dispatch.cpp',TEST)]:
  if sha(ROOT/n)!=h:fail('SHIPPING_OR_SOURCE_IDENTITY:'+n)
 cm=(ROOT/'CMakeLists.txt').read_bytes();block=(ROOT/'metadata/GF13_CMAKE_NATIVE_BLOCK.txt').read_bytes()
 if not cm.endswith(block) or sha_bytes(cm[:-len(block)])!=CM_PARENT:fail('CMAKE_PARENT_OR_BLOCK_DRIFT')
 if block.count(b'add_executable(gf13_original_dispatch_native')!=1:fail('NATIVE_LINK_TARGET_DRIFT')
 for expected in (b'if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)',b'adopted_hf19/tests/unit/test_dispatch.cpp',b'adopted_hf19/src/runtime/avatar_path.cpp',b'add_test(NAME gf13_original_dispatch_real_windows',b'gf11_exchange_units gf12_exchange_transitive_units'):
  if expected not in block:fail('NATIVE_GUARD_OR_SOURCE_MISSING')
 shiplinks=re.findall(rb'\btarget_link_libraries\s*\(\s*gf_preloader\b([^)]*)\)',cm,re.I|re.S)
 if len(shiplinks)!=1 or shiplinks[0].split()!=[b'PRIVATE',b'gf_core',b'gf_adopted']:fail('PREMATURE_SHIPPING_LINK')
 if b'WILL_FAIL' in cm:fail('FORGED_TEST_PASS')
 runner=(ROOT/'scripts/RUN_WINDOWS_GF13.cmd').read_text()
 for n in ['GF13_ENABLE_ORIGINAL_DISPATCH_NATIVE_LINK=ON','gf13_original_dispatch_native','gf_windows_export_shape','gf13_original_dispatch_real_windows','RUN_WINDOWS_GF13.cmd','GF13_TESTS_']:
  if n not in (runner+(ROOT/'scripts/gf13_record_native.py').read_text()):fail('RUNNER_GATE_MISSING:'+n)
 if parent:
  if sha(parent)!=PARENT:fail('PARENT_ZIP_HASH')
  with zipfile.ZipFile(parent) as z:
   if z.testzip() is not None:fail('PARENT_ZIP_CRC')
   inherited=set(z.namelist())
   for n in inherited-{'CMakeLists.txt','metadata/manifest.json'}:
    if not (ROOT/n).is_file() or (ROOT/n).read_bytes()!=z.read(n):fail('INHERITED_FILE_DRIFT:'+n)
   if got-inherited-{'metadata/manifest.json','CMakeLists.txt'} != EXTRA:fail('EXTRA_SCOPE_DRIFT')
 if hf19:
  if sha(hf19)!=HF19:fail('HF19_ZIP_HASH')
  with zipfile.ZipFile(hf19) as z:
   if z.testzip() is not None:fail('HF19_ZIP_CRC')
   for n in ('src/runtime/avatar_path.cpp','tests/unit/test_dispatch.cpp'):
    if (ROOT/'adopted_hf19'/n).read_bytes()!=z.read(n):fail('HF19_ORIGINAL_DRIFT:'+n)
 return len(manifest)
def sha_bytes(b):return hashlib.sha256(b).hexdigest()
if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('--parent',type=pathlib.Path);ap.add_argument('--hf19',type=pathlib.Path);a=ap.parse_args()
 try:print('GF13_VERIFY_PASS',verify(a.parent,a.hf19),'FILES','NON_RELEASE_WORK')
 except Exception as e:print('GF13_VERIFY_FAIL',str(e));sys.exit(1)
