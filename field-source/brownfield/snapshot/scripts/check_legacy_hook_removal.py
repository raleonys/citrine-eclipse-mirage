#!/usr/bin/env python3
"""Fail-closed HF13 active-tree removal of the two obsolete hook engines.
Historical migration ledgers and provenance are intentionally not rewritten.
"""
from pathlib import Path
import re, sys
r=Path(__file__).resolve().parents[1]
retired={
 'src/platform/surge_hooks.cpp','src/platform/surge_hooks_internal.hpp',
 'src/platform/surge_inline_hooks.cpp','src/platform/surge_policy.cpp',
 'src/platform/surge_wrapper_hooks.cpp',
 'include/preloader_l/platform/surge_hooks.hpp',
 'include/preloader_l/platform/surge_policy.hpp',
 'tests/unit/test_surge_native_policy.cpp','tests/unit/test_native_hook_status.cpp'
}
fail=[]
for rel in sorted(retired):
    if (r/rel).exists(): fail.append('legacy file persists: '+rel)
# Operating source and CMake only. Project SURGE entrypoint, Win32 ordinary
# APIs and immutable historical records are outside this hook-only contract.
roots=[r/'CMakeLists.txt',r/'src',r/'include',r/'tools',r/'tests']
for root in roots:
    files=[root] if root.is_file() else root.rglob('*')
    for p in files:
        if not p.is_file() or p.suffix.lower() not in ('.cpp','.hpp','.h','.txt'):
            continue
        rel=p.relative_to(r).as_posix()
        if rel.startswith('tests/support/research/'):
            continue
        content=p.read_text(encoding='utf-8',errors='replace')
        for term in ('Process32NextW','_Mtx_trylock','surge_hooks.hpp','surge_hooks_internal.hpp','surge_policy.hpp','surge_inline_hooks.cpp','surge_wrapper_hooks.cpp','surge_policy.cpp','surge_hooks.cpp'):
            if term in content:
                fail.append(f'{rel}: retired hook reference {term}')
for rel in ['src/runtime/bootstrap_thread.cpp','CMakeLists.txt']:
    p=r/rel
    if not p.is_file(): fail.append('missing mandatory owner '+rel)
    elif 'surge_' in p.read_text(encoding='utf-8'):
        fail.append('obsolete surge hook wiring '+rel)
# HF13 Windows logs 2026-10-14: these host prerequisites no longer include
# the retired _Mtx_trylock/MSVCP target. Preserve ADVAPI32 registry availability.
for rel,code in (
        ('tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp', '4'),
        ('tools/preloader_l_release_host/release_host_main.cpp', '9')):
    p=r/rel
    if not p.is_file():
        fail.append('required native host missing: '+rel)
        continue
    body=p.read_text(encoding='utf-8',errors='replace')
    if '.msvcp' in body:
        fail.append('retired MSVCP prerequisite field in '+rel)
    if 'struct HookPrerequisiteModules { HMODULE advapi; };' not in body:
        fail.append('unexpected hook prerequisite host layout in '+rel)
    if 'if(!hook_prerequisites.advapi) finish('+code+');' not in body:
        fail.append('missing fail-closed ADVAPI32 prerequisite in '+rel)
    if 'LoadLibraryW(L"ADVAPI32.dll")' not in body:
        fail.append('required registry hook target module preload absent in '+rel)

# Preserve still-required hook infrastructure; removal must not mean no hook engine.
minhook=(r/'src/platform').glob('*.cpp')
if not any('MH_' in p.read_text(encoding='utf-8',errors='replace') for p in minhook):
    fail.append('remaining MinHook engine implementation missing')
if fail:
    print('HF13_LEGACY_HOOK_REMOVAL: FAIL')
    for item in fail[:60]: print(' -',item)
    sys.exit(1)
print('HF13_LEGACY_HOOK_REMOVAL: PASS retired=9 files; hook-specific references absent; other MinHook retained')
