#!/usr/bin/env python3
from pathlib import Path
import shutil,subprocess,tempfile,sys
r=Path(__file__).resolve().parents[1]
sec=(r/'src/runtime/secondary_integration_entry.cpp').read_text()
boot=(r/'src/runtime/bootstrap_thread.cpp').read_text()
reg=(r/'src/platform/registry_hook_runtime.cpp').read_text()
wrap=(r/'src/platform/registry_wrappers.cpp').read_text()
cm=(r/'CMakeLists.txt').read_text()
patch=(r/'scripts/build_orchestration/PatchMinHookSafeFreeze.cmake').read_text()
runtime=(r/'src/platform/minhook_runtime.cpp').read_text()
core=(r/'src/platform/core_hooks.cpp').read_text()
addons=(r/'src/platform/addon_hooks.cpp').read_text()
helper_start=boot.find('PostLoaderHookInstallResult install_post_loader_minhook_surface(')
helper_end=boot.find('DWORD run_release_bootstrap_core() noexcept',helper_start)
helper=boot[helper_start:helper_end] if helper_start>=0 and helper_end>helper_start else ''
checks={
 'secondary schedules only': 'schedule_release_bootstrap(module)' in sec,
 'secondary has no surge install': 'install_surge_native_hooks_main_module()' not in sec,
 'secondary has no crypt install': 'install_crypt_query_observer_main_module()' not in sec,
 'secondary has no registry install': 'install_windows_registry_minhook_early()' not in sec,
 'secondary has no connect redirect install': 'install_connect_redirect()' not in sec,
 'secondary has no MinHook apply': 'MH_ApplyQueued' not in sec,
 'post-loader surface helper': 'PostLoaderHookInstallResult install_post_loader_minhook_surface(' in boot and 'AddonHookSelection selection' in helper,
 'single transaction begin': 'begin_minhook_batch_transaction()' in boot,
 'single transaction commit': 'commit_minhook_batch_transaction()' in boot,
 'transaction staging backend': 'batch_transaction_staging()' in runtime and 'append_transaction_batch' in runtime,
 'transaction rollback backend': 'rollback_minhook_batch_transaction()' in runtime and 'rollback_incomplete' in runtime,
 'PTD before post-loader transaction': 0 <= boot.find('initialize_ptd_runtime_windows()') < boot.find('const auto hook_install = install_post_loader_minhook_surface(selection);'),
 'transaction order': 0 <= helper.find('begin_minhook_batch_transaction()') < helper.find('stage_core_hooks()') < helper.find('stage_addon_hooks(selection.mask)') < helper.find('commit_minhook_batch_transaction()') and 0 <= core.find('install_crypt_query_observer_main_module()') < core.find('install_windows_registry_minhook_early()') < core.find('install_connect_redirect()'),
 'PTD failure pre-transaction unwind': 0 <= boot.find('initialize_ptd_runtime_windows()') < boot.find('const auto hook_install = install_post_loader_minhook_surface(selection);') and 'finalize_pre_acquire_bootstrap_failure()' in boot[boot.find('if (!preloader_l::runtime::initialize_ptd_runtime_windows())'):boot.find('if (preloader_l::runtime::bootstrap_stop_requested())',boot.find('if (!preloader_l::runtime::initialize_ptd_runtime_windows())'))],
 'post-loader crypt': 'install_crypt_query_observer_main_module()' in core and 'stage_core_hooks()' in helper,
 'post-loader registry passive': 'install_windows_registry_minhook_early()' in core and 'stage_core_hooks()' in helper,
 'post-loader connect redirect': 'install_connect_redirect()' in core and 'stage_core_hooks()' in helper,
 'registry passive state': 'g_registry_policy_active' in reg and 'set_registry_policy_active(false)' in reg,
 'registry later activation': 'set_registry_policy_active(true)' in reg,
 'registry passthrough while passive': wrap.count('if (!registry_policy_active())') == 5,
 'safe freeze patch staged': 'PatchMinHookSafeFreeze.cmake' in cm and '_preloader_l_minhook_patched' in cm,
 'safe freeze backend': 'NtGetNextThread' in patch and 'PRELOADER_L_MH_SAFE_FREEZE_V1' in patch,
 'HF07 explicit peer preparation result':'PEER_PREPARATION_RESULT' in patch and 'PEER_UNRESOLVED' in patch and 'PEER_TERMINATED' in patch,
 'HF07 retained peer ownership':'g_retainedThreads' in patch and 'g_emergencyThread' in patch and 'RetryRetainedThreadOwnership' in patch,
 'HF07 checked resume close heap':'resumeResult = ResumeThread' in patch and 'if (CloseHandle(pItem->handle))' in patch and 'if (!HeapFree(g_hHeap, 0, pThreads->pItems))' in patch,
 'HF07 unfreeze status propagation':'FinishFreeze' in patch and 'status = FinishFreeze(&threads, status);' in patch,
 'toolhelp not used by replacement backend': 'CreateToolhelp32Snapshot(' not in patch.replace('CreateToolhelp32Snapshot.',''),
 'bootstrap thread normal create': 'CreateThread(nullptr, 0, &release_bootstrap_thread, pinned,' in boot,
 'no suspended bootstrap dispatch': 'ResumeThread(' not in boot and 'kCreateSuspended' not in boot,
 'shipping diagnostic logger removed': not (r/'src/platform/minhook_diag_windows.cpp').exists() and not (r/'include/preloader_l/platform/minhook_diag_windows.hpp').exists(),
 'no redundant Surge upgrade': 'upgrade_surge_native_hooks_global' not in boot,
 'one Crypt install call': core.count('install_crypt_query_observer_main_module()') == 1 and boot.count('install_crypt_query_observer_main_module()') == 0,
 'one connect redirect install call': core.count('const auto connect_status = install_connect_redirect();') == 1 and boot.count('const auto connect_status = install_connect_redirect();') == 0,
 'transaction all-hooks queue convergence': 'queue_all_transaction_hooks(false)' in runtime and 'MH_ALL_HOOKS' in runtime,
}
# Execute the actual source transformer against the pinned vendored hook.c and
# inspect the emitted ownership contract, not just PatchMinHookSafeFreeze.cmake tokens.
cmake=shutil.which('cmake')
if not cmake:
    print('MINHOOK_SAFE_FREEZE_TRANSACTION_CONTRACT: FAIL - cmake missing');sys.exit(1)
