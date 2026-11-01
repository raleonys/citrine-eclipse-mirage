#!/usr/bin/env python3
"""Fail-closed current HF12 static CRT source policy."""
from pathlib import Path
import sys
sys.dont_write_bytecode=True  # active release source must never accumulate __pycache__ artifacts
from check_crt_transition import source_errors
r=Path(__file__).resolve().parents[1]
cm=(r/'CMakeLists.txt').read_text()
errs=source_errors(cm)
dm=(r/'src/dll_entry.cpp').read_text()
if 'defined(PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY)' not in dm:errs.append('historical comparison guard lost (must remain dormant)')
if 'src/runtime/freestanding_memory.cpp' in cm:errs.append('freestanding shim in active CMake')
if errs:
    print('CRT_RUNTIME_POLICY: FAIL')
    for e in errs:print(' -',e)
    sys.exit(1)
print('CRT_RUNTIME_POLICY: PASS (HF12 static runtime, no custom entry)')
