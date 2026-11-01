#!/usr/bin/env python3
"""HF09 exact-shipping-source QueryPresence/QueryContent/QueryEntitlements joint runtime regression.
The 21 named assertions exercise real production handlers, XML pipeline and Config2 state.
Linux GCC/Clang source testing is NOT Windows/native/game acceptance.
"""
from pathlib import Path
import shutil,subprocess,tempfile,sys
r=Path(__file__).resolve().parents[1]
common=['src/handlers/store_routes.cpp','src/handlers/query_presence_route.cpp','src/xml/request_parser.cpp','src/xml/request_children.cpp','src/xml/lsx_framing.cpp','src/xml/response_builder.cpp','src/xml/value_codec.cpp','src/xml/value_codec_utf8.cpp','src/xml/value_codec_normalize.cpp','src/xml/value_codec_semantic.cpp','src/xml/attribute_parser.cpp','src/config/config2_pegtl.cpp','src/config/runtime_configuration.cpp','src/config/runtime_configuration_parser.cpp','src/config/runtime_configuration_storage.cpp','src/config/runtime_provider.cpp','src/config/settings_values.cpp','src/config/settings_provider.cpp','src/handlers/selector_pipeline.cpp','src/runtime/service_state.cpp','src/runtime/service_state_storage.cpp','src/runtime/service_state_parser.cpp','src/runtime/service_state_grant.cpp','third_party/tinyxml2/tinyxml2.cpp']
with tempfile.TemporaryDirectory(prefix='hf09_qp_qce_') as td:
 for compiler in ('g++','clang++'):
  tool=shutil.which(compiler)
  if not tool:
   print('HF09_QP_QCE: FAIL compiler not available',compiler);sys.exit(2)
  exe=Path(td)/('test_'+compiler.replace('+','p'))
  cmd=[tool,'-std=c++20','-O0','-Wno-unused-function','-Wno-misleading-indentation','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I',str(r/'include'),'-I',str(r/'third_party/pegtl/upstream/include'),'-I',str(r/'tests/support'),'-I',str(r/'src/debug'),'-I',str(r/'src/runtime'),str(r/'tests/unit/test_presence_content_entitlements.cpp')]+[str(r/p) for p in common]+['-o',str(exe)]
  c=subprocess.run(cmd,capture_output=True,text=True,timeout=180)
  if c.returncode:
   print('HF09_QP_QCE: FAIL compile',compiler,c.stderr[-1200:]);sys.exit(1)
  v=subprocess.run([str(exe)],capture_output=True,text=True,timeout=20)
  checks=[x for x in v.stdout.splitlines() if x.startswith('CHECK|')]
  good= v.returncode==0 and len(checks)==21 and all(x.endswith('|PASS') for x in checks)
  print('HF09_QP_QCE:',compiler,str(sum(x.endswith('|PASS') for x in checks))+'/21', 'PASS' if good else 'FAIL')
  if not good:
   print(v.stdout[-1500:]);print(v.stderr[-500:]);sys.exit(1)
print('HF09_QP_QCE_REGRESSION: PASS (real shipping source, 21/21 GCC, 21/21 Clang)')
