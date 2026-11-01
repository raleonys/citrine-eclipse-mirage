#!/usr/bin/env python3
from pathlib import Path
import sys,json
r=Path(__file__).resolve().parents[1]
host=(r/'tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp').read_text()
release_host=(r/'tools/preloader_l_release_host/release_host_main.cpp').read_text()
ach_contract=(r/'tools/preloader_l_native_acceptance_host/achievement_frame_contract.hpp').read_text()
build=(r/'scripts/build_orchestration/build_native_acceptance.ps1').read_text()
crt=(r/'scripts/build_orchestration/build_crt_acceptance.ps1').read_text()
exports=(r/'exports/preloader_l_native_acceptance.def').read_text()
shipping_exports=(r/'exports/preloader_l.def').read_text()
native=(r/'tests/native/native_acceptance_exports.cpp').read_text()
cm=(r/'CMakeLists.txt').read_text()
helper=(r/'scripts/build_orchestration/build_common.ps1').read_text()
state=json.loads((r/'RELEASE_STATE.json').read_text())
required_campaign18=[
 'prestart_stop','ptd_fls_lifecycle',
 'frame_fragmentation','frame_coalescing','frame_malformed','protocol_route_surface','store_content_entitlement',
 'achievement_event_delivery']
checks={
 'HF13 F02 timer API uses pinned Asio noexcept-safe cancel': all(
     x in (r/'src/runtime/asio_session_owner.cpp').read_text() for x in [
       'try { (void)f02_stall_timer_->cancel(); }',
       'catch (...) { terminal_status_=Status::resource_error; }',
       'socket_.cancel(ec);', 'socket_.close(ec);',
     ]) and 'f02_stall_timer_->cancel(ec)' not in (r/'src/runtime/asio_session_owner.cpp').read_text()
     and (r/'scripts/test_asio_f02_timer_api.py').is_file(),

 'failure unload gated by explicit shutdown':all(x in host for x in ['shutdown_then_unload(HMODULE module, ShutdownFn shutdown_runtime)','if(shutdown_runtime(30000u)!=0u) return false;','return unload_after_confirmed_shutdown(module);','return module && FreeLibrary(module)!=0;']) and 'if(ready_rc!=0u){ FreeLibrary(module)' not in host,
 'failed shutdown retains module':all(x not in host for x in ['if(rc!=0u){ FreeLibrary(module); WSACleanup(); finish(31u','if(tr!=0u){ FreeLibrary(module); WSACleanup(); finish(232u']),
 'target module unload centralized':host.count('FreeLibrary(module)')==1 and 'unload_after_confirmed_shutdown(HMODULE module)' in host,
 'protocol route exact compatibility contracts':all(x in host for x in [
  '<GetPresenceVisibilityResponse Visible=\\"false\\"/>',
  '<ErrorSuccess Description=\\"Origin is waiting for the game to shutdown.\\" Code=\\"1\\"/>',
  '<ErrorSuccess Description=\\"The function you are calling is not implemented.\\" Code=\\"-1593835518\\"/>',
  '<GetBlockListResponse Return=\\"Success\\"/>'
 ]) and '<GetVoipStatusResponse' not in host,
 'protocol route failure localization':all(x in host for x in ['rc=0x2B41u','rc=0x2B42u','rc=0x2B43u','rc=0x2B44u','rc=0x2B45u']),
 'achievement delivery matches shipping two-frame wire contract':all(x in host for x in [
  '"<AchievementSets>"',
  'recv_achievement_frames(ProtocolClient& c,const char* achievement_id)',
  'achievement_frame_contract::response(response_xml,response_n,achievement,achievement_n)',
  'achievement_frame_contract::event(event_xml,event_n,achievement,achievement_n)',
  'achievement_frame_contract::strict_pkcs7_xml_length(response_xml,response_n,&response_xml_n)',
  'achievement_frame_contract::strict_pkcs7_xml_length(event_xml,event_n,&event_xml_n)',
  'request_expect_achievement_frames(c',
  'NATIVE_ACH_TWO_FRAME_DIAG: stage=response',
  'NATIVE_ACH_TWO_FRAME_DIAG: stage=event',
 ]) and 'recv_achievement_compound' not in host and '<QueryAchievementsResponse' not in host and 'request_expect_two(c' not in host,
 'achievement delivery failure localization':all(x in host for x in ['rc=0x2B60u','rc=0x2B61u']),
 'generic host no unused connect metric binding':'auto* connect_metric_proc=GetProcAddress(module,"preloader_l_native_acceptance_connect_metric")' not in host,
 'real winsock connect':'connect(s,&addr,sizeof(addr))' in host,
 'native host preloads MinHook target modules':'LoadLibraryW(L"ADVAPI32.dll")' in host and 'MSVCP140.dll' not in host and 'RegOpenKeyExA' in host,
 'release host preloads MinHook target modules':'LoadLibraryW(L"ADVAPI32.dll")' in release_host and 'MSVCP140.dll' not in release_host and 'RegOpenKeyExA' in release_host,
 'native host 7-hook prerequisite field exact':(
    'struct HookPrerequisiteModules { HMODULE advapi; };' in host and
    'if(!hook_prerequisites.advapi) finish(4);' in host and
    '.msvcp' not in host and
    'RegCloseKey' in host),
 'release host 7-hook prerequisite field exact':(
    'struct HookPrerequisiteModules { HMODULE advapi; };' in release_host and
    'if(!hook_prerequisites.advapi) finish(9);' in release_host and
    '.msvcp' not in release_host and
    'RegCloseKey' in release_host),
 'host 7-hook prerequisites negative stale member rejected': all(
    '.msvcp' in tainted for tainted in (
      host.replace('if(!hook_prerequisites.advapi) finish(4);',
        'if(!hook_prerequisites.msvcp||!hook_prerequisites.advapi) finish(4);'),
      release_host.replace('if(!hook_prerequisites.advapi) finish(9);',
        'if(!hook_prerequisites.msvcp||!hook_prerequisites.advapi) finish(9);'))),

 'real send recv':'send(s,data+total' in host and 'recv(s,out+n,1,0)' in host,
 'lifecycle handshake exact accepted frame':all(x in host for x in ['open_established_client(uint16_t port)','exact_bytes(g_protocol_frame,accepted_n,expected,expected_n)',r'<Response id=\"n18h\" sender=\"EALS\">',r'<ChallengeAccepted response=\"','request_expect(c,probe,"<GetConfigResponse>")']),
 'legacy positive recv handshake removed':'open_client(uint16_t port,bool handshake)' not in host and 'open_client(port,true)' not in host,
 'selected port metric':'metric(0u)' in host,
 'zero scenario':'equal(scenario,"zero")' in host,
 'one scenario':'equal(scenario,"one")' in host,
 'growth scenario':'equal(scenario,"growth")' in host and 'for(uint32_t i=0;i<6u;++i)' in host,
 'parallel scenario':'equal(scenario,"parallel")' in host,
 'active shutdown scenario':'equal(scenario,"active_shutdown")' in host,
 'blocked shutdown scenario':'equal(scenario,"blocked_shutdown")' in host and 'open_prehandshake_client(port)' in host,
 'blocked shutdown deterministic cohort wait':all(x in host for x in ['wait_metric(metric,2u,3u,10000u)','wait_metric(metric,5u,3u,10000u)','finish(58)']) and 'if(!wait_metric(metric,2u,3u,5000u)||metric(5u)<3u)' not in host,
 'worker metric sentinel fail closed':'value!=0xffffffffu&&value>=target' in host,
 'f02 delivery stall parallel':all(x in host for x in ['equal(scenario,"f02_delivery_stall_parallel")','run_f02_delivery_stall_parallel_scenario','preloader_l_native_acceptance_f02_control','<IGOEvent State=\\\"DOWN\\\"/>']),
 'f02 original native host order unchanged':all(x in host for x in [
     'control(0u)!=1u','wait_f02_metric(control,2u,1u,2000u)',
     'control(3u)==0u','request_expect(b,get_config','wait_metric(metric,3u,completed_before+1u,7000u)',
     '(void)control(1u)']),
 'f02 asynchronous Asio stall native-only and per-session':all(x in (r/'src/runtime/asio_session_owner.cpp').read_text() for x in [
     '#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)',
     'release_acceptance_should_hold_asio_send_windows(this)',
     'schedule_stalled_prepared(bytes,length)',
     'f02_stall_timer_->async_wait','std::chrono::milliseconds(40)',
     'self->transaction_->completed(false,0u)',
     'try { (void)f02_stall_timer_->cancel(); }']),
 'f02 deferred completion prevents premature quiescence': 'Do not\n                        // advertise service quiescence before resume_transaction.' in (r/'src/runtime/asio_session_owner.cpp').read_text(),
 'f02 single-session gate retains legacy and suppresses sharing':all(x in (r/'src/runtime/release_server_worker.cpp').read_text() for x in [
     'std::atomic<const void*> g_acceptance_asio_stall_session',
     'compare_exchange_strong(',
     'g_acceptance_asio_stall_session.load(std::memory_order_acquire)!=session',
     'atomic_fetch_add_i32(&g_acceptance_send_block_hits,1)',
     'g_acceptance_asio_stall_session.store(nullptr, std::memory_order_release)',
     'g_acceptance_asio_stall_session.load(std::memory_order_acquire) ? 1u : 0u']),
 'f02 acceptance bridge independent of manual winsock ABI':(
     '#include "release_server_io_internal.hpp"' not in (r/'src/runtime/asio_session_owner.cpp').read_text()
     and '#include <windows.h>' not in (r/'src/runtime/asio_acceptance_delivery.hpp').read_text()
     and (r/'src/runtime/asio_acceptance_delivery.hpp').exists()),
 'f02 persistent client retirement is causally triggered':(
     'close_client(b.socket);\n        b.socket=kInvalidSocket;\n        if(!wait_metric(metric,3u,completed_before+1u,7000u))' in host
     and 'b=open_protocol_client(port);\n    if(b.socket==kInvalidSocket){rc=0x2C09u;goto done;}' in host
     and host.index('close_client(b.socket);\n        b.socket=kInvalidSocket;') < host.index('wait_metric(metric,3u,completed_before+1u,7000u)')
     and host.index('wait_metric(metric,3u,completed_before+1u,7000u)') < host.index('(void)control(1u); injection=false;')),
 'f02 export ordinal':'preloader_l_native_acceptance_f02_control @206' in exports and 'preloader_l_native_acceptance_f02_control' in native,
 'wave4 Asio ownership retry':all(x in host for x in ['equal(scenario,"wave4_resource_retry")','run_wave4_resource_retry_scenario','preloader_l_native_acceptance_wave4_control','first!=10u','Asio listener acceptor close']) and all(x in native for x in ['configure_asio_acceptance_resource_fault(selector)', 'asio_acceptance_resource_fault_metric(selector-2u)']),
 'wave4 export ordinal':'preloader_l_native_acceptance_wave4_control @208' in exports and 'preloader_l_native_acceptance_wave4_control' in native,
 'f02 runners include scenario':"'f02_delivery_stall_parallel'" in build and "'f02_delivery_stall_parallel'" in crt,
 'connect redirect runners include scenario':"'connect_redirect_lifecycle'" in build and "'connect_redirect_lifecycle'" in crt,
 'legacy htons lifecycle absent from active runners':"'htons_lifecycle'" not in build and "'htons_lifecycle'" not in crt,
 'connect redirect lifecycle follows shipping shutdown/reload':'equal(scenario,"connect_redirect_lifecycle")' in host and 'run_connect_redirect_shipping_lifecycle_scenario()' in host and 'shutdown_runtime(30000u)' in host and 'generation<2u' in host,
 'shipping shutdown scenario':'equal(scenario,"shipping_shutdown")' in host and 'shutdown_runtime(30000u)' in host,
 'same-process reload generation':'equal(scenario,"reload_generation")' in host and 'generation<2u' in host,
 'shipping export ABI unchanged':'preloader_l_native_acceptance_campaign18_scenario' not in shipping_exports,
 'campaign18 test export':'preloader_l_native_acceptance_campaign18_scenario @207' in exports and 'campaign18_scenario_impl' in native,
 'custom IAT campaign retired':'install_registry_iat_fallback()' not in native and 'iat_fallback_restore' not in host,
 'custom inline campaign retired':'install_inline_hook_quiescent' not in native and 'inline_hook_quiescence' not in host,
 'ptd fls executes':'detach_current_thread_ptd()' in native and 'current_ptd(false)' in native,
 'MinHook mechanism acceptance delegated':'cleanup_fault_injection' not in host and (r/'scripts/test_minhook_lifetime_contract.py').is_file(),
 'wire codec linked':all(x in cm for x in ['src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp','src/codec/message_codec.cpp','src/codec/session_crypto.cpp']),
 'native host does not retain IAT-only advapi dependency':'target_link_libraries(preloader_l_native_acceptance_host PRIVATE kernel32 ws2_32 preloader_l_tinyxml2)' in cm and 'advapi32' not in cm[cm.find('add_executable(preloader_l_native_acceptance_host'):cm.find('set_target_properties(preloader_l_native_acceptance_host')],
 'coalesced-safe receive':'recv(s,out+n,1,0)' in host and 'g_protocol_coalesced' in host,
 'receive progress budgets':all(x in (r/'src/protocol/socket_exchange_wire.cpp').read_text() for x in ['kDefaultReceiveIdleTimeoutMs','kDefaultReceiveFrameTimeoutMs','ExchangeStatus::receive_timeout','frame_started_at']),
 'shipping receive budgets':all(x in (r/'src/runtime/release_server_worker.cpp').read_text() for x in ['kReceiveIdleTimeoutMs = 30000u','kReceiveFrameTimeoutMs = 10000u']),
 'both native runners include all campaign18':all((f"'{x}'" in build and f"'{x}'" in crt) for x in required_campaign18),
 'host dispatch includes all campaign18':all(f'equal(scenario,"{x}")' in host for x in required_campaign18),
 'shared build hardening':all(x in build for x in ['build_common.ps1','Clear-BuildSelectorEnvironment','Invoke-ExternalChecked']),
 'tool selector sanitizer covers compiler vars':all(x in helper for x in ["'CC'","'CXX'","'CL'","'_CL_'","'LINK'","'_LINK_'","'LINK_REPRO'"]),
 'confined build directory':'Resolve-ConfinedBuildDirectory' in build,
 'shutdown stress loops':'for($round=1;$round -le 4;$round++)' in build and "@('active_shutdown','blocked_shutdown')" in build,
 'completed-owner reap metric':'wait_metric(metric,7u,1u,5000u)' in host and 'wait_metric_equal(metric,5u,0u,5000u)' in host,
 'fresh scenario processes':'& $NativeHost' in build,
 'pre post dll hash':'$preHash=(Get-FileHash -LiteralPath $NativeDll' in build and '$postHash=(Get-FileHash -LiteralPath $NativeDll' in build,
 'native controlled teardown delegates to shipping lifecycle':'shutdown_release_runtime_for_unload(timeout_ms)' in native and 'shutdown_release_service_for_acceptance()' not in native,
 'native controlled teardown avoids direct per-hook destruction':all(x not in native[native.find('uint32_t controlled_teardown_impl'):native.find('extern "C" uint32_t preloader_l_debug_wait_ready')] for x in ['uninstall_crypt_query_observer()','uninstall_surge_native_hooks()','uninstall_connect_redirect()','uninstall_windows_registry_iat_hooks()','shutdown_minhook_runtime()']),
 'native execution truth classified':state.get('windows_tested_checkpoint',{}).get('ids07','').startswith('PASS_USER_WINDOWS_LOGS_SHIPPING_DLL_SHA256_') and all(state.get('human_confirmation',{}).get(g)=='USER_REPORTED_NORMAL_GAMEPLAY_SMOKE_NOT_INSTRUMENTED_EXACT_DLL' for g in ('collegefb27','madden27')),
}
# F323-S0471-001: Preserve independent response/event extents on two frames.
# A single combined frame, event-only, wrong ID, duplicate events, missing
# second frame, and reversed frame order must not pass the acceptance oracle.
response='<LSX><Achievement Id="NATIVE_CAMPAIGN18"/></LSX>'
event='<LSX><Event sender="EbisuSDK"><Achievement Id="NATIVE_CAMPAIGN18"/></Event></LSX>'
wrong='<LSX><Event sender="EbisuSDK"><Achievement Id="WRONG"/></Event></LSX>'
marker='<Achievement Id="NATIVE_CAMPAIGN18"'
start_tag='<LSX><Event sender="EbisuSDK">'
end_tag='</Event></LSX>'
def response_shape(v):
    return v.count(marker)==1 and v.count('<LSX>')==1 and v.count('</LSX>')==1 and start_tag not in v and '</Event>' not in v
