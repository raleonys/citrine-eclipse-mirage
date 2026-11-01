#!/usr/bin/env python3
from pathlib import Path
import re,sys
r=Path(__file__).resolve().parents[1]
cm=(r/'CMakeLists.txt').read_text()
ps=(r/'scripts/test_production_cpp_syntax.ps1').read_text()
m=re.search(r'add_library\(preloader_l SHARED\s*(.*?)\n\)',cm,re.S)
if not m:
    print('ACTIVE_PRODUCTION_CPP_SOURCE_GRAPH: FAIL - CMake production block'); sys.exit(1)
cm_entries=re.findall(r'(?m)^\s*([A-Za-z0-9_./-]+\.(?:cpp|c|rc|def|asm))\s*$',m.group(1))
cm_sources=[x for x in cm_entries if x.startswith('src/') and x.endswith('.cpp')]
errors=[]
for rel in cm_entries:
    if not rel.startswith(('src/','resources/','exports/')):
        errors.append('non-production root leaked into production target: '+rel)
# The Windows compiler gate must derive the exact list from CMake; a second
# literal source inventory is forbidden because it previously drifted.
for token in ['Get-Content $CMakePath -Raw','add_library\\(preloader_l SHARED','ForEach-Object { $_.Groups[1].Value }']:
    if token not in ps: errors.append('Windows syntax gate does not derive CMake source graph: '+token)
if '$Sources = @(' not in ps: errors.append('missing derived source array')
for token in ['/Zs','third_party\\minhook\\upstream\\include','/DPRELOADER_L_RELEASE_BOOTSTRAP=1','/DPRELOADER_L_TRACE_DIAGNOSTICS=0']:
    if token not in ps: errors.append('missing '+token)
if '/DPRELOADER_L_NATIVE_ACCEPTANCE' in ps: errors.append('native acceptance macro leaked into production syntax gate')
if 'src/debug/runtime/event_trace.cpp' in m.group(1) or 'src/debug/runtime/debug_trace.cpp' in m.group(1): errors.append('trace implementation leaked into normalized CMake graph')
if errors:
    print('ACTIVE_PRODUCTION_CPP_SOURCE_GRAPH: FAIL'); [print(' -',e) for e in errors]; sys.exit(1)
print(f'ACTIVE_PRODUCTION_CPP_SOURCE_GRAPH: PASS ({len(cm_sources)}/{len(cm_sources)}, source-only CMake-derived Windows /Zs inventory; Windows compiler not executed by this gate)')

for forbidden in ['PRELOADER_L_VERSION_CODE','PRELOADER_L_BUILD_TAG','preloader_l-3.1.3-martavex','preloader_l-3.1.4-leporvex']:
    if forbidden in ps: raise SystemExit('ACTIVE_PRODUCTION_CPP_SYNTAX_SURFACE: FAIL embedded release identity '+forbidden)
