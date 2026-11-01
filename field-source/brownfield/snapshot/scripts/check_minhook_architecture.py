#!/usr/bin/env python3
from pathlib import Path
import re, sys
r=Path(__file__).resolve().parents[1]
errors=[]

def text(rel):
    p=r/rel
    if not p.is_file():
        errors.append(f'missing {rel}')
        return ''
    return p.read_text(encoding='utf-8',errors='replace')

cm=text('CMakeLists.txt')
runtime=text('src/platform/minhook_runtime.cpp')
runtime_h=text('include/preloader_l/platform/minhook_runtime.hpp')
registry=text('src/platform/registry_hooks.cpp')
registry_wrap=text('src/platform/registry_wrappers.cpp')
crypt=text('src/platform/crypt_observer.cpp')
connect_redirect=text('src/platform/connect_redirect_hook.cpp')
bootstrap=text('src/runtime/bootstrap_thread.cpp')

pin='8af6b4acae5a9388fd742b56fa79ece89d96f823'
if pin not in cm: errors.append('MinHook commit pin drift')
if 'target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)' not in cm:
    errors.append('shipping target does not link MinHook')
if 'src/platform/minhook_runtime.cpp' not in cm:
    errors.append('MinHook runtime missing from production graph')
if '    src/platform/addon_hooks.cpp\n' not in cm: errors.append('HF17 optional hook translation unit not in shipping source graph')
optional=text('src/platform/addon_hooks.cpp')
if optional.count('create_minhook(')!=1 or optional.count('enable_minhook_batch(')!=1: errors.append('HF17 optional hooks lack single MinHook owner / batch publication')
for hook in ('CreateProcessA','CreateProcessW','SHGetFolderPathA','SHGetFolderPathW'):
    if hook not in optional: errors.append('HF17 missing optional hook: '+hook)
if 'stage_addon_hooks(' not in bootstrap or 'uninstall_addon_hooks()' not in bootstrap: errors.append('HF17 optional hook bootstrap/teardown missing')
if 'enable_language(ASM_MASM)' in cm or 'crypt_query_tail_wrapper_x64.asm' in cm:
    errors.append('legacy MASM hook path still active')

legacy=[
 'src/platform/inline_hook_backend_windows.cpp',
 'src/platform/inline_hook_allocator_windows.cpp',
 'src/platform/inline_hook_page_trap_windows.cpp',
 'src/platform/inline_hook_patch_windows.cpp',
 'src/platform/inline_hook_quiescence.cpp',
 'src/platform/x64_trampoline_relocator.cpp',
 'src/platform/model_hook_allocator_strict_windows.cpp',
 'src/platform/iat_hook_backend_windows.cpp',
 'src/platform/surge_native_hooks_iat_windows.cpp',
 'src/platform/crypt_query_tail_wrapper_x64.asm',
]
for rel in legacy:
    if (r/rel).exists(): errors.append('legacy hook engine file still present: '+rel)
    if rel in cm: errors.append('legacy hook engine still in CMake: '+rel)

for token in ['MH_Initialize()', 'MH_CreateHook(', 'MH_QueueEnableHook(', 'MH_ApplyQueued()',
              'MH_QueueDisableHook(', 'MH_RemoveHook(', 'MH_Uninitialize()']:
    if token not in runtime: errors.append('missing MinHook runtime primitive: '+token)
for token in ['kGateInstalling','kGateActive','kGateDraining','kGateDirect',
              'atomic_store_pointer(&gate->forwarding_target','wait_gate(gates[i])',
              'conservatively marks every', 'restore_disabled_after_failed_enable']:
    if token not in runtime: errors.append('missing publication-aware lifetime contract: '+token)

registry_names=['preloader_l_RegOpenKeyExA','preloader_l_RegOpenKeyExW','preloader_l_RegQueryValueExA',
                'preloader_l_RegQueryValueExW','preloader_l_RegCloseKey']
for idx,name in enumerate(registry_names):
    if name not in registry_wrap: errors.append('missing registry detour '+name)
    # S0221: inspect each active wrapper body; a token in another wrapper
    # must never mask a lost original-forwarding path.
    live = re.sub(r'/\*.*?\*/|//[^\n]*', '', registry_wrap, flags=re.S)
    body = re.search(r'\b'+re.escape(name)+r'\s*\([^;{]*\)\s*noexcept\s*\{(.*?)^\}',live,re.M|re.S)
    if not body:
        errors.append('registry wrapper body not found '+name)
    else:
        expected = f'minhook_gate_forwarding_target(registry_quiescence_gate({idx}u))'
        if expected not in re.sub(r'\s+', '', body.group(1)):
            errors.append('registry wrapper has no exact gate forwarding '+name)
