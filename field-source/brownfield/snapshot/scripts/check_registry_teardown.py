#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
policy='\n'.join((r/p).read_text() for p in [
    'src/platform/registry_policy.cpp',
    'src/platform/registry_policy_config.cpp',
    'src/platform/registry_policy_state.cpp',
    'src/platform/registry_policy_values.cpp',
])
header=(r/'include/preloader_l/platform/registry_policy.hpp').read_text()
facade=(r/'src/platform/registry_hook_runtime.cpp').read_text()
bootstrap=(r/'src/runtime/bootstrap_thread.cpp').read_text()
hooks=(r/'src/platform/registry_hooks.cpp').read_text()
uninstall=facade[facade.index('bool uninstall_windows_registry_iat_hooks() noexcept {'):facade.index('WindowsRegistryHookMode windows_registry_hook_mode() noexcept {')]
compact=''.join(uninstall.split())
hooks_compact=''.join(hooks.split())
checks={
 'fence state exists':'g_pseudo_allocation_fenced=false' in policy,
 'pseudo publication checks fence':'if(g_pseudo_allocation_fenced)' in policy and 'fresh->next=g_handles;g_handles=fresh' in policy,
 'teardown API declared':'bool begin_registry_policy_teardown() noexcept;' in header,
 'preflight facade declared':'bool prepare_windows_registry_teardown() noexcept;' in (r/'include/preloader_l/platform/registry_hook_runtime.hpp').read_text(),
 'teardown rejects outstanding pseudo':'if(n->flags&k_pseudo){unlock_state();return false;}' in policy,
 'teardown establishes fence':'g_pseudo_allocation_fenced=true' in policy,
 'uninstall preflights before policy deactivation':(
     'if(!prepare_windows_registry_teardown())returnfalse;' in compact and
     'set_registry_policy_active(false);' in compact and
     compact.index('if(!prepare_windows_registry_teardown())returnfalse;') <
     compact.index('set_registry_policy_active(false);')
 ),
 'global rollback preflight':(
     'prepare_windows_registry_teardown()' in bootstrap and
     'rollback_minhook_batch_transaction_with_timeout' in bootstrap and
     bootstrap.index('prepare_windows_registry_teardown()') <
     bootstrap.index('rollback_minhook_batch_transaction_with_timeout')
 ),
 'hook teardown owns mutation':(
     'booluninstall_registry_hooks()noexcept{' in hooks_compact and
     'disable_minhook_batch(' in hooks_compact and
     'remove_minhook(' in hooks_compact
 ),
 'failed uninstall does not reopen':(
     'if(!uninstall_registry_hooks())returnfalse;' in compact and
     compact.index('if(!uninstall_registry_hooks())returnfalse;') <
     compact.index('clear_registry_policy_state_for_test();')
 ),
 'successful reset reopens':'void clear_state_impl() noexcept' in policy and 'g_pseudo_allocation_fenced=false' in policy[policy.index('void clear_state_impl() noexcept'):],
}
failed=[k for k,v in checks.items() if not v]
if failed:
 print('F1420_REGISTRY_TEARDOWN: FAIL')
 [print(' -',x) for x in failed]
 sys.exit(1)
print(f'F1420_REGISTRY_TEARDOWN: PASS ({len(checks)}/{len(checks)})')
