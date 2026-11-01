#!/usr/bin/env python3
from pathlib import Path
import shutil,subprocess,tempfile,sys
r=Path(__file__).resolve().parents[1]
cxx=shutil.which('g++') or shutil.which('clang++')
if not cxx:
    print('MODEL_SETTINGS_SEMANTIC_RUNTIME: FAIL - no C++ compiler'); sys.exit(1)
with tempfile.TemporaryDirectory(prefix='semantic_settings_') as td:
    exe=Path(td)/'semantic_settings'
    cmd=[cxx,'-std=c++17','-O2','-Wall','-Wextra','-Werror','-I',str(r/'include'),
         str(r/'src/runtime/service_policy.cpp'),
         str(r/'src/runtime/service_values.cpp'),
         str(r/'src/runtime/service_settings.cpp'),
         str(r/'src/runtime/service_xml.cpp'),
         str(r/'tests/unit/test_model_settings_semantic_runtime.cpp'),'-o',str(exe)]
    subprocess.run(cmd,cwd=r,check=True)
    subprocess.run([str(exe)],cwd=r,check=True)
