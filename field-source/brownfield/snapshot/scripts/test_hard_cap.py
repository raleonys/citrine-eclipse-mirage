#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, tempfile, sys
r=Path(__file__).resolve().parents[1]
cxx=shutil.which('g++') or shutil.which('clang++')
if not cxx:
    print('HARDCAP_CONVERGENCE: FAIL - no C++ compiler'); sys.exit(1)
srcs=[
    r/'tests/unit/test_hardcap_convergence.cpp',
    r/'src/config/runtime_configuration.cpp',
    r/'src/config/runtime_configuration_parser.cpp',
    r/'src/config/config2_pegtl.cpp',
    r/'src/config/runtime_configuration_storage.cpp',
    r/'third_party/tinyxml2/tinyxml2.cpp',
    r/'src/config/store_values.cpp',
    
    r/'src/config/store_parser.cpp',
    r/'src/config/store_materialization.cpp',
    r/'src/config/store_runtime.cpp',
    r/'src/config/store_service_bridge.cpp',
    r/'src/config/settings_values.cpp',
    r/'src/runtime/service_state.cpp',
    r/'src/runtime/service_state_storage.cpp',
    r/'src/runtime/service_state_parser.cpp',
    r/'src/runtime/service_state_grant.cpp',
]
with tempfile.TemporaryDirectory(prefix='hardcap_') as td:
    exe=Path(td)/'hardcap'
    cmd=[cxx,'-std=c++20','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-I',str(r/'include'),'-I',str(r/'third_party/pegtl/upstream/include')]+[str(x) for x in srcs]+['-o',str(exe)]
    subprocess.run(cmd,cwd=r,check=True)
    subprocess.run([str(exe)],cwd=r,check=True)
print('HARDCAP_CONVERGENCE: PASS (6/6)')