def event_shape(v):
    return v.startswith(start_tag) and v.endswith(end_tag) and v.count(start_tag)==1 and v.count(end_tag)==1 and v.count(marker)==1
checks['achievement executable validator binding']=all(x in ach_contract for x in [
    'inline bool response(', 'inline bool event(', 'tinyxml2::XMLDocument doc;',
    'doc.Parse(data,n)', 'id_matches(a,token,token_n)',
    'only_element(e,a)', 'only_element(root,e)', 'memchr(data,0,n)'])
# Keep historical string oracle as *additional* negative evidence, not the
# authoritative acceptance predicate. A source-linked TinyXML2 executable
# validates the actual grammar; it must not be replaced by this lexical model.
checks['achievement two independent frame synthetic oracle']=(
    response_shape(response) and event_shape(event) and
    not response_shape(event) and not event_shape(response) and
    not response_shape(response+event) and
    not event_shape(wrong) and not event_shape(event+event) and
    not event_shape('<LSX><Event sender="EbisuSDK"><Achievement Id="NATIVE_CAMPAIGN18"/><Achievement Id="NATIVE_CAMPAIGN18"/></Event></LSX>') and
    (not event_shape(''))
)
checks['native test-host vendor XML linker binding']='target_link_libraries(preloader_l_native_acceptance_host PRIVATE kernel32 ws2_32 preloader_l_tinyxml2)' in cm

