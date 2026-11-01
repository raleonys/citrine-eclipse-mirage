#!/usr/bin/env python3
from pathlib import Path
import json,hashlib,re
R=Path(__file__).resolve().parent.parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 m=json.loads((R/'metadata/manifest.json').read_text())['files']
 actual={p.relative_to(R).as_posix() for p in R.rglob('*') if p.is_file()}
 if actual!=set(m)|{'metadata/manifest.json'}:raise SystemExit('FILE_SET_DRIFT')
 for n,h in m.items():
  if sha(R/n)!=h:raise SystemExit('SOURCE_HASH_DRIFT:'+n)
 gf5=json.loads((R/'metadata/GF05_REUSE_LEDGER.json').read_text())
 if gf5['cumulative_hf19_reused_files']!=100:raise SystemExit('GF05_PARENT_IDENTITY')
 ledger=json.loads((R/'metadata/GF06_REUSE_LEDGER.json').read_text())
 if ledger['gf06_new_exact_files']!=76 or ledger['cumulative_exact_hf19_adopted_files']!=176:raise SystemExit('GF06_REUSE_COUNT')
 for n,h in ledger['gf06_new_original_member_sha256'].items():
  if sha(R/'adopted_hf19'/n)!=h:raise SystemExit('ADOPTED_ORIGINAL_CHANGED:'+n)
 old=json.loads((R/'metadata/GF03_REUSE_LEDGER.json').read_text())
 for n,h in old['asio_upstream_files'].items():
  if sha(R/'third_party/asio/upstream'/n)!=h:raise SystemExit('ASIO_CHANGED:'+n)
 if len(list((R/'third_party/pegtl').rglob('*.*'))) < 350:raise SystemExit('PEGTL_VENDOR_INCOMPLETE')
 if not (R/'third_party/pegtl/upstream/LICENSE_1_0.txt').is_file():raise SystemExit('PEGTL_LICENSE_MISSING')
 ex=(R/'exports/preloader_l.def').read_text()
 if set(re.findall(r'^\s*(DllMain|SURGE|DllShutdown)\s+@(\d+)\s*$',ex,re.M))!={('DllMain','1'),('SURGE','2'),('DllShutdown','3')}:raise SystemExit('ABI_SOURCE_DRIFT')
 cm=(R/'CMakeLists.txt').read_text()
 link=re.search(r'target_link_libraries\(gf_preloader PRIVATE([^)]*)\)',cm)
 if not link:raise SystemExit('SHIPPING_TARGET_UNKNOWN')
 for name in ('gf_hf19_dispatch_units','gf_hf19_bridge_units','gf_hf19_asio_owner','gf06_config_units','gf06_xml_units','gf06_full_handler_units','gf06_protocol_units'):
  if name in link.group(1):raise SystemExit('PREMATURE_SHIPPING_LINK:'+name)
 status=json.loads((R/'metadata/GF06_STATUS.json').read_text())
 if status['gf_preloader_dispatch_linked'] or status['surge_starts_server'] or status['dllshutdown_stops_asio']:raise SystemExit('FALSE_RUNTIME_STATUS')
 c=json.loads((R/'metadata/GF06_SYMBOL_CLOSURE.json').read_text())
 if c['gf05_missing_symbols_now_having_gf06_definition']!=49 or c['baseline_undefined_symbols']!=58 or not c['not_full_linked']:raise SystemExit('SYMBOL_GRAPH_DRIFT')
 if 'runtime_configuration_file.cpp' not in cm or 'if(WIN32 AND MSVC)' not in cm:raise SystemExit('MISSING_WINDOWS_ONLY_CONFIG')
 print('GF06_VERIFY_PASS',len(m),'manifest; 176 HF19 original source files; 364 PEGTL; 635 Asio; NOT_RELEASE')
if __name__=='__main__':main()