with tempfile.TemporaryDirectory(prefix='minhook_safe_freeze_') as td_s:
    td=Path(td_s); hook=td/'hook.c'
    shutil.copyfile(r/'third_party/minhook/upstream/src/hook.c',hook)
    subprocess.run([cmake,f'-DMINHOOK_HOOK_C={hook}','-P',str(r/'scripts/build_orchestration/PatchMinHookSafeFreeze.cmake')],cwd=r,check=True)
    generated=hook.read_text(encoding='utf-8',errors='replace')
    generated_checks={
      'generated explicit peer outcome':'PEER_PREPARATION_RESULT' in generated and 'PEER_UNRESOLVED' in generated,
      'generated retained owner':'g_retainedThreads' in generated and 'g_emergencyThread' in generated,
      'generated checked resume':'resumeResult = ResumeThread' in generated and 'resumeResult != 0xFFFFFFFF' in generated,
      'generated checked close':'if (CloseHandle(pItem->handle))' in generated and 'ERROR_INVALID_HANDLE' in generated,
      'generated checked heap':'if (!HeapFree(g_hHeap, 0, pThreads->pItems))' in generated,
      'generated cleanup propagated':generated.count('status = FinishFreeze(&threads, status);') == 4,
      'generated retry before uninit/all-hooks':'if (!RetryRetainedThreadOwnership())' in generated,
      'generated enumeration outcomes truthful':all(x in generated for x in ['THREAD_ENUMERATION_API_FAILED','THREAD_ENUMERATION_ALLOC_FAILED','THREAD_ENUMERATION_OWNERSHIP_FAILED']),
      'generated API failure non-allocation':'return MH_UNKNOWN;' in generated and 'enumeration == THREAD_ENUMERATION_ALLOC_FAILED' in generated,
    }
    checks.update(generated_checks)

bad=[k for k,v in checks.items() if not v]
if bad:
    print('MINHOOK_SAFE_FREEZE_TRANSACTION_CONTRACT: FAIL - '+', '.join(bad));sys.exit(1)
print(f'MINHOOK_SAFE_FREEZE_TRANSACTION_CONTRACT: PASS ({len(checks)}/{len(checks)})')
