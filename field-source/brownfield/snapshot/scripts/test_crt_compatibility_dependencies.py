#!/usr/bin/env python3
"""Static CRT mismatch and legacy-entry negative probes; no Windows acceptance claim."""
from pathlib import Path
import sys
sys.dont_write_bytecode=True  # active release source must never accumulate __pycache__ artifacts
from check_crt_transition import source_errors
r=Path(__file__).resolve().parents[1]
cm=(r/'CMakeLists.txt').read_text()
mods=[
 ('shipping_wrong_dynamic',cm.replace('TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded"','TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"')),
 ('minhook_wrong_dynamic',cm.replace('TARGET minhook PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded"','TARGET minhook PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"')),
 ('tinyxml2_wrong_dynamic',cm.replace('TARGET preloader_l_tinyxml2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded"','TARGET preloader_l_tinyxml2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"')),
 ('debug_wrong_release',cm.replace('TARGET preloader_l_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug"','TARGET preloader_l_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded"')),
 ('shipping_custom_entry',cm.replace('target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)','target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)\n/ENTRY:DllMain')),
 ('compatibility_gate_removed',cm.replace('message(FATAL_ERROR "HF12 static CRT does not support the historical custom-entry comparison; consult preserved lineage")','message(STATUS "unsafe custom entry")')),
 ('missing_vendor',cm.replace('target_link_libraries(preloader_l PRIVATE preloader_l_tinyxml2)','target_link_libraries(preloader_l PRIVATE kernel32)')),
]
if source_errors(cm):print('CRT_STATIC_NEGATIVES: FAIL positive');sys.exit(1)
for name,modified in mods:
    if modified==cm or not source_errors(modified):print('CRT_STATIC_NEGATIVES: FAIL',name);sys.exit(1)
print(f'CRT_STATIC_NEGATIVES: PASS ({len(mods)+1}/{len(mods)+1})')