if 'registry_hook_count=5u' not in text('src/platform/registry_hook_runtime_internal.hpp').replace(' ',''):
    errors.append('registry hook count is not five')
if 'enable_minhook_batch(batch, gates, registry_hook_count)' not in registry:
    errors.append('registry hooks are not batch-enabled through MinHook')

for rel in ('src/platform/surge_hooks.cpp','src/platform/surge_inline_hooks.cpp',
            'src/platform/surge_wrapper_hooks.cpp','src/platform/surge_policy.cpp',
            'include/preloader_l/platform/surge_hooks.hpp','include/preloader_l/platform/surge_policy.hpp'):
    if (r/rel).exists(): errors.append('retired legacy hook remains: '+rel)
if (r/'src/platform/minhook_diag_windows.cpp').exists() or (r/'include/preloader_l/platform/minhook_diag_windows.hpp').exists():
    errors.append('temporary MinHook diagnostic logger remains in release candidate')
if 'minhook_diag_windows.cpp' in cm:
    errors.append('temporary MinHook diagnostic logger remains in production CMake')
if 'crypt_query_detour(' not in crypt or 'create_minhook(' not in crypt:
    errors.append('CryptQueryObject is not a C++ MinHook detour')
if 'redirected_connect(' not in connect_redirect or 'create_minhook(' not in connect_redirect:
    errors.append('connect redirect is not a MinHook detour')
if 'GetProcAddress(ws2, "connect")' not in connect_redirect:
    errors.append('connect redirect does not target ws2_32!connect')
if 'GetProcAddress(ws2, "htons")' in connect_redirect or 'redirected_htons(' in connect_redirect:
    errors.append('process-wide htons interception returned')
if 'shutdown_minhook_runtime()' not in bootstrap:
    errors.append('bootstrap does not shut down MinHook runtime')
if 'shutdown_inline_hook_runtime()' in bootstrap:
    errors.append('legacy inline runtime shutdown still referenced')
for token in ['begin_minhook_batch_transaction()', 'commit_minhook_batch_transaction()', 'rollback_minhook_batch_transaction_with_timeout(']:
    if token not in bootstrap: errors.append('MinHook transaction bootstrap primitive missing: '+token)
for token in ['batch_transaction_staging()', 'append_transaction_batch', 'BatchTransactionMode::faulted']:
    if token not in runtime: errors.append('MinHook transaction runtime invariant missing: '+token)
if 'PatchMinHookSafeFreeze.cmake' not in cm: errors.append('SAFE-FREEZE MinHook patch not staged')
if 'patch_minhook_startup_nofreeze.cmake' in cm or 'PRELOADER_L_MH_ApplyQueuedNoFreezeTransactional' in runtime:
    errors.append('superseded startup no-freeze path remains active')
if 'MinHookPublishBinding' in runtime_h or 'MinHookPublishBinding' in runtime:
    errors.append('external original publication authority still present')
for rel,body in [('registry wrappers',registry_wrap),('Crypt detour',crypt),('connect detour',connect_redirect)]:
    if 'minhook_gate_forwarding_target(' not in body:
        errors.append(rel+' does not load forwarding authority from the MinHook gate')

active_hook_sources='\n'.join([runtime,registry,registry_wrap,crypt,connect_redirect])
for forbidden in ['AddVectoredExceptionHandler(', 'RemoveVectoredExceptionHandler(', 'VirtualProtect(',
                  'VirtualAlloc(', 'VirtualFree(', 'FlushInstructionCache(']:
    if forbidden in active_hook_sources:
        errors.append('project-owned patch/VEH primitive remains active: '+forbidden)

# Production source graph existence.
m=re.search(r'add_library\(preloader_l SHARED\s*(.*?)\n\)',cm,re.S)
if not m:
    errors.append('cannot parse production CMake source graph')
    prod=[]
else:
    entries=re.findall(r'(?m)^\s*([A-Za-z0-9_./-]+\.(?:cpp|c|rc|def|asm))\s*$',m.group(1))
    prod=[rel for rel in entries if rel.startswith('src/') and rel.endswith('.cpp')]
    for rel in entries:
        if not rel.startswith(('src/','resources/','exports/')):
            errors.append('non-production root leaked into production target: '+rel)
        if not (r/rel).is_file(): errors.append('CMake source missing: '+rel)

if errors:
    print('MINHOOK_ARCHITECTURE: FAIL')
    for e in errors: print(' -',e)
    sys.exit(1)
print(f'MINHOOK_ARCHITECTURE: PASS (production_cpp={len(prod)}, detours=11/11, one_safe_freeze_transaction, legacy_engine=absent)')
