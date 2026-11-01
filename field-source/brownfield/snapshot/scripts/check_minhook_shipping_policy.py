#!/usr/bin/env python3
from pathlib import Path
import sys,re
r=Path(__file__).resolve().parents[1]
cm=(r/'CMakeLists.txt').read_text(encoding='utf-8',errors='replace')
errors=[]
pin='8af6b4acae5a9388fd742b56fa79ece89d96f823'
required=[
 ('Windows x64 guard','preloader_l is a Windows x64 DLL project' in cm and 'preloader_l must be built for x64' in cm),
 ('C and CXX enabled','project(preloader_l LANGUAGES C CXX)' in cm),
 ('MinHook exact pin',pin in cm and (r/'third_party/minhook/UPSTREAM_COMMIT.txt').read_text().strip()==pin),
 ('MinHook fully vendored','third_party/minhook/upstream' in cm and 'add_library(minhook STATIC' in cm),
 ('MinHook network fallback absent',all(x not in cm for x in ['FetchContent','GIT_REPOSITORY','PRELOADER_L_MINHOOK_SOURCE_DIR'])),
 ('upstream MinHook CMake bypassed','add_subdirectory("${_preloader_l_minhook_patched}"' not in cm),
 ('MinHook target required','if(NOT TARGET minhook)' in cm),
 ('shipping links MinHook','target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)' in cm),
 ('shipping MinHook runtime matched','set_property(TARGET minhook PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")' in cm),
 ('debug MinHook runtime target','add_library(minhook_debug STATIC' in cm and 'set_property(TARGET minhook_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")' in cm),
 ('debug links runtime-matched MinHook and TinyXML2','target_link_libraries(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 minhook_debug preloader_l_tinyxml2_debug)' in cm),
 ('debug does not reuse shipping MinHook','target_link_libraries(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 minhook)' not in cm),
 ('normal CRT retained','MSVC_RUNTIME_LIBRARY "MultiThreaded"' in cm),
 ('shipping GS retained','/EHsc;/GR-;/GS>' in cm),
 ('manifest suppression retained','/MANIFEST:NO' in cm),
 ('no active MASM hook path','enable_language(ASM_MASM)' not in cm and 'crypt_query_tail_wrapper_x64.asm' not in cm),
 ('MinHook runtime in production','src/platform/minhook_runtime.cpp' in cm),
 ('SAFE-FREEZE patch staged','PatchMinHookSafeFreeze.cmake' in cm),
 ('startup no-freeze patch absent','patch_minhook_startup_nofreeze.cmake' not in cm),
 ('temporary MinHook diagnostic logger absent','minhook_diag_windows.cpp' not in cm),
]
for name,ok in required:
    if not ok: errors.append(name)
# The compatibility custom-entry branch may still link msvcrt/vcruntime manually;
# the normalized branch must not.
normal='target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)'
if normal not in cm: errors.append('normalized link line missing')
legacy_tokens=['src/platform/inline_hook_backend_windows.cpp','src/platform/iat_hook_backend_windows.cpp',
               'src/platform/surge_native_hooks_iat_windows.cpp','src/platform/x64_trampoline_relocator.cpp']
for token in legacy_tokens:
    if token in cm: errors.append('legacy source remains in CMake: '+token)
if errors:
    print('MINHOOK_SHIPPING_POLICY: FAIL')
    for x in errors: print(' -',x)
    sys.exit(1)
print('MINHOOK_SHIPPING_POLICY: PASS')
