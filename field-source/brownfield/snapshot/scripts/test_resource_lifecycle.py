#!/usr/bin/env python3
from pathlib import Path
import json,sys
r=Path(__file__).resolve().parents[1]
def t(p): return (r/p).read_text(encoding='utf-8',errors='replace')
ptd=t('src/runtime/ptd_runtime.cpp')
ptdw=t('src/runtime/ptd_fls.cpp')
allocw=t('src/runtime/heap_allocator.cpp')
boot=t('src/runtime/bootstrap_thread.cpp')
boot_h=t('include/preloader_l/runtime/bootstrap_thread.hpp')
boot_state=t('src/runtime/bootstrap_state.cpp')
minhook=t('src/platform/minhook_runtime.cpp')
patch=t('scripts/build_orchestration/PatchMinHookSafeFreeze.cmake')
debug_crypt=t('src/debug/platform/crypt_observer_diagnostics.cpp')
config_storage=t('src/config/runtime_configuration_storage.cpp')
store_storage=t('src/config/store_materialization.cpp')
service_storage=t('src/runtime/service_state_storage.cpp')
event_queue=t('src/runtime/event_queue.cpp')
registry_state=t('src/platform/registry_policy_state.cpp')
svc=t('src/runtime/release_service_lifetime.cpp')
lifecycle=t('src/runtime/release_server_lifecycle.cpp')
acceptor=t('src/runtime/release_server_acceptor.cpp')
worker=lifecycle+acceptor
policy=t('src/runtime/service_policy.cpp')
derived=t('src/runtime/service_projection.cpp')
config=t('src/config/runtime_configuration.cpp')
store=t('src/config/store_runtime.cpp')+t('src/config/store_service_bridge.cpp')
reg=t('src/platform/registry_hooks.cpp')
native=t('tests/native/native_acceptance_exports.cpp')
host=t('tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp')
exports=t('exports/preloader_l_native_acceptance.def')
asio_bridge=t('src/runtime/asio_release_bridge.cpp')
asio_session=t('src/runtime/asio_session_owner.cpp')
asio_listener=t('src/runtime/asio_listener_owner.cpp')
asio_test=t('src/runtime/asio_acceptance_resource_fault.hpp')
native_runner=t('scripts/build_orchestration/build_native_acceptance.ps1')
crt_runner=t('scripts/build_orchestration/build_crt_acceptance.ps1')
quiesce_start=lifecycle.find('void quiesce_release_worker_policy_windows(')
quiesce_end=lifecycle.find('bool prepare_release_worker_policy_retirement_windows(',quiesce_start)
quiesce=lifecycle[quiesce_start:quiesce_end] if quiesce_start>=0 and quiesce_end>quiesce_start else ''
wait_start=acceptor.find('bool quiesce_all_client_threads(')
wait_end=acceptor.find('bool stop_all_client_threads(',wait_start)
wait_only=acceptor[wait_start:wait_end] if wait_start>=0 and wait_end>wait_start else ''
retire_start=lifecycle.find('bool prepare_release_worker_policy_retirement_windows(')
retire_end=lifecycle.find('void release_release_worker_policy_windows(',retire_start)
retire=lifecycle[retire_start:retire_end] if retire_start>=0 and retire_end>retire_start else ''
config_clear_start=config.find('bool clear_runtime_configuration_retryable() noexcept')
config_clear_end=config.find('void clear_runtime_configuration() noexcept',config_clear_start)
config_clear=config[config_clear_start:config_clear_end] if config_clear_start>=0 and config_clear_end>config_clear_start else ''
checks={
 'HF08 service physical owner slots':all(x in svc for x in ['g_pending_adapter_allocation','g_pending_control_allocation','retry_pending_service_physical_releases','release_or_retain_service_allocation']),
 'HF08 service shutdown waits physical owners':'reference_owner_release_alias(&g_published_owner);' in svc and 'if (!retry_pending_service_physical_releases())' in svc and 'ReleaseServiceShutdownResult::resource_cleanup_pending' in svc,
 'HF08 service retry blocks new generation':'acquire_release_service_once' in svc and 'return ReleaseServiceAcquireResult::owner_not_published;' in svc and 'if (!retry_pending_service_physical_releases())' in svc,
 'F06 socket close retained/retryable':'close_owned_socket_or_retain' in worker and 'retry_pending_owned_sockets' in worker,
 'F06 jthread handle retained':'if (!close_handle_owned(handle))' in worker and 'return false;' in worker,
 'F06 lifecycle preflight':'prepare_release_worker_policy_retirement_windows(service)' in svc,
 'F07 policy retryable':'release_model_service_policy_retryable' in policy and 'clear_runtime_configuration_retryable' in config,
 'HF09 config retired policy owner':all(x in config for x in ['g_retired_model_policy','g_retired_model_policy_pending','retry_retired_model_policy_cleanup','detach_live_model_policy_for_clear']),
 'HF09 config clear stages before mutation':config_clear.find('prepare_empty_retired_model_policy()') < config_clear.find('detach_live_model_policy_for_clear()') < config_clear.find('clear_behavioral_service_state()') if config_clear else False,
 'HF09 config clear one logical generation':config_clear.count('end_policy_write(true)')==2 and 'end_policy_write(false);' in config_clear and 'const bool cleanup_ok=retry_retired_model_policy_cleanup();' in config_clear,
 'HF09 config publish blocked by pending cleanup':('if(!g_model_service||g_retired_model_policy_pending)' in config or ('if (!g_model_service || g_retired_model_policy_pending ||' in config and '!retry_deferred_staged_policy_cleanup()' in config)) and 'if(g_retired_model_policy_pending) return RuntimeConfigurationStatus::resource_error;' in config,
 'Current deferred staged owner blocks publication until retry':('g_deferred_staged_policy_pending=true;' in config and 'if (!retry_deferred_staged_policy_cleanup())' in config and 'release_or_defer_staged_model_policy(&staged)' in config) if json.loads((r/'RELEASE_STATE.json').read_text()).get('hotfix_ordinal')>=19 else True,
 'F07 derived retryable':'release_model_service_derived_state_retryable' in derived and 'clear_runtime_store_retryable' in store,
 'F07 shutdown propagates':'ReleaseServiceShutdownResult::resource_cleanup_pending' in svc and 'release_model_service_policy_retryable(service)' in svc,
 'F08 PTD lock final retirement':'shutdown_ptd_platform_locks_windows()' in boot and 'DeleteCriticalSection' in ptdw,
 'F08 allocator lock final retirement':'shutdown_allocator_platform_lock_windows()' in boot and 'DeleteCriticalSection' in allocw,
 'F08 final-unload result':'platform_lock_cleanup_pending = 12' in boot_h,
 'F12 intrusive PTD tail':'PtdPrivateTail' in ptd and 'g_pending_ptd_head' in ptd and 'k_pending_ptd_capacity' not in ptd,
 'F12 infallible intrusive retention':'void retain_pending_ptd(void* ptd) noexcept' in ptd and '(void)retain_pending_ptd' not in ptd,
 'F13 registry rollback state':'rollback_incomplete' in reg and 'minhook_ticket_has_ownership' in reg,
 'W1 bounded teardown retained':'rollback_minhook_batch_transaction_with_timeout' in boot,
 'W2 send timeout retained':'kSendIdleTimeoutMs = 5000u' in t('src/runtime/release_server_worker.cpp'),
 'W3 connect redirect retained':'install_connect_redirect()' in boot and not (r/'src/platform/htons_redirect.cpp').exists(),
 'W4 native Asio fault control':'preloader_l_native_acceptance_wave4_control' in native and 'configure_asio_acceptance_resource_fault(selector)' in native and 'configure_release_acceptance_resource_fault_windows(selector)' not in native,
 'W4 native Asio socket+acceptor retry':'run_wave4_resource_retry_scenario' in host and 'first!=10u' in host and 'control(5u)!=0u' in host and 'Asio listener acceptor close' in host,
 'W4 Asio exact owned socket retained until close':all(x in asio_session for x in ['consume_asio_acceptance_socket_close_failure()', 'acceptance_socket_close_retained_', 'note_asio_acceptance_socket_close_retained()', 'note_asio_acceptance_socket_close_retired()', 'if (ec || socket_.is_open()) return;']),
 'W4 Asio listener no premature quiescence':all(x in asio_listener for x in ['consume_asio_acceptance_listener_close_failure()', 'acceptance_listener_close_retained_', 'note_asio_acceptance_listener_close_retained()', 'note_asio_acceptance_listener_close_retired()', 'if (close_error || acceptor_.is_open())']),
 'W4 no hidden stop retry success':all(x in asio_bridge for x in ['g_asio_fail_socket_close.load()!=0u', 'g_asio_fail_listener_close.load()!=0u', 'asio_acceptance_resource_fault_metric(3u)!=0u', 'return false;']),
 'W4 native-only, not shipping':all(x in asio_test for x in ['defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)']) and asio_bridge.count('#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)')>=2 and 'preloader_l_native_acceptance_wave4_control' not in asio_bridge, 
 'W4 native export ordinal':'preloader_l_native_acceptance_wave4_control @208' in exports,
 'W4 native+CRT runner ownership':"'wave4_resource_retry'" in native_runner and "'wave4_resource_retry'" in crt_runner,
 'HF06 outer failure publication':'mark_bootstrap_failed_if_unreported(result)' in boot and 'bool mark_bootstrap_failed_if_unreported' in boot_state,
 'HF06 stop-before-acquisition':'mark_bootstrap_cancelled_before_run()' in boot and boot.find('bootstrap_stop_requested()') < boot.find('const auto hook_install = install_post_loader_minhook_surface(selection);') and boot.count('bootstrap_stop_requested()') >= 4,
 'HF06 batch prevalidation before gate mutation':minhook.find('if (!validate_enable_batch(tickets, gates, count)) return false;') < minhook.find('prepare_gate(gates[i], tickets[i]);', minhook.find('bool enable_minhook_batch(')),
 'HF07 peer preparation explicit':'PEER_PREPARATION_RESULT' in patch and 'PEER_UNRESOLVED' in patch,
 'HF07 resume obligation retained':'g_retainedThreads' in patch and 'g_emergencyThread' in patch and 'ResumeOwnedThread' in patch and 'RetryRetainedThreadOwnership' in patch,
 'HF07 physical release checked':'if (CloseHandle(pItem->handle))' in patch and 'if (!HeapFree(g_hHeap, 0, pThreads->pItems))' in patch,
 'W6R1 quiesce uses wait-only client boundary':'quiesce_all_client_threads(service)' in quiesce and 'stop_all_client_threads(service)' not in quiesce,
 'W6R1 wait-only helper preserves physical ownership':'WaitForSingleObject(handle, kInfinite)' in wait_only and 'destroy_jthread(' not in wait_only and 'close_handle_owned(' not in wait_only,
 'W6R1 retryable preflight owns physical client retirement':'stop_all_client_threads(service)' in retire and 'retry_pending_owned_sockets()' in retire and 'destroy_jthread(&service->accept_thread)' in retire,
 'Audit3 S0152 loader-lock path retains pin':'if (FreeLibrary(pinned))' not in boot[boot.find('bool schedule_release_bootstrap('):boot.find('ReleaseBootstrapReadyResult wait_release_bootstrap_ready(')] and 'mark_bootstrap_failed(0x5403u);' in boot,
 'Audit3 S0158 readiness sees pre-worker failed phase':'snapshot_bootstrap().phase == BootstrapPhase::failed' in boot and 'ReleaseBootstrapReadyResult::failed' in boot,
 'Audit3 S0179 enumeration outcome truthful':all(x in patch for x in ['THREAD_ENUMERATION_API_FAILED','THREAD_ENUMERATION_ALLOC_FAILED','THREAD_ENUMERATION_OWNERSHIP_FAILED','return MH_UNKNOWN;']) and 'enumeration == THREAD_ENUMERATION_ALLOC_FAILED' in patch,
 'Audit3 S0206 debug protection obligation retained':all(x in debug_crypt for x in ['ObserverProtectionObligation','retry_observer_protection_restore()','g_observer_protection_obligation =','if (!retry_observer_protection_restore()) return false;']),
 'Audit3 S0277 config/store/service intrusive release owner':all('allocate_transient_bytes' in x and 'release_transient_bytes' in x and 'release_block(' not in x for x in [config_storage,store_storage,service_storage]),
 'Audit3 S0304 portable locks are atomic':event_queue.count('__atomic_exchange_n')>=3 and event_queue.count('__atomic_store_n')>=3 and service_storage.count('__atomic_exchange_n')>=2 and service_storage.count('__atomic_store_n')>=2 and registry_state.count('__atomic_exchange_n')>=1 and registry_state.count('__atomic_store_n')>=1,
}
failed=[]
for k,v in checks.items():
    print(('PASS' if v else 'FAIL')+': '+k)
    if not v: failed.append(k)
if failed:
    print(f'WAVE4_RESOURCE_LIFECYCLE_CONTRACT: FAIL ({len(checks)-len(failed)}/{len(checks)})')
    sys.exit(1)
print(f'WAVE4_RESOURCE_LIFECYCLE_CONTRACT: PASS ({len(checks)}/{len(checks)})')
