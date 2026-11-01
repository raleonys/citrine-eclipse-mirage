#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, sys
r=Path(__file__).resolve().parents[1]
cxx=shutil.which('g++') or shutil.which('clang++')
if not cxx:
    print('PORTABLE_SOURCE_SYNTAX: FAIL - no C++ compiler'); sys.exit(1)
excluded_rel={
    'src/dll_entry.cpp',
    'src/config/runtime_configuration_file.cpp',
    'src/net/winsock_native.cpp',
    'src/net/winsock_lifecycle_native.cpp',
    'src/debug/runtime/event_trace.cpp',
    'src/debug/runtime/trace_format.cpp',
    'src/config/process_configuration.cpp',
    'src/debug/platform/crypt_observer_diagnostics.cpp',
    'src/debug/platform/optional_host_scan.cpp',
    'src/debug/runtime/debug_trace.cpp',
    'src/platform/connect_redirect_hook.cpp',
    'src/platform/crypt_observer.cpp',
    'src/platform/core_hooks.cpp', # Windows-only; separate bounded synthetic Windows compile
    'src/platform/addon_hooks.cpp', # Windows-only MinHook detours; direct source-included Win32-shim compile follows below
    'src/platform/process_environment.cpp',
    'src/platform/registry_hooks.cpp',
    'src/platform/registry_hook_runtime.cpp',
    'src/platform/registry_wrappers.cpp',
    'src/platform/minhook_runtime.cpp',
    'src/runtime/heap_allocator.cpp',
    'src/runtime/avatar_path.cpp',
    'src/runtime/bootstrap_thread.cpp',
    'src/runtime/fatal_termination.cpp',
    'src/runtime/ptd_fls.cpp',
    'src/runtime/release_server_acceptor.cpp',
    'src/runtime/asio_release_bridge.cpp',  # Windows-only shipping IOCP bridge; compiled by MSVC
    'src/runtime/release_server_lifecycle.cpp',
    'src/runtime/release_service_lifetime.cpp',
    'src/runtime/release_server_worker.cpp',
    'src/runtime/secondary_integration_entry.cpp',
}
files=[]; failed=[]
for p in sorted((r/'src').rglob('*.cpp')):
    rel=p.relative_to(r).as_posix()
    if rel in excluded_rel:
        continue
    cmd=[cxx,'-std=c++20','-Wall','-Wextra','-Werror','-Wno-error=volatile','-fsyntax-only','-DPRELOADER_L_TRACE_DIAGNOSTICS=0','-I'+str(r/'include'),'-I'+str(r/'src'/'debug'),'-I'+str(r/'third_party/asio/upstream/include'),'-I'+str(r/'third_party/pegtl/upstream/include'),'-I'+str(r/'third_party/filesystem/upstream/include')]
    q=subprocess.run(cmd+[str(p)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    files.append(p)
    if q.returncode:
        failed.append((p.relative_to(r).as_posix(),q.stderr[-1800:]))
for rel in ['src/debug/runtime/event_trace.cpp','src/debug/runtime/trace_format.cpp']:
    p=r/rel
    cmd=[cxx,'-std=c++20','-Wall','-Wextra','-Werror','-Wno-error=volatile','-fsyntax-only','-DPRELOADER_L_TRACE_DIAGNOSTICS=1','-I'+str(r/'include'),'-I'+str(r/'src'/'debug'),'-I'+str(r/'third_party/asio/upstream/include'),'-I'+str(r/'third_party/pegtl/upstream/include'),'-I'+str(r/'third_party/filesystem/upstream/include'),str(p)]
    q=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    files.append(p)
    if q.returncode: failed.append((rel,q.stderr[-1800:]))
# Do not count Windows-only optional hooks as portable, but still compile their
# unchanged production implementation under the explicit, SHA-tracked Win32 test shim.
shim=r/'tests/support/optional_hook_api_shim'
unit=r/'tests/unit/test_addon_hooks_integration.cpp'
shim_check=subprocess.run([cxx,'-std=c++17','-Wall','-Wextra','-fsyntax-only','-D_WIN32','-I'+str(shim),'-I'+str(r/'include'),str(unit)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
if shim_check.returncode:failed.append(('src/platform/addon_hooks.cpp (Windows shim)',shim_check.stderr[-1800:]))
else:print('OPTIONAL_HOOKS_WINDOWS_SHIM_SYNTAX: PASS (unchanged production .cpp included in test)')
if failed:
    print('PORTABLE_SOURCE_SYNTAX: FAIL')
    for n,e in failed[:20]: print('---',n,'---\n'+e)
    sys.exit(1)
print(f'PORTABLE_SOURCE_SYNTAX: PASS ({len(files)}/{len(files)})')
