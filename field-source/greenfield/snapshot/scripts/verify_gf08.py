#!/usr/bin/env python3
"""Self-contained GF08 source manifest, provenance, and status verifier."""
from pathlib import Path
import hashlib,json,re,sys
R=Path(__file__).resolve().parent.parent
H=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
try:
  m=json.loads((R/'metadata/manifest.json').read_text())['files']
  actual={p.relative_to(R).as_posix() for p in R.rglob('*') if p.is_file()}
  assert actual==set(m)|{'metadata/manifest.json'},'FILE_SET_DRIFT'
  for n,h in m.items():assert H(R/n)==h,'HASH_DRIFT:'+n
  s=json.loads((R/'metadata/GF08_STATUS.json').read_text())
  l=json.loads((R/'metadata/GF08_REUSE_LEDGER.json').read_text())
  assert s['release']=='NOT_RELEASE' and s['classification']=='NON_RELEASE_WORK', 'UNAUTHORIZED_RELEASE'
  assert s['cumulative_exact_hf19_files']==206 and l['gf08_new_hf19']==9 and l['cumulative_hf19_exact']==206,'BAD_COUNTS'
  for n,h in l['gf08_new_original_member_sha256'].items():assert H(R/'adopted_hf19'/n)==h,'ORIGINAL_HF19_HASH_DRIFT:'+n
  for n,h in l['ghc_filesystem_vendor_member_sha256'].items():assert H(R/n)==h,'GHC_VENDOR_CHANGED:'+n
  assert len(l['gf08_new_original_member_sha256'])==9 and len(l['ghc_filesystem_vendor_member_sha256'])==8,'SOURCE_REUSE_SET_DRIFT'
  assert all(s[k]=='NOT_RUN' for k in ('windows_msvc_x64_release','windows_msvc_x64_debug','windows_abi_export_host','windows_original_owner_object_compile','windows_iocp','game_consumer')),'FORGED_NATIVE_RESULT'
  assert s['lsx']=='NOT_IMPLEMENTED' and s['surge_starts_server'] is False and s['gf_preloader_links_release_owner'] is False and s['dllshutdown_zero_owner'] is True,'FAKE_RUNTIME_CLAIM'
  exports=(R/'exports/preloader_l.def').read_text()
  assert set(re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',exports,re.M))=={('DllMain','1'),('SURGE','2'),('DllShutdown','3')},'ABI_DRIFT'
  cm=(R/'CMakeLists.txt').read_text()
  link=re.search(r'target_link_libraries\(gf_preloader PRIVATE([^)]*)\)',cm)
  assert link and not any(x in link.group(1) for x in ('gf08_original_win32_owner_units','gf07_real_dependencies','gf_hf19_dispatch_units','gf_hf19_asio_owner','gf_hf19_bridge_units','gf06_config_units')),'EARLY_SHIPPING_LINK'
  assert 'option(GF08_ENABLE_NATIVE_OWNER_OBJECTS' in cm and 'OFF)' in cm.split('option(GF08_ENABLE_NATIVE_OWNER_OBJECTS',1)[1].splitlines()[0], 'WIN32_OWNER_NOT_OPT_IN'
  for n in ('release_server_acceptor.cpp','release_server_lifecycle.cpp','release_server_worker.cpp'):
    assert '#error' in (R/'adopted_hf19/src/runtime'/n).read_text().splitlines()[1], 'WINDOWS_ONLY_GUARD_DRIFT:'+n
  tests=['test_filesystem_path.cpp','test_asio_session_timeout_policy.cpp','test_asio_delivery_completion.cpp']
  for n in tests: assert n in cm,'MISSING_ORIGINAL_TEST:'+n
  assert (R/'scripts/RUN_WINDOWS_GF08.cmd').exists(),'NO_NATIVE_HANDOFF'
  print('GF08_SOURCE_VERIFY_PASS',len(m),'files; 9 new direct HF19; ghc filesystem 8/8; NOT_RELEASE')
except Exception as e:
  print('GF08_VERIFY_FAIL',str(e));sys.exit(1)
