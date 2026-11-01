#!/usr/bin/env python3
from pathlib import Path
import json,hashlib,re
R=Path(__file__).resolve().parent.parent
H=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
status=json.loads((R/'metadata/GF07_STATUS.json').read_text())
ledger=json.loads((R/'metadata/GF07_REUSE_LEDGER.json').read_text())
source=json.loads((R/'metadata/manifest.json').read_text())['files']
files={p.relative_to(R).as_posix() for p in R.rglob('*') if p.is_file()}
assert files==set(source)|{'metadata/manifest.json'},'FILE_SET_DRIFT'
for n,h in source.items():assert H(R/n)==h,'HASH_DRIFT:'+n
assert status['gf_preloader_full_dispatch_linked'] is False and status['SURGE_noop'] and status['DllShutdown_zero_owner'] and status['release']=='NOT_RELEASE','FALSE_RUNTIME_CLAIM'
assert status['Windows_x64_MSVC']=='NOT_RUN' and status['native_IOCP']=='NOT_RUN','FORGED_WINDOWS_ACCEPTANCE'
assert ledger['gf07_new_exact_hf19_files']==21 and ledger['cumulative_exact_hf19_files']==197,'BAD_REUSE_COUNT'
for n,h in ledger['gf07_new_original_member_sha256'].items():assert H(R/'adopted_hf19'/n)==h,'BAD_ORIGINAL:'+n
cm=(R/'CMakeLists.txt').read_text();s=re.search(r'target_link_libraries\(gf_preloader PRIVATE([^)]*)\)',cm)
assert s and all(a not in s.group(1) for a in ('gf07_real_dependencies','gf06_full_handler_units','gf_hf19_dispatch_units','gf_hf19_asio_owner','gf_hf19_bridge_units')),'EARLY_SHIPPING_LINK'
assert 'GF07_ENABLE_WINDOWS_DEEP_LINK_PROBE' in cm,'MISSING_DEEP_LINK_GUARD'
assert '#error avatar_path.cpp is Windows-only' in (R/'adopted_hf19/src/runtime/avatar_path.cpp').read_text(),'WIN_ONLY_GUARD_REMOVED'
assert set(re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',(R/'exports/preloader_l.def').read_text(),re.M))=={('DllMain','1'),('SURGE','2'),('DllShutdown','3')},'EXPORT_DRIFT'
print('GF07_VERIFY_PASS',len(source),'files, 21 new exact HF19 members, dormant integration, NOT_RELEASE')