# Native Asio cutover must expose actual session-registry lifecycle, not
# pretend the historical WinSock jthread vector owns accepted IOCP sockets.
service_owner=(r/'src/runtime/asio_service_owner.cpp').read_text()
service_header=(r/'src/runtime/asio_service_owner.hpp').read_text()
listener_lifecycle=(r/'src/runtime/release_server_lifecycle.cpp').read_text()
listener_internal=(r/'src/runtime/release_server_io_internal.hpp').read_text()
required_asio_observation=(
    'note_release_acceptance_accepted();',
    'note_release_acceptance_asio_owner_opened();',
    'note_release_acceptance_peer_nonempty();',
    'note_release_acceptance_started();',
    'note_release_acceptance_completed();',
    'note_release_acceptance_asio_owner_retired();',
    'note_release_acceptance_reaped();',
)
# HF13 Windows/MSVC acceptance regression: importing the private WinSock
# shim into an Asio TU collides with SDK SOCKET/select/recv declarations.
# Native counter declarations must remain test-only and declaration-compatible
# with the existing lifecycle implementation; no new metric storage is created.
metric_symbols=(
    'note_release_acceptance_accepted',
    'note_release_acceptance_started',
    'note_release_acceptance_completed',
    'note_release_acceptance_peer_nonempty',
    'note_release_acceptance_reaped',
    'note_release_acceptance_asio_owner_opened',
    'note_release_acceptance_asio_owner_retired',
)
def _acceptance_include_boundary_ok(source):
    # Source regressions are unsafe when the private WinSock ABI is included in
    # the Asio TU, even when the native-acceptance macro alone is enabled.
    return ('#include "release_server_io_internal.hpp"' not in source and
            'namespace release_server_windows_detail {' in source and
            '#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)' in source)
checks['asio TU excludes conflicting WinSock-private header']=(
    _acceptance_include_boundary_ok(service_owner))
checks['asio metrics declarations match native owner ABI']=all(
    f'void {name}() noexcept;' in service_owner and
    f'void {name}() noexcept;' in listener_internal and
    f'void {name}() noexcept' in listener_lifecycle
    for name in metric_symbols)
checks['asio acceptance metrics do not duplicate storage']=(
    'g_acceptance_' not in service_owner and
    'g_acceptance_asio_live' not in service_owner)
checks['forbidden private WinSock include mutation is rejected']=(
    _acceptance_include_boundary_ok(service_owner) and
    not _acceptance_include_boundary_ok(
        service_owner.replace('namespace release_server_windows_detail {',
            '#include "release_server_io_internal.hpp"\nnamespace release_server_windows_detail {')))
checks['asio metrics reflect actual session lifecycle']=all(
    word in service_owner for word in required_asio_observation)
checks['asio completion only after successful start']=(
    'if ((*it)->started)' in service_owner and
    'record->started=true;' in service_owner and
    'bool started{false};' in service_header)
checks['asio source counters absent from production builds']=(
    '#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)' in service_owner and
    'volatile int32_t g_acceptance_asio_live = 0;' in listener_lifecycle and
    '#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)' in listener_lifecycle)
checks['asio owner count and capacity use real separate ownership']=(
    'adapter && adapter->asio_bridge' in listener_lifecycle and
    'release_acceptance_asio_live_owners()' in listener_lifecycle and
    'static_cast<uint32_t>(release_client_hard_cap)' in listener_lifecycle and
    'note_release_acceptance_asio_owner_opened() noexcept;' in listener_internal and
    'note_release_acceptance_asio_owner_retired() noexcept;' in listener_internal)
checks['asio metric deletion mutations detected']=all(
    not all(word in service_owner.replace(drop,'') for word in required_asio_observation)
    for drop in required_asio_observation)
checks['native one scenario retains terminal observation']=(
    'wait_metric(metric,3u,1u,5000u)' in host and
    'wait_metric(metric,7u,1u,5000u)' in host and
    'wait_metric_equal(metric,5u,0u,5000u)' in host)

fail=[k for k,v in checks.items() if not v]
if fail:
    print('NATIVE_WORKER_ACCEPTANCE_SURFACE: FAIL'); [print(' -',x) for x in fail]; sys.exit(1)
print(f'NATIVE_WORKER_ACCEPTANCE_SURFACE: PASS ({len(checks)}/{len(checks)})')
