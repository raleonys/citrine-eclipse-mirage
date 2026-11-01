#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, tempfile, sys, re
r=Path(__file__).resolve().parents[1]
cxx=shutil.which('g++') or shutil.which('clang++')
if not cxx:
    print('BEHAVIORAL_REGRESSIONS: FAIL - no C++ compiler'); sys.exit(1)
common=[cxx,'-std=c++20','-Wall','-Wextra','-Werror','-Wno-error=volatile','-I',str(r/'third_party/pegtl/upstream/include'),'-I',str(r/'third_party/filesystem/upstream/include'),'-I',str(r/'include'),'-I',str(r/'tests'/'support'),'-I',str(r/'src'/'debug')]
runtime_config_sources=[
    r/'src/config/runtime_configuration.cpp',
    r/'src/config/runtime_configuration_parser.cpp',
    r/'src/config/runtime_configuration_storage.cpp',
    r/'src/config/config2_pegtl.cpp',
]
def close_config2_atom_units(units):
    # Dedicated owner of current Config2 syntax; many pre-split fixtures lack it.
    needed=any(Path(src).name in ('runtime_configuration_parser.cpp','service_state_parser.cpp') for src,_ in units)
    present=any(Path(src).name=='config2_pegtl.cpp' for src,_ in units)
    if needed and not present:
        units=[*units,(r/'src/config/config2_pegtl.cpp',
             ['-std=c++20','-I',str(r/'third_party/pegtl/upstream/include')])]
    return units

behavioral_state_sources=[
    r/'src/runtime/service_state.cpp',
    r/'src/runtime/service_state_storage.cpp',
    r/'src/runtime/service_state_parser.cpp',
    r/'src/runtime/service_state_grant.cpp',
]
behavioral_state_text='\n'.join(p.read_text(encoding='utf-8',errors='replace') for p in behavioral_state_sources)
value_codec_sources=[
    r/'src/xml/value_codec.cpp',
    r/'src/xml/value_codec_utf8.cpp',
    r/'src/xml/value_codec_normalize.cpp',
    r/'src/xml/value_codec_semantic.cpp',
]
request_sources=[
    r/'src/xml/request_parser.cpp',
    r/'third_party/tinyxml2/tinyxml2.cpp',
    r/'src/xml/request_children.cpp',
]
achievement_route_sources=[
    r/'src/handlers/achievement_routes.cpp',
    r/'src/handlers/achievement_grant_route.cpp',
    r/'src/handlers/achievement_query_route.cpp',
]
achievement_grant_text=(r/'src/handlers/achievement_grant_route.cpp').read_text(encoding='utf-8',errors='replace')
achievement_query_text=(r/'src/handlers/achievement_query_route.cpp').read_text(encoding='utf-8',errors='replace')
transient_owner_sources=[r/'src/runtime/memory_allocator.cpp',r/'src/runtime/ptd_runtime.cpp']

# Shard support: shared static/source assertions run in every shard; expensive
# compile+execute blocks are deterministically partitioned by stable block name.
_AUTHORITATIVE_SHARD_COUNT = 8
_shard_index = None
_shard_count = None
_has_shard_index = '--shard-index' in sys.argv
_has_shard_count = '--shard-count' in sys.argv
if not _has_shard_index and not _has_shard_count:
    print('BEHAVIORAL_REGRESSIONS: FAIL - full behavioral QA requires exactly 8 deterministic shards; invoke --shard-count 8 --shard-index 0..7')
    sys.exit(2)
if _has_shard_index != _has_shard_count:
    print('BEHAVIORAL_REGRESSIONS: FAIL - both --shard-index and --shard-count are required'); sys.exit(2)
try:
    _shard_index = int(sys.argv[sys.argv.index('--shard-index') + 1])
    _shard_count = int(sys.argv[sys.argv.index('--shard-count') + 1])
except (ValueError, IndexError):
    print('BEHAVIORAL_REGRESSIONS: FAIL - invalid shard arguments'); sys.exit(2)
if _shard_count != _AUTHORITATIVE_SHARD_COUNT:
    print(f'BEHAVIORAL_REGRESSIONS: FAIL - authoritative shard count is {_AUTHORITATIVE_SHARD_COUNT}, got {_shard_count}'); sys.exit(2)
if _shard_index < 0 or _shard_index >= _shard_count:
    print('BEHAVIORAL_REGRESSIONS: FAIL - shard range invalid'); sys.exit(2)

def _stable_bucket(name: str, count: int) -> int:
    # FNV-1a: deterministic across Python processes/platforms.
    h = 2166136261
    for b in name.encode('utf-8'):
        h ^= b
        h = (h * 16777619) & 0xffffffff
    return h % count

def run_heavy_case(name: str) -> bool:
    if _shard_count is None:
        return True
    return _stable_bucket(name, _shard_count) == _shard_index

# HOTFIX16/F1533-001: loader attach remains successful when worker dispatch
# fails, while each dispatch failure publishes a failed bootstrap phase.
secondary_win=(r/'src/runtime/secondary_integration_entry.cpp').read_text(encoding='utf-8',errors='replace')
bootstrap_win_h16=(r/'src/runtime/bootstrap_thread.cpp').read_text(encoding='utf-8',errors='replace')
required_h16_lifecycle=[
 'const bool scheduled = schedule_release_bootstrap(module);',
 'return true;',
 'mark_bootstrap_failed(0x5401u);',
 'mark_bootstrap_failed(0x5402u);',
 'mark_bootstrap_failed(0x5403u);',
]
combined_h16_lifecycle=secondary_win+'\n'+bootstrap_win_h16
if any(x not in combined_h16_lifecycle for x in required_h16_lifecycle):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HOTFIX16 bootstrap failure-state contract drift'); sys.exit(1)
if 'return scheduled;' in secondary_win:
    print('BEHAVIORAL_REGRESSIONS: FAIL - HOTFIX16 loader attach depends on worker dispatch'); sys.exit(1)

# HOTFIX16/F1566-001 + F1570-001: malformed-session abuse is bounded and
# all LSX transport boundary decisions use the quote-aware framing primitive.
framing_h16=(r/'src/xml/lsx_framing.cpp').read_text(encoding='utf-8',errors='replace')
socket_exchange_h16='\n'.join((
 (r/'src/protocol/socket_exchange.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/protocol/socket_exchange_wire.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/protocol/socket_exchange_frame_step.cpp').read_text(encoding='utf-8',errors='replace'),
))
required_h16_transport=[
 'enum class LexMode',
 'LexMode::comment',
 'LexMode::cdata',
 'LexMode::processing_instruction',
 'constexpr uint32_t kMalformedFrameLimit = 32u;',
 'bool tolerate_malformed_frame(uint32_t* streak) noexcept',
 'if (!streak || *streak >= kMalformedFrameLimit) return false;',
 'preloader_l::xml::scan_lsx_frame(xml, length)',
]
combined_h16_transport=framing_h16+'\n'+socket_exchange_h16
if any(x not in combined_h16_transport for x in required_h16_transport):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HOTFIX16 transport hardening markers missing'); sys.exit(1)
if 'first_lsx_end(' in socket_exchange_h16:
    print('BEHAVIORAL_REGRESSIONS: FAIL - HOTFIX16 duplicate LSX boundary scanner returned'); sys.exit(1)

# F1438-001 + HOTFIX03/F1568-001: GrantAchievement keeps all fallible
# render/allocation/watcher work before either immediate direct commit or the
# socket-delivery handoff. The socket path owns the deferred commit afterward.
routes=achievement_grant_text
start=routes.find('HandlerStatus handle_grant_achievement_impl(')
end=routes.find('HandlerStatus handle_grant_achievement(',start)
body=routes[start:end] if start>=0 and end>start else ''
required=['prepare_grant_achievement_locked','format_grant_achievement_record(r,rec,out,cap,written)',
          'prepare_achievement_event','publish_achievement_watcher_snapshot','if(deferred){',
          'deferred->transaction=mutation.detach()','deferred->event=prepared_event.detach()',
          'mutation.commit()','prepared_event.commit()']
if not body or any(x not in body for x in required):
    print('BEHAVIORAL_REGRESSIONS: FAIL - GrantAchievement transaction markers missing'); sys.exit(1)
pos=[body.find(x) for x in required]
if pos!=sorted(pos) or 'publish_achievement_event(' in body:
    print('BEHAVIORAL_REGRESSIONS: FAIL - GrantAchievement transaction ordering drift'); sys.exit(1)

# HF16: achievement route decomposition is a permanent structural boundary.
achievement_facade=(r/'src/handlers/achievement_routes.cpp').read_text(encoding='utf-8',errors='replace')
achievement_internal=(r/'src/handlers/achievement_routes_internal.hpp').read_text(encoding='utf-8',errors='replace')
required_hf16_files=[
    r/'src/handlers/achievement_routes.cpp',
    r/'src/handlers/achievement_grant_route.cpp',
    r/'src/handlers/achievement_query_route.cpp',
    r/'src/handlers/achievement_routes_internal.hpp',
]
if any(not p.is_file() for p in required_hf16_files):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF16 achievement route split incomplete'); sys.exit(1)
for forbidden in ['prepare_grant_achievement_locked(', 'publish_achievement_watcher_snapshot(', 'next_direct_child(r.command_content']:
    if forbidden in achievement_facade:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF16 achievement implementation leaked into facade'); sys.exit(1)
for required in ['detail::handle_grant_achievement(', 'detail::handle_query_achievements(']:
    if required not in achievement_facade:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF16 achievement facade forwarding drift'); sys.exit(1)
if 'handle_grant_achievement(' not in achievement_internal or 'handle_query_achievements(' not in achievement_internal:
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF16 achievement private contract drift'); sys.exit(1)

# Deferred-delivery transaction boundary: GrantAchievement and ShowIGO use deferred
# delivery objects and commit only after successful send_all(). Any
# send/encode/resource failure is handled by RAII rollback.
socket_exchange=(r/'src/protocol/socket_exchange.cpp').read_text(encoding='utf-8',errors='replace')
socket_transaction=(r/'src/protocol/socket_exchange_transaction.cpp').read_text(encoding='utf-8',errors='replace')
# Production transaction-owner structures now live in an internal header;
# the send-completion decisions are a distinct real production TU.
transaction_owner=(r/'src/protocol/socket_exchange_transaction_owner_private.hpp').read_text(encoding='utf-8',errors='replace')
transaction_delivery=(r/'src/protocol/socket_exchange_transaction_delivery.cpp').read_text(encoding='utf-8',errors='replace')
required_owner=[
 'struct GrantSocketDelivery {',
 '~GrantSocketDelivery() noexcept { rollback(); }',
 'struct QueuedEventSocketDelivery {',
 '~QueuedEventSocketDelivery() noexcept { rollback(); }',
 'prepare_grant_achievement_delivery(',
 'begin_queued_event_delivery(',
 'commit_grant_achievement_delivery_store(',
 'commit_queued_event_delivery(',
 'finish_grant_achievement_delivery(',
 'resolve_queued_event_delivery_for_retry(&queue)',
]
if any(x not in transaction_owner for x in required_owner):
    print('BEHAVIORAL_REGRESSIONS: FAIL - transaction owner/rollback contract drift'); sys.exit(1)
if ('prepare_show_igo_delivery(' not in socket_transaction or
    'DispatchTransactionResult execute_response_transaction(' not in socket_transaction or
    socket_transaction.count('measured.route==preloader_l::router::RouteId::grant_achievement')!=1):
    print('BEHAVIORAL_REGRESSIONS: FAIL - transaction dispatcher ownership drift'); sys.exit(1)
required_completion=[
 'grant_delivery.commit_response();',
 'grant_delivery.commit_event();',
 'show_igo_delivery.commit();',
 'grant_delivery.retry_event();',
 'generic_event_delivery.retry();',
 'complete_delivery_write(',
]
if any(x not in transaction_delivery for x in required_completion):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF10 B resolution/commit markers drift'); sys.exit(1)
if (transaction_delivery.count('grant_delivery.commit_response();')!=1 or
    transaction_delivery.count('grant_delivery.commit_event();')!=1 or
    transaction_delivery.count('show_igo_delivery.commit();')!=1):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF10 B commit cardinality drift'); sys.exit(1)
if socket_exchange.count('execute_response_transaction(') != 2:
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF03 exchange paths do not share transaction primitive'); sys.exit(1)
# The encoded Response+Event preflight occurs before returning the prepared
# transaction; commit is only possible after complete_delivery_write confirms
# a full response, then the event write completes (shipping source-linked QA).
preflight_pos=socket_transaction.find('if(event_plain_n!=0u){\n        size_t event_encoded=0u;')
writer_pos=socket_transaction.find('if (!begin_delivery_writes(&transaction->cursor,response_required,event_required))')
if not (0 <= preflight_pos < writer_pos < socket_transaction.find('return true;',writer_pos)):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF10 two-frame preflight drift'); sys.exit(1)
response_complete=transaction_delivery.find('if (was_response) {')
response_commit=transaction_delivery.find('transaction->grant_delivery.commit_response();',response_complete)
event_start=transaction_delivery.find('if (decision==DeliveryWriteDecision::event_needed)',response_complete)
event_commit=transaction_delivery.find('transaction->grant_delivery.commit_event();',event_start)
show_commit=transaction_delivery.find('transaction->show_igo_delivery.commit();',event_start)
if not (0 <= response_complete < response_commit < event_start < event_commit < show_commit):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF10 completion/commit ordering drift'); sys.exit(1)
# The HF08 standalone alias owns ID 15 and must follow the same prepared
# event transaction as the primary ShowIGO ID 14. Match identifier boundaries.
if (len(re.findall(r'measured\.route==preloader_l::router::RouteId::show_igo(?![\w])',socket_transaction))!=1 or
    len(re.findall(r'measured\.route==preloader_l::router::RouteId::show_igo_window(?![\w])',socket_transaction))!=1):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF08 ShowIGO/ShowIGOWindow prepared transaction branch drift'); sys.exit(1)

# F1438-002/Phase1446 boundary: production currently has no watcher binder.
# If one appears, re-entry/concurrency must be audited before aggregate QA can pass.
binder_sites=[]
for src in (r/'src').rglob('*.cpp'):
    text_src=src.read_text(encoding='utf-8',errors='replace')
    if 'bind_achievement_watcher_sink(' in text_src and src.name!='achievement_watcher.cpp':
        binder_sites.append(str(src.relative_to(r)))
if binder_sites:
    print('BEHAVIORAL_REGRESSIONS: FAIL - production achievement watcher binder requires re-entry audit:', ', '.join(binder_sites)); sys.exit(1)

# MinHook architecture reset: custom relocation/patch/IAT/VEH machinery is
# intentionally retired. Preserve only the project-owned MinHook lifetime
# contract and require all nine shipping detours to use its guarded forwarding
# authority. Low-level relocation/protection correctness belongs to pinned MinHook.
minhook_runtime=(r/'src/platform/minhook_runtime.cpp').read_text(encoding='utf-8',errors='replace')
minhook_header=(r/'include/preloader_l/platform/minhook_runtime.hpp').read_text(encoding='utf-8',errors='replace')
required_mh=[
 'MH_CreateHook(', 'MH_QueueEnableHook(', 'MH_ApplyQueued()', 'MH_QueueDisableHook(', 'MH_RemoveHook(',
 'kGateInstalling', 'kGateActive', 'kGateDraining', 'kGateDirect',
 'atomic_store_pointer(&gate->forwarding_target', 'wait_gate(gates[i])',
 'restore_disabled_after_failed_enable',
]
if any(x not in minhook_runtime for x in required_mh):
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - MinHook lifetime contract drift'); sys.exit(1)
legacy_hook_files=[
 'src/platform/inline_hook_backend_windows.cpp','src/platform/inline_hook_allocator_windows.cpp',
 'src/platform/inline_hook_page_trap_windows.cpp','src/platform/inline_hook_patch_windows.cpp',
 'src/platform/inline_hook_quiescence.cpp','src/platform/x64_trampoline_relocator.cpp',
 'src/platform/iat_hook_backend_windows.cpp','src/platform/surge_native_hooks_iat_windows.cpp',
 'src/platform/crypt_query_tail_wrapper_x64.asm',
]
if any((r/x).exists() for x in legacy_hook_files):
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - retired custom hook engine returned'); sys.exit(1)
registry_mh=(r/'src/platform/registry_wrappers.cpp').read_text(encoding='utf-8',errors='replace')
crypt_mh=(r/'src/platform/crypt_observer.cpp').read_text(encoding='utf-8',errors='replace')
connect_mh=(r/'src/platform/connect_redirect_hook.cpp').read_text(encoding='utf-8',errors='replace')
if registry_mh.count('MinHookCallGuard quiescence(') != 5 or registry_mh.count('minhook_gate_forwarding_target(') != 5:
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - registry MinHook guard/forwarding drift'); sys.exit(1)
for name,body in [('Crypt',crypt_mh),('connect redirect',connect_mh)]:
    if 'MinHookCallGuard' not in body or 'minhook_gate_forwarding_target(' not in body:
        print('BEHAVIORAL_REGRESSIONS_MH: FAIL - '+name+' MinHook guard/forwarding drift'); sys.exit(1)

# HF21 ownership boundaries outside the retired hook engine remain enforced.
release_internal_hf21=(r/'src/runtime/release_server_io_internal.hpp').read_text(encoding='utf-8',errors='replace')
behavioral_internal_hf21=(r/'src/runtime/service_state_internal.hpp').read_text(encoding='utf-8',errors='replace')
host_internal_hf21=(r/'src/platform/registry_hook_runtime_internal.hpp').read_text(encoding='utf-8',errors='replace')
host_wrappers_hf21=(r/'src/platform/registry_wrappers.cpp').read_text(encoding='utf-8',errors='replace')
if any(token in release_internal_hf21 for token in ['g_stop_requested','g_acceptance_accepted','g_acceptance_started','g_acceptance_completed','g_acceptance_peer_nonempty','g_acceptance_reaped']):
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - HF21 release server leaks raw lifecycle storage'); sys.exit(1)
if re.search(r'\bactive_store\s*\(', behavioral_internal_hf21):
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - HF21 broad behavioral Store accessor returned'); sys.exit(1)
if 'host_bridge_state()' in host_internal_hf21 or 'host_bridge_state()' in host_wrappers_hf21:
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - HF21 broad HostBridgeState accessor returned'); sys.exit(1)

# F1418-002: manual-offline must have one runtime policy. Config2
# EALaunchOfflineMode remains a host environment name/compatibility parse key only;
# LSX GetSetting/GetSettings derive manual-offline exclusively from Online.
runtime_provider=(r/'src/config/runtime_provider.cpp').read_text(encoding='utf-8',errors='replace')
selector_pipeline=(r/'src/handlers/selector_pipeline.cpp').read_text(encoding='utf-8',errors='replace')
if 'case SettingKey::launch_offline_mode: return nullptr;' not in runtime_provider:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-002 runtime provider exposes duplicate manual-offline policy'); sys.exit(1)
required_manual=[
 'BooleanPolicy manual_offline_policy() noexcept',
 'SettingKey::online, "true", &used_default',
 'return {!preloader_l::config::online_enabled(online), used_default, true};',
 'const auto policy = manual_offline_policy();',
 'const auto manual = manual_offline_policy();',
]
if any(x not in selector_pipeline for x in required_manual):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-002 sole-Online selector policy drift'); sys.exit(1)
if 'SettingKey::launch_offline_mode, true' in selector_pipeline:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-002 compatibility key still overrides Online'); sys.exit(1)

# F1418-003: process -emuGameLang precedence must be reverse-published into
# the shared SettingKey::language policy before LSX/environment consumers.
win_config=(r/'src/config/process_configuration.cpp').read_text(encoding='utf-8',errors='replace')
required_language_provider=[
 'ValueView online; ValueView language;',
 'if(key==SettingKey::language && g.language.present){*out=g.language;return true;}',
 'void set_runtime_language_override(ValueView v) noexcept{g.language=v;}',
 'g.language=missing_value();',
]
if any(x not in runtime_provider for x in required_language_provider):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-003 shared language provider publication drift'); sys.exit(1)
required_language_sync=[
 'void synchronize_process_language_policy() noexcept',
 'query_process_emu_game_language(&value,&length)',
 'set_runtime_language_override(ValueView{value,length,true});',
 'set_runtime_language_override(missing_value());',
 'result.file_status = load_module_relative_configuration();',
 'synchronize_process_language_policy();',
]
if any(x not in win_config for x in required_language_sync):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-003 process language synchronization drift'); sys.exit(1)
load_pos=win_config.find('result.file_status = load_module_relative_configuration();')
sync_pos=win_config.find('synchronize_process_language_policy();',load_pos)
return_pos=win_config.find('return result;',sync_pos)
if min(load_pos,sync_pos,return_pos)<0 or not (load_pos < sync_pos < return_pos):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-003 bootstrap publication ordering drift'); sys.exit(1)
if 'query_process_emu_game_language' in selector_pipeline:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-003 selector retains split CLI language policy'); sys.exit(1)
if 'SettingKey::language, "en_US", used_fallback' not in selector_pipeline:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-003 installed language is not shared-provider backed'); sys.exit(1)
environment_policy=(r/'src/platform/environment_policy.cpp').read_text(encoding='utf-8',errors='replace')
if 'SettingKey::language, &value' not in environment_policy:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1418-003 EAGameLocale bypasses shared language policy'); sys.exit(1)

# F1419-001: DLL-relative config/avatar paths must acquire the module path as
# UTF-16. Config opens the explicit sibling with CreateFileW; QueryImage's
# fallback materializes that same sibling policy as semantic UTF-8.
config_windows=(r/'src/config/process_configuration.cpp').read_text(encoding='utf-8',errors='replace')
config_file=(r/'src/config/runtime_configuration_file.cpp').read_text(encoding='utf-8',errors='replace')
avatar_path=(r/'src/runtime/avatar_path.cpp').read_text(encoding='utf-8',errors='replace')
required_unicode_paths=[
 ('config module path', 'GetModuleFileNameW' in config_windows and 'GetModuleFileNameA' not in config_windows),
 ('config wide file boundary', 'load_runtime_configuration_pair_wide(path,override_path)' in config_windows and 'CreateFileW' in config_file),
 ('avatar module path', 'GetModuleFileNameW' in avatar_path and 'GetModuleFileNameA' not in avatar_path),
 ('avatar UTF-8 boundary', 'WideCharToMultiByte' in avatar_path and 'kCpUtf8=65001u' in avatar_path and 'kWcErrInvalidChars=0x80u' in avatar_path),
]
for label,ok in required_unicode_paths:
    if not ok:
        print(f'BEHAVIORAL_REGRESSIONS: FAIL - F1419-001 {label} drift'); sys.exit(1)

# F1417-001: flat runtime settings must only be published from the curio
# top-level section that owns each shipping policy. Root-level leaves and
# unknown sections remain parse-compatible but non-authoritative.
config_source='\n'.join(p.read_text(encoding='utf-8',errors='replace') for p in runtime_config_sources)
required_config_policy=[
 'enum class SectionPolicy : uint8_t { none=0, game, emulator, user, achievements };',
 'SectionPolicy reserved_key_policy(const Cursor& c,Token key) noexcept',
 'if(actual==SectionPolicy::none) return true;',
 'if(reserved!=SectionPolicy::none&&reserved!=actual) return true;',
]
if any(x not in config_source for x in required_config_policy):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1417-001 section policy isolation drift'); sys.exit(1)

# F1417-002/F1417-003: only direct leaves under a curio top-level section
# may publish flat runtime settings, and unterminated block comments are lexical
# errors rather than implicit EOF.
parser_peg=(r/'src/config/config2_pegtl.cpp').read_text(encoding='utf-8',errors='replace')
required_config_parser_hardening=[
 'if(depth==1u)return put_normalized_leaf_setting(*x.staged,x.cursor,path[0],key,value);',
 'const auto parsed=config2_pegtl::parse(data,length,visitor);',
 'if(parsed!=config2_pegtl::Result::ok)',
]
required_config_peg_hardening=[
 'struct BlockComment : p::seq<',
 'struct Document : p::seq<',
 'struct BehavioralDocument : p::seq<',
 'p::opt<UnclosedBlockTail>',
 'const bool matched=service_legacy_trailing_unclosed_comment',
 'if(!matched) return tracker.failure;',
]
if (any(x not in config_source for x in required_config_parser_hardening) or
    any(x not in parser_peg for x in required_config_peg_hardening)):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1417-002/F1417-003 PEGTL parser hardening drift'); sys.exit(1)

# F1416-002: failed FlsFree must retain the live FLS index and platform
# policy. Initialization rollback must not turn that retained state into a
# false-success initialized runtime; retry cleanup precedes a fresh allocation.
ptd=(r/'src/runtime/ptd_runtime.cpp').read_text(encoding='utf-8',errors='replace')
required_ptd=[
 'bool g_ptd_runtime_initialized = false;',
 'if (g_ptd_runtime_initialized) return true;',
 'if (!g_ops.fls_free(g_ops.context, g_fls_index)) return false;',
 'if(g_ops.fls_free(g_ops.context,index))',
 'if(!g_ops.fls_free(g_ops.context,index)) return;',
 'if(!retry_pending_ptds()) return;',
 'g_ptd_runtime_initialized=true;',
 'g_ptd_runtime_initialized=false;',
]
if any(x not in ptd for x in required_ptd):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1416-002 retryable FlsFree state drift'); sys.exit(1)
shutdown_pos=ptd.find('void shutdown_ptd_runtime() noexcept')
shutdown_end=ptd.find('void* current_ptd',shutdown_pos)
shutdown_ptd=ptd[shutdown_pos:shutdown_end] if shutdown_pos>=0 and shutdown_end>shutdown_pos else ''
if not shutdown_ptd or shutdown_ptd.find('if(!g_ops.fls_free(g_ops.context,index)) return;') > shutdown_ptd.find('g_fls_index=ptd_fls_out_of_indexes;'):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1416-002 clears PTD state before successful FlsFree'); sys.exit(1)

# HF17 lifecycle integration: CWD mirror is retired by configuration clear and
# queued events are retired at the quiescent service-owner shutdown boundary.
runtime_config=(r/'src/config/runtime_configuration.cpp').read_text(encoding='utf-8',errors='replace')
service_lifetime=(r/'src/runtime/release_service_lifetime.cpp').read_text(encoding='utf-8',errors='replace')
memory_allocator_text=(r/'src/runtime/memory_allocator.cpp').read_text(encoding='utf-8',errors='replace')
response_support_text=(r/'src/handlers/route_response_support.hpp').read_text(encoding='utf-8',errors='replace')
socket_txn_text=(r/'src/protocol/socket_exchange_transaction.cpp').read_text(encoding='utf-8',errors='replace')
semantic_value_text=(r/'src/xml/value_codec_semantic.cpp').read_text(encoding='utf-8',errors='replace')
hf13_required=[
    ('intrusive transient owner','retain_transient_owner(header)' in memory_allocator_text and 'header->next = g_transient_owner_head' in memory_allocator_text),
    ('route response transient release','release_transient_bytes(data)' in response_support_text),
    ('socket transaction transient release','release_transient_bytes(data)' in transaction_owner and 'release_transient_bytes(transaction)' in transaction_delivery),
    ('semantic value transient release','release_transient_bytes(owned_)' in semantic_value_text),
    ('service lifecycle transient retry','retry_pending_transient_releases()' in service_lifetime),
]
if any(not ok for _,ok in hf13_required):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF13 transient owner contract drift: '+','.join(name for name,ok in hf13_required if not ok)); sys.exit(1)
if 'transient_free(g_cwd,g_cwd_capacity);' not in runtime_config or 'g_cwd=nullptr; g_cwd_len=0u; g_cwd_capacity=0u;' not in runtime_config:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1476 current-directory mirror teardown drift'); sys.exit(1)
service_start=service_lifetime.find('ReleaseServiceShutdownResult shutdown_published_owner_if_idle()')
service_end=service_lifetime.find('namespace preloader_l { namespace runtime {',service_start)
service_body=service_lifetime[service_start:service_end] if service_start>=0 and service_end>service_start else ''
required_service_teardown=['if (!preloader_l::runtime::clear_queued_events())','clear_runtime_configuration_retryable()','prepare_release_worker_policy_retirement_windows(service)','preloader_l::runtime::reference_owner_release_alias(&g_published_owner);']
if not service_body or any(x not in service_body for x in required_service_teardown):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1519 service event-queue teardown marker missing'); sys.exit(1)
positions=[service_body.find(x) for x in required_service_teardown]
if positions != sorted(positions):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1519 queue cleanup/owner-release ordering drift'); sys.exit(1)

builds=[
 ('storage',[r/'tests/unit/test_service_storage.cpp',r/'src/runtime/service_storage.cpp',r/'src/runtime/reference_owner.cpp'],[]),
 ('live_storage',[r/'tests/unit/test_service_ownership.cpp',r/'src/runtime/service_storage.cpp',r/'src/runtime/reference_owner.cpp'],[]),
 ('owner',[r/'tests/unit/test_reference_owner.cpp',r/'src/runtime/reference_owner.cpp'],[]),
 ('strict_status',[r/'tests/unit/test_consistency_baseline.cpp'],[]),
 ('allocator_release',[r/'tests/unit/test_allocator_release_block.cpp',r/'src/runtime/memory_allocator.cpp',r/'src/runtime/ptd_runtime.cpp'],[]),
 ('transient_allocation_owner',[r/'tests/unit/test_transient_allocation_owner.cpp',r/'src/runtime/memory_allocator.cpp',r/'src/runtime/ptd_runtime.cpp'],[]),
 ('semantic_value_release_retry',[r/'tests/unit/test_semantic_value_release_retry.cpp',r/'src/xml/value_codec.cpp',r/'src/xml/value_codec_utf8.cpp',r/'src/xml/value_codec_normalize.cpp',r/'src/xml/value_codec_semantic.cpp',r/'src/runtime/memory_allocator.cpp',r/'src/runtime/ptd_runtime.cpp'],['-D_WIN32','-D__stdcall=','-D__declspec(x)=']),
 ('route_transient_owner',[r/'tests/unit/test_route_transient_owner.cpp',r/'src/runtime/memory_allocator.cpp',r/'src/runtime/ptd_runtime.cpp'],['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']),
 ('socket_failure_policy',[r/'tests/unit/test_socket_failure_policy.cpp',r/'src/net/socket_runtime.cpp'],[]),
 ('bootstrap_shutdown_policy',[r/'tests/unit/test_bootstrap_shutdown_policy.cpp',r/'src/runtime/bootstrap_runner.cpp',r/'src/runtime/bootstrap_state.cpp'],[]),
 ('bootstrap_state_outer_boundary',[r/'tests/unit/test_release_bootstrap_state.cpp',r/'src/runtime/bootstrap_state.cpp'],[]),
 ('bootstrap_state_concurrency',[r/'tests/unit/test_bootstrap_state_concurrency.cpp',r/'src/runtime/bootstrap_state.cpp'],['-pthread']),
 ('ptd_flsfree_failure',[r/'tests/unit/test_ptd_fls_free.cpp',r/'src/runtime/ptd_runtime.cpp'],[]),
 ('ptd_intrusive_pending',[r/'tests/unit/test_ptd_intrusive_pending.cpp',r/'src/runtime/ptd_runtime.cpp'],[]),
 ('config_section_policy',[r/'tests/unit/test_config_section_policy.cpp']+runtime_config_sources+[r/'src/config/settings_values.cpp']+behavioral_state_sources,['-Wno-unused-function']),
 ('config_parser_hardening',[r/'tests/unit/test_config_parser_hardening.cpp']+runtime_config_sources+[r/'src/config/settings_values.cpp']+behavioral_state_sources,['-Wno-unused-function']),
 ('bootstrap_failure_unwind',[r/'tests/unit/test_bootstrap_failure_unwind.cpp',r/'src/platform/environment_policy.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp'],[]),
 ('manual_offline_policy',[r/'tests/unit/test_manual_offline_policy.cpp']+runtime_config_sources+[r/'src/config/runtime_provider.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',r/'src/handlers/selector_pipeline.cpp',*request_sources,r/'src/xml/lsx_framing.cpp']+behavioral_state_sources,['-Wno-unused-function']),
 # The following two tests exercise actual Windows SDK pathname semantics and
 # Win32 process argument APIs. Do not run them using POSIX _WIN32/ghc path
 # emulation: native MSVC acceptance is mandatory, recorded separately.
 # Native owners: tests/unit/test_language_policy.cpp and
 # tests/unit/test_unicode_sibling_paths.cpp. Windows receipt gate: IDS08.
 ('worker_shutdown_liveness',[r/'tests/unit/test_worker_shutdown_liveness.cpp',r/'src/runtime/release_server_worker.cpp'],['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-DPRELOADER_L_TRACE_DIAGNOSTICS=0','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']),
 ('shipping_shutdown_contract',[r/'tests/unit/test_shipping_shutdown_contract.cpp'],[]),
 ('grant_txn',[r/'tests/unit/test_grant_achievement_transaction.cpp']+behavioral_state_sources,['-Wno-unused-function']),
 ('prepared_event',[r/'tests/unit/test_prepared_event_queue.cpp',r/'src/runtime/event_queue.cpp'],['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-misleading-indentation']),
]
if run_heavy_case('preloader_l_qa'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_qa_') as td:
        td=Path(td)
        for name,srcs,extra in builds:
            exe=td/name
            # Current service_state_parser.cpp delegates to PEGTL; fixtures
            # must link the actual production parser owner, never a fake stub.
            if r/'src/runtime/service_state_parser.cpp' in srcs and r/'src/config/config2_pegtl.cpp' not in srcs:
                srcs=[*srcs,r/'src/config/config2_pegtl.cpp']
            subprocess.run(common+extra+[str(x) for x in srcs]+['-o',str(exe)],cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)

# AUDIT-HOTFIX07 QA truthfulness: historically stale/non-executed unit fixtures
# must compile and execute against the current policy model. Exact literal
# "false" is the offline policy; compatibility string "0" remains online.
if run_heavy_case('preloader_l_stale_qa'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_stale_qa_') as td:
        td=Path(td)
        def run_case(name, srcs, extra=None):
            exe=td/name
            subprocess.run(common+(extra or [])+[str(x) for x in srcs]+['-o',str(exe)],cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)

        run_case('environment_policy_current',[
            r/'tests/unit/test_environment_policy.cpp',r/'src/platform/environment_policy.cpp',
            r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp'])

        # Rust-derived numeric OriginError taxonomy is an
        # executable contract, not a documentation-only convenience.
        run_case('hf02_origin_error',[r/'tests/unit/test_origin_error.cpp'])

        selector_common=runtime_config_sources+[
            r/'src/config/runtime_provider.cpp',
            r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',r/'src/handlers/selector_pipeline.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',
            *value_codec_sources,r/'src/xml/attribute_parser.cpp']+behavioral_state_sources
        run_case('selector_pipeline_current',[r/'tests/unit/test_selector_pipeline.cpp']+selector_common,['-Wno-unused-function'])

        route_common=runtime_config_sources+[
            r/'src/config/runtime_provider.cpp',
            r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',
            *value_codec_sources,r/'src/xml/attribute_parser.cpp',r/'src/router/route_inventory.cpp',
            r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp']+behavioral_state_sources
        route_extra=['-Wno-unused-function','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']
        run_case('query_friends_current',[r/'tests/unit/test_query_friends.cpp',r/'src/handlers/friends_route.cpp']+route_common,route_extra)
        run_case('check_permission_current',[r/'tests/unit/test_check_permission.cpp',r/'src/handlers/permission_route.cpp']+route_common,route_extra)
        run_case('start_download_current',[r/'tests/unit/test_start_download.cpp',r/'src/handlers/integration_routes.cpp',r/'src/runtime/event_queue.cpp']+route_common,route_extra)
        run_case('set_presence_current',[r/'tests/unit/test_set_presence.cpp',r/'src/handlers/integration_routes.cpp',r/'src/runtime/event_queue.cpp']+route_common,route_extra)

        # ShowIGO additionally publishes an event. Supply portable Win32 heap stubs
        # from the test TU and execute the actual event queue path.
        show_extra=['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function',
                    '-Wno-misleading-indentation','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']
        run_case('show_igo_current',[
            r/'tests/unit/test_show_igo.cpp',r/'src/handlers/integration_routes.cpp',r/'src/runtime/event_queue.cpp',
            r/'src/config/runtime_provider.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,
            r/'src/xml/attribute_parser.cpp',r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp'],show_extra)
# F1421-001/F1421-003: preserve Winsock failure policy and never let a
# cleanup failure disappear at the release-service/bootstrap boundary.
socket_runtime=(r/'src/net/socket_runtime.cpp').read_text(encoding='utf-8',errors='replace')
release_server=(r/'src/runtime/release_server.cpp').read_text(encoding='utf-8',errors='replace')
bootstrap_runner=(r/'src/runtime/bootstrap_runner.cpp').read_text(encoding='utf-8',errors='replace')
required_network_failure=[
 'const int startup_result = api->startup(api->context, winsock_requested_version);',
 'runtime->last_error = startup_result;',
 'if (context && context->runtime.started) {',
 'const auto status = preloader_l::net::stop_socket_runtime(&context->runtime);',
 'status != preloader_l::net::SocketStatus::not_started',
 'bool stop_and_shutdown_backend(const BootstrapBackend& backend, bool stop_listener) noexcept',
 'return BootstrapRunResult::shutdown_failed;',
]
combined_network=socket_runtime+release_server+bootstrap_runner
if any(x not in combined_network for x in required_network_failure):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1421 Winsock failure policy drift'); sys.exit(1)
if 'runtime->last_error = api->last_error(api->context);\n        runtime->api = nullptr;' in socket_runtime:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1421-001 startup still uses last_error policy'); sys.exit(1)
shutdown_start=release_server.find('bool shutdown_backend(void* opaque) noexcept')
shutdown_end=release_server.find('\n}\n\n}',shutdown_start)
shutdown_body=release_server[shutdown_start:shutdown_end] if shutdown_start>=0 and shutdown_end>shutdown_start else ''
if not shutdown_body or shutdown_body.find('stop_socket_runtime(&context->runtime)') > shutdown_body.find('try_shutdown()'):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1421-003 Winsock cleanup retry occurs after service shutdown'); sys.exit(1)
if '(void)backend.shutdown(backend.context);' in bootstrap_runner:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1421-003 bootstrap still discards shutdown failure'); sys.exit(1)

# M315-AUD-003: service acquisition happens after PTD/hook publication. A
# failed acquisition must transactionally tear down those pre-acquire owners,
# preserve retry policy on cleanup failure, and report shutdown_failed when
# the unwind cannot complete.
bootstrap_windows=(r/'src/runtime/bootstrap_thread.cpp').read_text(encoding='utf-8',errors='replace')
pre_required=[
 'bool finalize_pre_acquire_bootstrap_failure() noexcept',
 'if (teardown_bootstrap_failure_hooks() != HookTeardownResult::ok) return false;',
 'shutdown_release_service_after_bootstrap_failure();',
 'preloader_l::runtime::detach_current_thread_ptd();',
 'preloader_l::runtime::shutdown_ptd_runtime();',
 'preloader_l::runtime::ptd_cleanup_pending()',
 'const bool unwind_ok = finalize_pre_acquire_bootstrap_failure();',
 '? preloader_l::runtime::BootstrapRunResult::initialize_failed',
 ': preloader_l::runtime::BootstrapRunResult::shutdown_failed;',
]
if any(x not in bootstrap_windows for x in pre_required):
    print('BEHAVIORAL_REGRESSIONS: FAIL - M315-AUD-003 pre-acquire unwind markers missing'); sys.exit(1)
pre_start=bootstrap_windows.find('bool finalize_pre_acquire_bootstrap_failure() noexcept')
pre_end=bootstrap_windows.find('bool finalize_post_acquire_bootstrap_failure(',pre_start)
pre_body=bootstrap_windows[pre_start:pre_end] if pre_start>=0 and pre_end>pre_start else ''
pre_order=[
 'teardown_bootstrap_failure_hooks()',
 'shutdown_release_service_after_bootstrap_failure()',
 'detach_current_thread_ptd()',
 'shutdown_ptd_runtime()',
 'ptd_cleanup_pending()',
]
pre_positions=[pre_body.find(x) for x in pre_order]
if not pre_body or any(x < 0 for x in pre_positions) or pre_positions != sorted(pre_positions):
    print('BEHAVIORAL_REGRESSIONS: FAIL - M315-AUD-003 pre-acquire unwind ordering drift'); sys.exit(1)
acquire=bootstrap_windows.find('const auto service_status = preloader_l::runtime::acquire_release_service_once(')
acquire_fail=bootstrap_windows.find('if (service_status != preloader_l::runtime::ReleaseServiceAcquireResult::acquired',acquire)
bind_after=bootstrap_windows.find('bind_runtime_configuration_model_service',acquire_fail)
acquire_body=bootstrap_windows[acquire_fail:bind_after] if acquire_fail>=0 and bind_after>acquire_fail else ''
if 'finalize_pre_acquire_bootstrap_failure()' not in acquire_body or 'BootstrapRunResult::shutdown_failed' not in acquire_body:
    print('BEHAVIORAL_REGRESSIONS: FAIL - M315-AUD-003 acquire failure can bypass unwind'); sys.exit(1)

# F1413-002/F1413-003: environment publication failure is fatal and any
# post-acquire terminal failure must transactionally unwind temporary lease,
# host-facing hooks, then the permanent service owner/config-store policy.
bootstrap_windows=(r/'src/runtime/bootstrap_thread.cpp').read_text(encoding='utf-8',errors='replace')
service_header=(r/'include/preloader_l/runtime/release_service_lifetime.hpp').read_text(encoding='utf-8',errors='replace')
bootstrap_failure_required=[
 'bool restore_bootstrap_environment() noexcept',
 'HookTeardownResult teardown_bootstrap_failure_hooks(',
 'bool finalize_post_acquire_bootstrap_failure(',
 'if (environment_status != preloader_l::platform::EnvironmentStatus::ok)',
 'const bool environment_ok = restore_bootstrap_environment();',
 'const bool unwind_ok = finalize_post_acquire_bootstrap_failure(&service);',
 'shutdown_release_service_after_bootstrap_failure()',
]
if any(x not in bootstrap_windows for x in bootstrap_failure_required):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1413 bootstrap failure transaction drift'); sys.exit(1)
env_fail=bootstrap_windows.find('if (environment_status != preloader_l::platform::EnvironmentStatus::ok)')
registry_install=bootstrap_windows.find('install_windows_registry_iat_hooks()', env_fail)
if min(env_fail,registry_install)<0 or not env_fail < registry_install:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1413 environment failure still reaches later startup'); sys.exit(1)
finalize_start=bootstrap_windows.find('bool finalize_post_acquire_bootstrap_failure(')
finalize_end=bootstrap_windows.find('DWORD run_release_bootstrap_core()', finalize_start)
finalize_body=bootstrap_windows[finalize_start:finalize_end] if finalize_start>=0 and finalize_end>finalize_start else ''
required_finalize=[
 'release_release_service(service);',
 'teardown_bootstrap_failure_hooks()',
 'shutdown_release_service_after_bootstrap_failure()',
 'detach_current_thread_ptd()',
 'shutdown_ptd_runtime()',
 'ptd_cleanup_pending()',
]
if not finalize_body or any(x not in finalize_body for x in required_finalize):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1413 post-acquire unwind markers missing'); sys.exit(1)
positions=[finalize_body.find(x) for x in required_finalize]
if positions != sorted(positions):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1413/F1544 lease/hook/owner/PTD unwind ordering drift'); sys.exit(1)
if 'ptd_cleanup_pending()' not in finalize_body:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1544 post-acquire PTD/FLS cleanup completion is not verified'); sys.exit(1)
service_source=(r/'src/runtime/release_service_lifetime.cpp').read_text(encoding='utf-8',errors='replace')
if 'ReleaseServiceShutdownResult shutdown_release_service_after_bootstrap_failure() noexcept' not in service_source:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1413 permanent-owner failure cleanup missing'); sys.exit(1)
if 'ReleaseServiceShutdownResult shutdown_release_service_after_bootstrap_failure() noexcept;' not in service_header:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1413 failure cleanup contract missing'); sys.exit(1)

# F1415-001/F1415-002: native worker/client sockets must be explicit nonblocking
# endpoints, accept must use bounded readiness polling, and listener close must
# occur only after the accept owner has joined.
worker_src='\n'.join((
 (r/'src/runtime/release_server_worker.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/runtime/release_server_acceptor.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/runtime/release_server_lifecycle.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/runtime/release_server_io_internal.hpp').read_text(encoding='utf-8',errors='replace'),
))
worker_header=(r/'include/preloader_l/runtime/release_server_io.hpp').read_text(encoding='utf-8',errors='replace')
required_worker_liveness=[
 'configure_release_socket_nonblocking_windows',
 'wait_release_socket_windows',
 'ioctlsocket(socket, kFionbio, &enabled) == 0',
 'timeout.tv_usec = 100000',
 'if (!configure_release_socket_nonblocking_windows(client))',
 'const auto ready = wait_release_socket_windows(listener, false);',
]
if any(x not in worker_src for x in required_worker_liveness):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1415 nonblocking/poll policy drift'); sys.exit(1)
if 'ReleaseSocketWaitStatus' not in worker_header:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1415 wait policy contract missing'); sys.exit(1)
prepare_start=worker_src.find('bool prepare_release_listener_windows(')
prepare_end=worker_src.find('bool serve_release_workers_windows(',prepare_start)
prepare=worker_src[prepare_start:prepare_end] if prepare_start>=0 and prepare_end>prepare_start else ''
if 'configure_release_socket_nonblocking_windows(' not in prepare:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1415 listener is not made nonblocking'); sys.exit(1)
quiesce_start=worker_src.find('void quiesce_release_worker_policy_windows(')
quiesce_end=worker_src.find('void release_release_worker_policy_windows(',quiesce_start)
quiesce=worker_src[quiesce_start:quiesce_end] if quiesce_start>=0 and quiesce_end>quiesce_start else ''
wait_pos=quiesce.find('WaitForSingleObject(')
close_pos=quiesce.find('close_socket_listener(&context->listener)')
if min(wait_pos,close_pos)<0 or not wait_pos < close_pos:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1415 listener closes before accept-owner join'); sys.exit(1)
if quiesce[:wait_pos].find('close_socket_listener(&context->listener)') >= 0:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1415 concurrent listener close reintroduced'); sys.exit(1)

# F1412-001: shipping explicit unload must have an outside-loader-lock owner.
# The worker HANDLE stays live until a real kernel wait proves thread exit and
# therefore self-pin release. DllShutdown is additive @3 and fails
# closed on timeout, active leases, retained hooks, or pending FLS cleanup.
bootstrap_windows=(r/'src/runtime/bootstrap_thread.cpp').read_text(encoding='utf-8',errors='replace')
bootstrap_header=(r/'include/preloader_l/runtime/bootstrap_thread.hpp').read_text(encoding='utf-8',errors='replace')
dllmain=(r/'src/dll_entry.cpp').read_text(encoding='utf-8',errors='replace')
secondary=(r/'src/runtime/secondary_integration_entry.cpp').read_text(encoding='utf-8',errors='replace')
prod_def=(r/'exports/preloader_l.def').read_text(encoding='utf-8',errors='replace')
service_header=(r/'include/preloader_l/runtime/release_service_lifetime.hpp').read_text(encoding='utf-8',errors='replace')
required_shutdown=[
 'ReleaseRuntimeShutdownResult shutdown_release_runtime_for_unload(uint32_t timeout_ms) noexcept',
 'request_release_bootstrap_stop();',
 'const auto wait = wait_release_bootstrap_stopped(',
 'shutdown_release_service_after_quiescence()',
 'uninstall_crypt_query_observer()',
  'uninstall_connect_redirect()',
 'uninstall_windows_registry_iat_hooks()',
 'shutdown_minhook_runtime()',
 'shutdown_ptd_runtime();',
 'ptd_cleanup_pending()',
]
if any(x not in bootstrap_windows for x in required_shutdown):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 explicit unload sequence drift'); sys.exit(1)
shutdown_pos=bootstrap_windows.find('ReleaseRuntimeShutdownResult shutdown_release_runtime_for_unload')
shutdown_tail=bootstrap_windows[shutdown_pos:] if shutdown_pos>=0 else ''
hook_pos=shutdown_tail.find('teardown_bootstrap_failure_hooks(')
service_pos=shutdown_tail.find('shutdown_release_service_after_quiescence()')
ptd_pos=shutdown_tail.find('shutdown_ptd_runtime();')
if min(hook_pos,service_pos,ptd_pos)<0 or not (hook_pos < service_pos < ptd_pos):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 fail-closed teardown ordering drift'); sys.exit(1)
helper_start=bootstrap_windows.find('HookTeardownResult teardown_bootstrap_failure_hooks(')
helper_end=bootstrap_windows.find('bool finalize_pre_acquire_bootstrap_failure()',helper_start)
helper=bootstrap_windows[helper_start:helper_end] if helper_start>=0 and helper_end>helper_start else ''
for token in ['rollback_minhook_batch_transaction_with_timeout(', 'uninstall_crypt_query_observer()', 'uninstall_connect_redirect()', 'uninstall_windows_registry_iat_hooks()', 'shutdown_minhook_runtime()']:
    if token not in helper:
        print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 consolidated hook teardown helper drift'); sys.exit(1)
if 'WaitForSingleObject(thread, wait_ms)' not in bootstrap_windows or 'FreeLibraryAndExitThread(pinned_module, result)' not in bootstrap_windows:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 kernel quiescence/self-pin proof missing'); sys.exit(1)
thread_start=bootstrap_windows.find('DWORD __stdcall release_bootstrap_thread')
thread_end=bootstrap_windows.find('namespace preloader_l { namespace runtime {',thread_start)
thread_body=bootstrap_windows[thread_start:thread_end] if thread_start>=0 and thread_end>thread_start else ''
if 'CloseHandle(' in thread_body:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 worker still destroys join HANDLE'); sys.exit(1)
if 'DllShutdown                             @3' not in prod_def and 'DllShutdown                    @3' not in prod_def:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 production lifecycle export missing/additive ordinal drift'); sys.exit(1)
if 'extern "C" uint32_t __stdcall DllShutdown(uint32_t timeout_ms) noexcept' not in dllmain:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 exported shutdown wrapper missing'); sys.exit(1)
if 'shutdown_release_service_after_quiescence() noexcept' not in service_header:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412-001 service teardown remains acceptance-only'); sys.exit(1)
if 'DllShutdown' not in dllmain or 'preloader_l_shutdown' in dllmain or 'preloader_l_shutdown' in prod_def:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1412 lifecycle export rename/ABI drift'); sys.exit(1)

# SAFE-FREEZE transaction closure of F1412-002.
# DllMain/secondary attach must never enter MinHook's Freeze/Apply path. It only
# schedules the bootstrap thread. Windows serializes DLL initialization so the
# worker cannot execute DLL code until DllMain has returned. The worker's first
# operation stages all nine MinHook hooks and commits them through one snapshot-free
# NtGetNextThread safe-freeze transaction, then continues PTD/config/network startup.
forbidden_dllmain=[
 'initialize_ptd_runtime_windows(', 'detach_current_thread_ptd(', 'shutdown_ptd_runtime(',
 'install_crypt_query_observer', 'uninstall_',
 'shutdown_minhook_runtime(', 'WaitForSingleObject(', 'VirtualAlloc(', 'VirtualProtect(',
]
if any(x in dllmain for x in forbidden_dllmain):
    print('BEHAVIORAL_REGRESSIONS: FAIL - SAFE-FREEZE heavy DllMain bridge operation reintroduced'); sys.exit(1)
forbidden_secondary=[
 'install_crypt_query_observer_main_module()',
 'install_windows_registry_minhook_early()', 'install_connect_redirect()', 'MH_ApplyQueued',
 'initialize_ptd_runtime_windows(', 'detach_current_thread_ptd(', 'shutdown_ptd_runtime(',
 'WaitForSingleObject(', 'run_persistent_bootstrap(', 'load_process_runtime_configuration(',
]
if any(x in secondary for x in forbidden_secondary):
    print('BEHAVIORAL_REGRESSIONS: FAIL - SAFE-FREEZE loader-lock hook mutation reintroduced'); sys.exit(1)
if 'schedule_release_bootstrap(module)' not in secondary:
    print('BEHAVIORAL_REGRESSIONS: FAIL - SAFE-FREEZE bootstrap dispatch missing'); sys.exit(1)
worker_init=bootstrap_windows.find('DWORD run_release_bootstrap_core() noexcept')
worker_tail=bootstrap_windows[worker_init:] if worker_init>=0 else ''
# HF18 deliberately changed the old hook-first bootstrap to config-first.
# Assert the genuine current order, not the inherited HF17 literal.
worker_order=[
 'initialize_ptd_runtime_windows()',
 'acquire_release_service_once(',
 'load_process_runtime_configuration()',
 'freeze_runtime_configuration_for_hooks()',
 'read_committed_addon_selection(&selection)',
 'install_post_loader_minhook_surface(selection)',
 'install_windows_registry_iat_hooks()',
 'run_persistent_bootstrap(',
]
positions=[worker_tail.find(token) for token in worker_order]
if any(pos<0 for pos in positions) or positions!=sorted(positions) or len(set(positions))!=len(positions):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF18 config-first PTD/service/CFG/freeze/hooks ordering drift'); sys.exit(1)
required_surface=[
 'install_crypt_query_observer_main_module()',
 'install_windows_registry_minhook_early()',
 'install_connect_redirect()',
]
core_hook_source=(r/'src/platform/core_hooks.cpp').read_text(encoding='utf-8',errors='replace')
if any(x not in core_hook_source for x in required_surface):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF18 core hook staging surface incomplete'); sys.exit(1)
stage_owner=bootstrap_windows[bootstrap_windows.find('PostLoaderHookInstallResult install_post_loader_minhook_surface('):worker_init]
if not (0<=stage_owner.find('begin_minhook_batch_transaction()')
    <stage_owner.find('stage_core_hooks()')
    <stage_owner.find('stage_addon_hooks(selection.mask)')
    <stage_owner.find('commit_minhook_batch_transaction()')):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF18 one-transaction core+addon order drift'); sys.exit(1)
if ('failed_ownership_retained' not in worker_tail
    or 'finalize_post_acquire_bootstrap_failure(&service)' not in worker_tail):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF18 rollback/ownership closure drift'); sys.exit(1)
patch_path=r/'scripts/build_orchestration/PatchMinHookSafeFreeze.cmake'
patch_text=patch_path.read_text(encoding='utf-8',errors='replace') if patch_path.exists() else ''
if 'NtGetNextThread' not in patch_text or 'PRELOADER_L_MH_SAFE_FREEZE_V1' not in patch_text:
    print('BEHAVIORAL_REGRESSIONS: FAIL - SAFE-FREEZE MinHook enumerator patch missing'); sys.exit(1)
cmake_safe=(r/'CMakeLists.txt').read_text(encoding='utf-8',errors='replace')
if 'PatchMinHookSafeFreeze.cmake' not in cmake_safe:
    print('BEHAVIORAL_REGRESSIONS: FAIL - SAFE-FREEZE CMake patch staging missing'); sys.exit(1)
if 'ResumeThread(' in bootstrap_windows or 'kCreateSuspended' in bootstrap_windows:
    print('BEHAVIORAL_REGRESSIONS: FAIL - suspended/resume bootstrap dispatch reintroduced'); sys.exit(1)
if 'CreateThread(nullptr, 0, &release_bootstrap_thread, pinned,' not in bootstrap_windows:
    print('BEHAVIORAL_REGRESSIONS: FAIL - bounded bootstrap dispatch missing'); sys.exit(1)

# F1446-002: exercise the real backend enter/leave accounting with the Windows
# backend object; section GC discards unrelated hook machinery and the test
# supplies only the scheduler yield import reached by the quiescence path.
if run_heavy_case('preloader_l_hotfix03_delivery_rollback'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_hotfix03_delivery_rollback_') as td:
        td=Path(td)
        test_obj=td/'test.o'; event_obj=td/'event.o'; exe=td/'grant_delivery_rollback'
        subprocess.run(common+['-c',str(r/'tests/unit/test_grant_delivery_rollback.cpp'),'-o',str(test_obj)],cwd=r,check=True)
        behavioral_objs=[]
        for i,src in enumerate(behavioral_state_sources):
            obj=td/f'behavioral_{i}.o'; behavioral_objs.append(obj)
            subprocess.run(common+['-Wno-unused-function','-c',str(src),'-o',str(obj)],cwd=r,check=True)
        subprocess.run(common+['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-misleading-indentation',
                              '-c',str(r/'src/runtime/event_queue.cpp'),'-o',str(event_obj)],cwd=r,check=True)
        peg_obj=td/'config2_pegtl.o'
        subprocess.run(common+['-c',str(r/'src/config/config2_pegtl.cpp'),'-o',str(peg_obj)],cwd=r,check=True)
        subprocess.run([cxx,str(test_obj)]+[str(x) for x in behavioral_objs]+[str(event_obj),str(peg_obj),'-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

# AUDIT-HOTFIX05 / F1426-001: exercise the centralized response/value-codec
# path for five XML specials plus XML-legal TAB/LF/CR semantic attribute data.
# This keeps the serializer from regressing to reject-all-C0 behavior while
# preserving rejection of non-XML controls.
if run_heavy_case('preloader_l_xml_attribute_semantics'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_xml_attribute_semantics_') as td:
        td=Path(td); exe=td/'xml_attribute_semantics'
        subprocess.run(common+[str(r/'tests/unit/test_response.cpp'),str(r/'src/xml/response_builder.cpp'),*[str(x) for x in value_codec_sources],'-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

# F1438-002: exercise the real shipping GrantAchievement handler across
# tiny/exact capacity failure, retry, event-allocation failure, FIFO consistency,
# and watcher rejection. Build the monolithic route TU with section GC so the
# regression does not need unrelated platform backends.
if run_heavy_case('preloader_l_grant_atomicity'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_grant_atomicity_') as td:
        td=Path(td); obj=td/'obj'; obj.mkdir(); exe=td/'grant_failure_atomicity'
        win=['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-misleading-indentation']
        atom_common=[cxx,'-std=c++17','-O0','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-I',str(r/'include')]
        units=[
          (r/'tests/unit/test_grant_achievement_failure_atomicity.cpp',[]),
          *[(src,win+['-Wno-unused-function']) for src in achievement_route_sources],
          *[(src,[]) for src in transient_owner_sources],
          *[(src,['-Wno-unused-function']) for src in behavioral_state_sources],
          (r/'src/runtime/event_queue.cpp',win),
          (r/'src/runtime/achievement_watcher.cpp',[]),
          (r/'src/runtime/utc_time.cpp',win),
          *[(src,[]) for src in request_sources],(r/'src/xml/lsx_framing.cpp',[]),(r/'src/xml/response_builder.cpp',[]),
          *[(src,[]) for src in value_codec_sources],(r/'src/xml/attribute_parser.cpp',[]),
          (r/'src/config/runtime_provider.cpp',[]),(r/'src/config/settings_values.cpp',[]),(r/'src/config/settings_provider.cpp',[]),
          (r/'src/handlers/selector_pipeline.cpp',[]),
        ]
        units=close_config2_atom_units(units)
        objects=[]
        for i,(src,extra) in enumerate(units):
            out=obj/f'{i}.o'; objects.append(out)
            subprocess.run(atom_common+extra+['-c',str(src),'-o',str(out)],cwd=r,check=True)
        subprocess.run([cxx,'-Wl,--gc-sections']+[str(x) for x in objects]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)
# F1416-001: generic large-allocation release must validate curio raw/user
# metadata before forwarding anything to the heap backend.
allocator=(r/'src/runtime/memory_allocator.cpp').read_text(encoding='utf-8',errors='replace')
allocator_header=(r/'include/preloader_l/runtime/memory_allocator.hpp').read_text(encoding='utf-8',errors='replace')
required_allocator_release=[
 'enum class AllocationReleaseStatus : uint32_t',
 'malformed_large_allocation = 1',
 'logical_size > static_cast<size_t>(-1) - large_allocation_overhead',
 'raw_address >= aligned || aligned - raw_address < 8u',
 'recovered_delta > 0x1fu',
]
if any(x not in allocator_header+allocator for x in required_allocator_release):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1416-001 large-release validation drift'); sys.exit(1)
if run_heavy_case('preloader_l_allocator_release_validation'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_allocator_release_validation_') as td:
        td=Path(td); exe=td/'allocator_release_validation'
        subprocess.run(common+[str(r/'tests/unit/test_allocator_release_validation.cpp'),str(r/'src/runtime/memory_allocator.cpp'),str(r/'src/runtime/ptd_runtime.cpp'),'-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

# F1435-001/F1435-003: explicit-empty Content fields must not suppress the
# retained Phase209 defaults. Exercise the real service-store Config2 adapter.
service_state=behavioral_state_text
required_content_empty_default=[
 'if(teq(c,key,"Name"))return v.size?replace_text(r->display_name,v):true;',
 'if(teq(c,key,"Version"))return v.size?replace_text(r->version,v):true;',
 'if(teq(c,key,"State"))return v.size?replace_text(r->state,v):true;',
]
if any(x not in service_state for x in required_content_empty_default):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1435-001 Content empty-default policy drift'); sys.exit(1)
if run_heavy_case('preloader_l_content_empty_defaults'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_content_empty_defaults_') as td:
        td=Path(td); exe=td/'content_empty_defaults'
        subprocess.run(common+['-Wno-unused-function',str(r/'tests/unit/test_content_empty_defaults.cpp')]+[str(x) for x in behavioral_state_sources]+[str(r/'src/config/config2_pegtl.cpp')]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)


# F1435-002/F1435-003: Content entry identity may still instantiate a record,
# but recognized Name/Version/State fields are authoritative only as direct
# children of Content/<id>. Nested descendants must not override direct fields.
service_state=behavioral_state_text
required_content_descendant=[
 'if(depth>=2&&teq(c,path[0],"Content"))',
 'if(depth!=2u)return true;',
]
if any(x not in service_state for x in required_content_descendant):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1435-002 Content descendant policy drift'); sys.exit(1)
if run_heavy_case('preloader_l_content_descendant_policy'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_content_descendant_policy_') as td:
        td=Path(td); exe=td/'content_descendant_policy'
        subprocess.run(common+['-Wno-unused-function',str(r/'tests/unit/test_content_descendant_policy.cpp')]+[str(x) for x in behavioral_state_sources]+[str(r/'src/config/config2_pegtl.cpp')]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)


# F1441-001/F1441-002 + F323-S0329-001: failed RegOpenKeyExA/W may leave
# phkResult untouched, and failed nonempty UTF-8->UTF-16 sizing must not be
# published as a successful empty REG_SZ.
host_bridge=(r/'src/platform/registry_bridge.cpp').read_text(encoding='utf-8',errors='replace')
required_failed_open=[
 'const uintptr_t original_result=(result&&original==host_error_success)',
 '? reinterpret_cast<uintptr_t>(*result) : 0u;',
]
if any(x not in host_bridge for x in required_failed_open):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1441-001 failed-open output handle read regression'); sys.exit(1)
registry_values=(r/'src/platform/registry_policy_values.cpp').read_text(encoding='utf-8',errors='replace')
required_registry_conversion=[
 'if(value.size==0u){',
 'if(need<=0)return host_error_invalid_parameter;',
 'void normalize_type(uint32_t* type) noexcept { if(type)*type=host_reg_sz; }',
]
if any(x not in registry_values for x in required_registry_conversion) or 'if(type&&*type==0u)' in registry_values:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F323-S0329/S0333 registry output classification drift'); sys.exit(1)
if run_heavy_case('preloader_l_registry_failed_open'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_registry_failed_open_') as td:
        td=Path(td); exe=td/'registry_failed_open_outparam'
        extra=['-D_WIN32','-D_WIN64','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function','-Wno-misleading-indentation']
        subprocess.run(common+extra+[str(r/'tests/unit/test_registry_output.cpp'),str(r/'src/platform/registry_bridge.cpp'),str(r/'src/platform/registry_policy.cpp'),str(r/'src/platform/registry_policy_config.cpp'),str(r/'src/platform/registry_policy_state.cpp'),str(r/'src/platform/registry_policy_values.cpp'),str(r/'src/config/settings_values.cpp'),str(r/'src/net/port_mapping.cpp'),'-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

# F323-S0352-001: capture GetLastError immediately after CreateFileA/W failure
# and classify only missing file/path as the optional not_found condition.
config_file_errors=(r/'src/config/runtime_configuration_file.cpp').read_text(encoding='utf-8',errors='replace')
required_config_open_error=[
 'GetLastError()',
 'error_file_not_found=2u, error_path_not_found=3u',
 'return error==error_file_not_found||error==error_path_not_found',
 ': RuntimeConfigurationFileStatus::io_error;',
]
if any(x not in config_file_errors for x in required_config_open_error):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F323-S0352-001 native config open error classification drift'); sys.exit(1)
if run_heavy_case('preloader_l_runtime_configuration_file_errors'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_runtime_configuration_file_errors_') as td:
        td=Path(td); exe=td/'runtime_configuration_file_errors'
        extra=['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function']
        # PEGTL compiles as a portable TU; the fake-Win32 test needs _WIN32
        # only for the loader I/O TU, not for the PEGTL header platform choice.
        pegtl_obj=td/'config2_pegtl.o'
        subprocess.run(common+['-c',str(r/'src/config/config2_pegtl.cpp'),'-o',str(pegtl_obj)],cwd=r,check=True)
        subprocess.run(common+extra+[str(r/'tests/unit/test_runtime_configuration_file_errors.cpp'),str(r/'src/config/runtime_configuration_file.cpp'),str(pegtl_obj),'-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)


# F1444-001: PE import descriptor traversal must stay within the declared
# Import Directory [RVA, RVA+Size) extent, while thunk/name RVAs remain
# validated against the mapped image extent.
pe_import=(r/'src/platform/pe_import.cpp').read_text(encoding='utf-8',errors='replace')
required_pe_import_bounds=[
 'const size_t import_extent = static_cast<size_t>(import_size);',
 'if (import_start >= image_size || import_extent > image_size - import_start) return false;',
 'if (!within(desc, 20u, import_end)) return false;',
 'if (import_start >= image_size || import_extent > image_size - import_start) return 0;',
 'if (!within(desc, 20u, import_end)) return matches;',
]
if any(x not in pe_import for x in required_pe_import_bounds):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1444-001 PE import directory extent bounds drift'); sys.exit(1)
if run_heavy_case('preloader_l_pe_import_directory_bounds'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_pe_import_directory_bounds_') as td:
        td=Path(td); exe=td/'pe_import_directory_bounds'
        subprocess.run(common+[str(r/'tests/unit/test_pe_import_directory_bounds.cpp'),str(r/'src/platform/pe_import.cpp'),'-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)



# F1437-001/F1437-003: QueryAchievements All uses the untrimmed direct-child
# text policy. Exact semantic lowercase true enables zero-progress records;
# leading/trailing whitespace, case/value variants, and empty values do not.
achievement_routes=achievement_query_text
required_query_achievements_all=[
 'size_t all_cursor=0;',
 'preloader_l::xml::next_direct_child(r.command_content,&all_cursor,&child)',
 'all_found=true;all_text=child.content;',
 'include_all=preloader_l::xml::source_value_equals_literal(all_text,"true",&semantic_status);',
]
query_start=achievement_routes.find('HandlerStatus handle_query_achievements(')
query_end=len(achievement_routes)
query_block=achievement_routes[query_start:query_end] if query_start>=0 and query_end>query_start else ''
if not query_block or any(x not in query_block for x in required_query_achievements_all) or 'find_direct_child_text(r.command_content,"All"' in query_block:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1437-001 QueryAchievements All raw-text policy drift'); sys.exit(1)
if run_heavy_case('preloader_l_query_achievements_all'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_query_achievements_all_') as td:
        td=Path(td); obj=td/'obj'; obj.mkdir(); exe=td/'query_achievements_all_whitespace'
        atom_common=[cxx,'-std=c++17','-O0','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-I',str(r/'include')]
        units=[
          (r/'tests/unit/test_query_achievements_all_whitespace.cpp',[]),
          *[(src,['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function','-Wno-misleading-indentation']) for src in achievement_route_sources],
          (r/'src/runtime/achievement_watcher.cpp',[]),
          *[(src,[]) for src in transient_owner_sources],
          *[(src,['-Wno-unused-function']) for src in behavioral_state_sources],
          *[(src,[]) for src in request_sources],(r/'src/xml/lsx_framing.cpp',[]),(r/'src/xml/response_builder.cpp',[]),
          *[(src,[]) for src in value_codec_sources],(r/'src/xml/attribute_parser.cpp',[]),
          (r/'src/config/runtime_configuration.cpp',['-Wno-unused-function']),(r/'src/config/runtime_configuration_parser.cpp',[]),(r/'src/config/runtime_configuration_storage.cpp',[]),(r/'src/config/runtime_provider.cpp',[]),
          (r/'src/config/settings_values.cpp',[]),(r/'src/config/settings_provider.cpp',[]),
          (r/'src/handlers/selector_pipeline.cpp',[]),
        ]
        units=close_config2_atom_units(units)
        objects=[]
        for i,(src,extra) in enumerate(units):
            out=obj/f'{i}.o'; objects.append(out)
            subprocess.run(atom_common+extra+['-c',str(src),'-o',str(out)],cwd=r,check=True)
        subprocess.run([cxx,'-Wl,--gc-sections']+[str(x) for x in objects]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

# Inherited closure: the three routes whose offline-only special cases were removed
# must execute their normal semantics while Online=false. GetAuthCode has its
# own executable policy block immediately below.
if run_heavy_case('preloader_l_offline_route_closure'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_offline_route_closure_') as td:
        td=Path(td); obj=td/'obj'; obj.mkdir(); exe=td/'offline_route_closure'
        atom_common=[cxx,'-std=c++17','-O0','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-I',str(r/'include')]
        units=[
          (r/'tests/unit/test_offline_route_closure.cpp',[]),
          (r/'src/handlers/store_routes.cpp',['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function','-Wno-misleading-indentation']),
          *[(src,['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function','-Wno-misleading-indentation']) for src in achievement_route_sources],
          *[(src,[]) for src in transient_owner_sources],
          *[(src,['-Wno-unused-function']) for src in behavioral_state_sources],
          (r/'src/runtime/utc_time.cpp',[]),(r/'src/runtime/achievement_watcher.cpp',[]),(r/'src/runtime/event_queue.cpp',['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-misleading-indentation']),
          *[(src,[]) for src in request_sources],(r/'src/xml/lsx_framing.cpp',[]),(r/'src/xml/response_builder.cpp',[]),
          *[(src,[]) for src in value_codec_sources],(r/'src/xml/attribute_parser.cpp',[]),
          (r/'src/config/runtime_configuration.cpp',['-Wno-unused-function']),(r/'src/config/runtime_configuration_parser.cpp',[]),(r/'src/config/runtime_configuration_storage.cpp',[]),(r/'src/config/runtime_provider.cpp',[]),
          (r/'src/config/settings_values.cpp',[]),(r/'src/config/settings_provider.cpp',[]),
          (r/'src/handlers/selector_pipeline.cpp',[]),
        ]
        units=close_config2_atom_units(units)
        objects=[]
        for i,(src,extra) in enumerate(units):
            out=obj/f'{i}.o'; objects.append(out)
            subprocess.run(atom_common+extra+['-c',str(src),'-o',str(out)],cwd=r,check=True)
        subprocess.run([cxx,'-Wl,--gc-sections']+[str(x) for x in objects]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

# F1432-003: execute the real GetAuthCode handler transaction, not only the
# service-level Auth8 helper. Measurement/tiny/exact-capacity failures must not
# commit a session token; successful render commits once and remains stable.
if run_heavy_case('preloader_l_auth8_handler_policy'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_auth8_handler_policy_') as td:
        td=Path(td); obj=td/'obj'; obj.mkdir(); exe=td/'connectivity_auth_policy'
        atom_common=[cxx,'-std=c++17','-O0','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-I',str(r/'include')]
        units=[
          (r/'tests/unit/test_connectivity_auth_policy.cpp',[]),
          (r/'src/handlers/auth_routes.cpp',['-D_WIN32','-D__stdcall=','-D__declspec(x)=','-Wno-unused-function','-Wno-misleading-indentation']),
          *[(src,['-Wno-unused-function']) for src in behavioral_state_sources],
          (r/'src/runtime/auth_code_service.cpp',[]),
          *[(src,[]) for src in request_sources],(r/'src/xml/lsx_framing.cpp',[]),(r/'src/xml/response_builder.cpp',[]),
          *[(src,[]) for src in value_codec_sources],(r/'src/xml/attribute_parser.cpp',[]),
          (r/'src/config/runtime_configuration.cpp',['-Wno-unused-function']),(r/'src/config/runtime_configuration_parser.cpp',[]),(r/'src/config/runtime_configuration_storage.cpp',[]),(r/'src/config/runtime_provider.cpp',[]),
          (r/'src/config/settings_values.cpp',[]),(r/'src/config/settings_provider.cpp',[]),
          (r/'src/handlers/selector_pipeline.cpp',[]),
        ]
        units=close_config2_atom_units(units)
        objects=[]
        for i,(src,extra) in enumerate(units):
            out=obj/f'{i}.o'; objects.append(out)
            subprocess.run(atom_common+extra+['-c',str(src),'-o',str(out)],cwd=r,check=True)
        subprocess.run([cxx,'-Wl,--gc-sections']+[str(x) for x in objects]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)


# F1445-004 + F1444-002/F1447-004: compatibility-only Crypt helpers and the
# compatibility conservative x64 entry planner must stay out of the shipping graph.
# The planner remains available to debug/tests, but REX.W MOVABS must decode
# its full imm64 length so the compatibility surface cannot silently regress.
cmake_text=(r/'CMakeLists.txt').read_text(encoding='utf-8',errors='replace')
prod_match=__import__('re').search(r'add_library\(preloader_l SHARED(.*?)\n\)',cmake_text,__import__('re').S)
prod_block=prod_match.group(1) if prod_match else ''
for retired in ['src/debug/evidence/x64_inline_hook_plan.cpp','src/debug/evidence/crypt_guard.cpp','src/debug/evidence/crypt_path_restore.cpp']:
    if retired in prod_block:
        print('BEHAVIORAL_REGRESSIONS: FAIL - dead/compatibility TU returned to production graph: '+retired); sys.exit(1)
crypt_prod=(r/'src/platform/crypt_observer.cpp').read_text(encoding='utf-8',errors='replace')
if '#include "preloader_l/platform/crypt_path_restore.hpp"' in crypt_prod:
    print('BEHAVIORAL_REGRESSIONS: FAIL - dead crypt_path_restore include returned to shipping observer'); sys.exit(1)
# HF04: keep debug-only Crypt diagnostics out of the shipping observer TU.
crypt_diag_path=r/'src/debug/platform/crypt_observer_diagnostics.cpp'
if not crypt_diag_path.exists():
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF04 crypt diagnostics TU missing'); sys.exit(1)
crypt_diag=crypt_diag_path.read_text(encoding='utf-8',errors='replace')
debug_match=__import__('re').search(r'add_library\(preloader_l_debug SHARED(.*?)\n    \)',cmake_text,__import__('re').S)
debug_block=debug_match.group(1) if debug_match else ''
if 'src/debug/platform/crypt_observer_diagnostics.cpp' in prod_block:
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF04 debug crypt diagnostics entered production graph'); sys.exit(1)
if 'src/debug/platform/crypt_observer_diagnostics.cpp' not in debug_block:
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF04 debug crypt diagnostics missing from debug graph'); sys.exit(1)
for token in ['ObserverSlotPatch','observed_crypt_msg_get_param','observed_cert_find_certificate_in_store','trace_query_caller_provenance','K32EnumProcessModules','VirtualProtect(']:
    if token in crypt_prod:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF04 diagnostic implementation leaked back into production observer: '+token); sys.exit(1)
for token in ['ObserverSlotPatch','observed_crypt_msg_get_param','observed_cert_find_certificate_in_store','trace_query_caller_provenance','install_loaded_modules']:
    if token not in crypt_diag:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF04 diagnostic implementation marker missing: '+token); sys.exit(1)
for token in ['trace_crypt_query_diagnostics(object_type, caller);','trace_crypt_query_companion_branch();','uninstall_crypt_observer_diagnostics()']:
    if token not in crypt_prod:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF04 production diagnostic facade hook missing: '+token); sys.exit(1)
if 'x64_inline_hook_plan' in prod_block or 'x64_trampoline_relocator' in prod_block:
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - retired custom relocation planner returned to production'); sys.exit(1)


# HF06: keep runtime configuration split across façade, parser and storage.
runtime_config_facade=(r/'src/config/runtime_configuration.cpp').read_text(encoding='utf-8',errors='replace')
runtime_config_parser=(r/'src/config/runtime_configuration_parser.cpp').read_text(encoding='utf-8',errors='replace')
runtime_config_storage=(r/'src/config/runtime_configuration_storage.cpp').read_text(encoding='utf-8',errors='replace')
if any(not p.is_file() for p in [r/'src/config/runtime_configuration_internal.hpp',*runtime_config_sources]):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF06 runtime configuration decomposition drift'); sys.exit(1)
for token in ['ParseCode parse_config2(', 'SectionPolicy reserved_key_policy(', 'config2_pegtl::parse(data,length,visitor)']:
    if token not in runtime_config_parser:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF06 parser marker missing: '+token); sys.exit(1)
for token in ['void* transient_alloc(', 'void reset_state(', 'bool put_setting(']:
    if token not in runtime_config_storage:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF06 storage marker missing: '+token); sys.exit(1)
for token in ['set_runtime_configuration_cfg(', 'clear_runtime_configuration()', 'bind_runtime_configuration_model_service(']:
    if token not in runtime_config_facade:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF06 façade marker missing: '+token); sys.exit(1)
for token in ['enum class TokenKind', 'ParseCode parse_object(', 'bool reserve_arena(', 'bool put_setting(']:
    if token in runtime_config_facade:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF06 implementation leaked into façade: '+token); sys.exit(1)

# HF11 migrates HF05's XML and entitlement scanners into exactly one pinned
# TinyXML2 engine + StoreState-owned entitlement projector/materializer.
store_value=(r/'src/config/store_values.cpp').read_text(encoding='utf-8',errors='replace')
store_document=(r/'src/config/store_parser.cpp').read_text(encoding='utf-8',errors='replace')
store_internal=(r/'src/config/store_internal.hpp').read_text(encoding='utf-8',errors='replace')
store_runtime=(r/'src/config/store_runtime.cpp').read_text(encoding='utf-8',errors='replace')
store_model_bridge=(r/'src/config/store_service_bridge.cpp').read_text(encoding='utf-8',errors='replace')
store_materialization=(r/'src/config/store_materialization.cpp').read_text(encoding='utf-8',errors='replace')
for retired in ('src/config/store_xml.cpp','src/config/store_entitlements.cpp','src/xml/request_scanner.cpp'):
    if (r/retired).exists():
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF11 legacy XML parser returned: '+retired); sys.exit(1)
for token in ['ParseCode decode_quartets(', 'ParseCode decode_hex(']:
    if token not in store_value:
        print('BEHAVIORAL_REGRESSIONS: FAIL - Store primitive decoder missing: '+token); sys.exit(1)
for token in ['tinyxml2::XMLDocument', 'ParseCode parse_entitlement(', 'ParseCode parse_store(', 'ParseCode parse_document(']:
    if token not in store_document:
        print('BEHAVIORAL_REGRESSIONS: FAIL - Store TinyXML2 projector missing: '+token); sys.exit(1)
store_owner_surface='\n'.join([store_internal,store_runtime,store_model_bridge,store_materialization,store_value,store_document])
for token in ['g_parse_store','parse_store()']:
    if token in store_owner_surface:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF17 implicit parse-store owner reappeared: '+token); sys.exit(1)
for token in ['void reset_store(StoreState& state)', 'ParseCode parse_entitlement(StoreState& state',
              'ParseCode parse_store(StoreState& state', 'ParseCode parse_document(StoreState& state']:
    if token not in store_owner_surface:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF17 explicit StoreState marker missing: '+token); sys.exit(1)

# Wave3 experiment: destination-aware connect redirect is the sole LSX port
# remap policy and MinHook remains the sole hook coordinator. HostBridge stays
# registry-only; the retired process-wide htons interception must not return.
host_header=(r/'include/preloader_l/platform/registry_bridge.hpp').read_text(encoding='utf-8',errors='replace')
host_win_header=(r/'include/preloader_l/platform/registry_hook_runtime.hpp').read_text(encoding='utf-8',errors='replace')
host_win='\n'.join((r/p).read_text(encoding='utf-8',errors='replace') for p in ['src/platform/registry_hook_runtime.cpp','src/platform/registry_wrappers.cpp','src/platform/registry_hooks.cpp'])
bootstrap_win=(r/'src/runtime/bootstrap_thread.cpp').read_text(encoding='utf-8',errors='replace')
if (r/'src/platform/hook_coordinator.cpp').exists() or (r/'include/preloader_l/platform/hook_coordinator.hpp').exists():
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - legacy generic hook coordinator returned'); sys.exit(1)
compatibility_htons_tokens=['HtonsOriginal','htons_fn','bridge_htons']
if any(x in host_header for x in compatibility_htons_tokens):
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - compatibility htons policy remains in HostBridge state/API'); sys.exit(1)
if 'set_windows_host_selected_port' in host_win_header or 'set_windows_host_selected_port' in host_win:
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - compatibility HostBridge selected-port publisher remains'); sys.exit(1)
if 'preloader_l_htons' in host_win or 'bridge_htons' in host_win:
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - compatibility HostBridge htons wrapper remains'); sys.exit(1)
if ('set_connect_selected_port(port);' not in bootstrap_win
    or core_hook_source.count('const auto connect_status = install_connect_redirect();') != 1
    or stage_owner.count('stage_core_hooks()') != 1):
    print('BEHAVIORAL_REGRESSIONS_MH: FAIL - HF18 single core-staged destination-aware connect policy drift'); sys.exit(1)
if run_heavy_case('preloader_l_f1442_single_connect_policy'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_wave3_connect_redirect_') as td:
        td=Path(td); hr=td/'connect_redirect'
        subprocess.run(common+[str(r/'tests/unit/test_connect_redirect.cpp'),str(r/'src/platform/connect_redirect.cpp'),'-o',str(hr)],cwd=r,check=True)
        subprocess.run([str(hr)],cwd=r,check=True)

# F1447-001: executable breadth must cover representative production-risk
# codec, framing, transport, routing, settings and runtime-settings surfaces;
# syntax-only coverage is not accepted as behavioral evidence.
if run_heavy_case('preloader_l_f1447_exec_breadth'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_f1447_exec_breadth_') as td:
        td=Path(td)
        breadth_common=[cxx,'-std=c++20','-Wall','-Wextra','-Werror','-I',str(r/'third_party/pegtl/upstream/include'),'-Wno-unused-function','-I',str(r/'include')]
        breadth=[
          ('framing',[r/'tests/unit/test_framing.cpp',r/'src/xml/lsx_framing.cpp']),
          ('hex',[r/'tests/unit/test_hex.cpp',r/'src/codec/hex_codec.cpp']),
          ('aes128',[r/'tests/unit/test_aes128.cpp',r/'src/codec/aes128_cipher.cpp']),
          ('fnv',[r/'tests/unit/test_fnv.cpp',r/'src/hash/fnv_hash.cpp']),
          ('port',[r/'tests/unit/test_port.cpp',r/'src/net/port_mapping.cpp']),
          ('transport',[r/'tests/unit/test_transport.cpp',r/'src/net/socket_transport.cpp']),
          ('receive_buffer',[r/'tests/unit/test_receive_buffer.cpp',r/'src/debug/net/receive_buffer.cpp']),
          ('network_session',[r/'tests/unit/test_network_session.cpp',r/'src/debug/net/session_state.cpp',r/'src/net/socket_transport.cpp']),
          ('message_codec',[r/'tests/unit/test_message_codec.cpp',r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp']),
          ('codec_session',[r/'tests/unit/test_codec_session.cpp',r/'src/codec/session_crypto.cpp',r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp']),
          ('router',[r/'tests/unit/test_router.cpp',r/'src/router/route_lookup.cpp',r/'src/router/route_inventory.cpp',r/'src/hash/fnv_hash.cpp']),
          ('inventory',[r/'tests/unit/test_inventory.cpp',r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp']),
          ('settings',[r/'tests/unit/test_settings.cpp',r/'src/config/settings_values.cpp']),
          ('progressive_installation',[r/'tests/unit/test_progressive_installation.cpp',r/'src/handlers/progressive_installation_route.cpp',*request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp']),
          ('internet_state',[r/'tests/unit/test_connection_state.cpp',r/'src/handlers/connection_state_route.cpp',*request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp']),
        ]
        settings_base=[r/'src/handlers/setting_value_route.cpp',r/'src/handlers/settings_snapshot_route.cpp',r/'src/handlers/connection_state_route.cpp',r/'src/debug/handlers/settings_query.cpp',*request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp']
        breadth.append(('get_setting',[r/'tests/unit/test_setting_value.cpp']+settings_base))
        breadth.append(('get_settings',[r/'tests/unit/test_settings_snapshot.cpp']+settings_base))
        breadth.append(('runtime_settings_routes',[r/'tests/unit/test_runtime_settings_routes.cpp',r/'src/handlers/runtime_settings_routes.cpp',r/'src/handlers/selector_pipeline.cpp']+runtime_config_sources+[r/'src/config/runtime_provider.cpp']+behavioral_state_sources+settings_base))
        # F319-P111-001: the selector fixture is executable release QA so raw
        # malformed source classification stays pinned for GetSetting/GetGameInfo.
        breadth.append(('selector_builder_strict',[r/'tests/unit/test_selector_policy.cpp',r/'src/handlers/runtime_settings_routes.cpp',r/'src/handlers/game_info_routes.cpp',r/'src/handlers/selector_pipeline.cpp',r/'src/runtime/utc_time.cpp']+runtime_config_sources+[r/'src/config/runtime_provider.cpp']+behavioral_state_sources+settings_base))
        for name,srcs in breadth:
            exe=td/name
            subprocess.run(breadth_common+[str(x) for x in srcs]+['-o',str(exe)],cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)


# F1415-003: completed client owners are reaped incrementally by the accept
# owner under the service mutex. Only signaled worker handles are destroyed;
# live workers remain in the vector for the existing shutdown bulk-stop path.
release_server_win='\n'.join((
 (r/'src/runtime/release_server_worker.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/runtime/release_server_acceptor.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/runtime/release_server_lifecycle.cpp').read_text(encoding='utf-8',errors='replace'),
 (r/'src/runtime/release_server_io_internal.hpp').read_text(encoding='utf-8',errors='replace'),
))
required_owner_reap=[
 'size_t reclaim_completed_client_threads_locked(',
 'detail::WaitForSingleObject(handle, 0u) != detail::kWaitObject0',
 'detail::destroy_jthread(thread)',
 'service->client_records.end = reinterpret_cast<uintptr_t>(begin + size);',
 '(void)reclaim_completed_client_threads_locked(service);',
 'case 7u: return detail::release_acceptance_counter(7u);',
]
if any(x not in release_server_win for x in required_owner_reap):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1415-003 completed-owner reaper drift'); sys.exit(1)
reap_start=release_server_win.find('size_t reclaim_completed_client_threads_locked(')
reap_end=release_server_win.find('enum class AppendClientResult',reap_start)
reap=release_server_win[reap_start:reap_end] if reap_start>=0 and reap_end>reap_start else ''
if not reap or reap.find('detail::WaitForSingleObject(handle, 0u)') > reap.find('detail::destroy_jthread(thread)'):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1415-003 destroys owner before completion proof'); sys.exit(1)


# HOTFIX10 combined runtime hardening: F1598-001/F1599-001/F1601-001/F1602-001.
# These source guards pin the Windows-only ownership/concurrency policy while the
# F1599 full-object zeroization is also executable-covered in the existing F1485
# socket-exchange block below.
release_acceptor=(r/'src/runtime/release_server_acceptor.cpp').read_text(encoding='utf-8',errors='replace')
release_internal=(r/'src/runtime/release_server_io_internal.hpp').read_text(encoding='utf-8',errors='replace')
release_public=(r/'include/preloader_l/runtime/release_server_io.hpp').read_text(encoding='utf-8',errors='replace')
event_queue_src=(r/'src/runtime/event_queue.cpp').read_text(encoding='utf-8',errors='replace')
exchange_src=(r/'src/protocol/socket_exchange.cpp').read_text(encoding='utf-8',errors='replace')

required_f1598=[
 'release_client_hard_cap = 128u',
 'if (!release_client_admission_allowed(size)) return AppendClientResult::capacity_rejected;',
 'if (grown > release_client_hard_cap) grown = release_client_hard_cap;',
 'if (appended != AppendClientResult::started)',
 'TraceEvent::client_capacity_rejected',
 'server_trace(context, TraceEvent::failure, 0x5d02u);',
]
if any(x not in (release_public+'\n'+release_acceptor) for x in required_f1598):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1598 worker admission hard-cap drift'); sys.exit(1)
if 'semantic_terminal(context, 0x5d02u)' in release_acceptor:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1598 per-client resource pressure is process-terminal again'); sys.exit(1)

required_f1599=[
 'void secure_zero_bytes(void* memory, size_t size) noexcept',
 'auto* bytes = static_cast<volatile uint8_t*>(memory);',
 'secure_zero_bytes(s, sizeof(*s));',
]
if any(x not in exchange_src for x in required_f1599):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1599 ExchangeState zeroization drift'); sys.exit(1)

required_f1601=[
 'atomic_load_u32(&mutex.owner_thread_id)',
 'atomic_load_u32(&mutex.recursion_count)',
 'atomic_fetch_add_u32(&mutex.recursion_count, 1)',
 'atomic_exchange_u32(&mutex.recursion_count, 1u)',
 'atomic_exchange_u32(&mutex.owner_thread_id, tid)',
 'atomic_fetch_add_u32(&mutex.recursion_count, -1)',
 'atomic_exchange_u32(&mutex.owner_thread_id, 0xffffffffu)',
 'atomic_exchange_u32(&mutex.recursion_count, 0u)',
]
if any(x not in release_internal for x in required_f1601):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1601 recursive mutex atomic publication drift'); sys.exit(1)
mutex_start=release_internal.find('inline bool lock_service_mutex(')
mutex_end=release_internal.find('inline model_abi::ModelJthread24* client_begin',mutex_start)
mutex_body=release_internal[mutex_start:mutex_end] if mutex_start>=0 and mutex_end>mutex_start else ''
if not mutex_body or 'mutex.owner_thread_id == tid' in mutex_body or '++mutex.recursion_count' in mutex_body or '--mutex.recursion_count' in mutex_body:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1601 plain pre-lock owner/recursion access returned'); sys.exit(1)

required_f1602=[
 'void* g_outer_dispatch_lock_word=nullptr;',
 'AcquireSRWLockExclusive(&g_outer_dispatch_lock_word);',
 'ReleaseSRWLockExclusive(&g_outer_dispatch_lock_word);',
]
if any(x not in event_queue_src for x in required_f1602):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1602 blocking outer-dispatch synchronization drift'); sys.exit(1)
if '_InterlockedCompareExchange(&g_outer_dispatch_lock' in event_queue_src:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1602 tight outer-dispatch spinlock returned'); sys.exit(1)


# F1421-002 + HF17/F1482-001: after WSAStartup succeeds, the native adapter
# validates exact Winsock 2.2. A mismatch first attempts WSACleanup. If that
# cleanup fails, return a distinct internal sentinel so SocketRuntime retains
# ownership and the existing stop/shutdown retry path can balance WSAStartup.
winsock_native=(r/'src/net/winsock_native.cpp').read_text(encoding='utf-8',errors='replace')
socket_runtime=(r/'src/net/socket_runtime.cpp').read_text(encoding='utf-8',errors='replace')
required_winsock_version=[
 'const int result = WSAStartup(version, data);',
 'if (result != 0) return result;',
 'winsock_negotiated_version_supported(negotiated)',
 'if (WSACleanup() != 0)',
 'return preloader_l::net::winsock_version_cleanup_retry_required;',
 'return preloader_l::net::winsock_version_not_supported;',
]
if any(x not in winsock_native for x in required_winsock_version):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1421-002/F1482 negotiated Winsock gate drift'); sys.exit(1)
check_pos=winsock_native.find('winsock_negotiated_version_supported(negotiated)')
cleanup_pos=winsock_native.find('if (WSACleanup() != 0)',check_pos)
retry_pos=winsock_native.find('return preloader_l::net::winsock_version_cleanup_retry_required;',cleanup_pos)
return_pos=winsock_native.find('return preloader_l::net::winsock_version_not_supported;',retry_pos)
if min(check_pos,cleanup_pos,retry_pos,return_pos)<0 or not (check_pos < cleanup_pos < retry_pos < return_pos):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1421-002/F1482 mismatch cleanup ordering drift'); sys.exit(1)
required_retry_state=[
 'if (startup_result == winsock_version_cleanup_retry_required) {',
 'runtime->started = true;',
 '++runtime->startup_count;',
 'runtime->api = nullptr;',
]
if any(x not in socket_runtime for x in required_retry_state):
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1482 retained cleanup ownership state drift'); sys.exit(1)
if run_heavy_case('preloader_l_winsock_version_policy'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_winsock_version_policy_') as td:
        td=Path(td); exe=td/'winsock_version_policy'
        subprocess.run(common+[str(r/'tests/unit/test_winsock_version_policy.cpp'),'-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)


# AUDIT-HOTFIX14 retained convergence for F1429/F1436/F1445 plus the
# AUDIT-HOTFIX15 F1423 consumer-policy correction. These cases execute the
# exact affected source surfaces rather than relying on static token checks.
if run_heavy_case('preloader_l_hf14_convergence'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_hf14_convergence_') as td:
        td=Path(td)
        def hf14_run(name,srcs,extra=None):
            exe=td/name
            subprocess.run(common+(extra or [])+[str(x) for x in srcs]+['-o',str(exe)],cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)
        codec=[r/'src/codec/session_crypto.cpp',r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp']
        hf14_run('hf14_behavioral_convergence',[r/'tests/unit/test_challenge_compatibility.cpp']+codec)
        hf14_run('hf15_f1423_consumer_seed',[r/'tests/unit/test_session_seed_policy.cpp']+codec)
        handshake=[r/'src/protocol/challenge_handshake.cpp']+codec+[*request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp']+runtime_config_sources+[r/'src/config/settings_values.cpp']+behavioral_state_sources
        hf14_run('hf14_handshake',[r/'tests/unit/test_handshake.cpp']+handshake,['-Wno-unused-function'])
        settings_base=[r/'src/handlers/setting_value_route.cpp',r/'src/handlers/settings_snapshot_route.cpp',r/'src/handlers/connection_state_route.cpp',r/'src/debug/handlers/settings_query.cpp',*request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp']
        hf14_run('hf14_get_settings',[r/'tests/unit/test_settings_snapshot.cpp']+settings_base)
        routes=[r/'src/handlers/store_routes.cpp',*request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp']+runtime_config_sources+[r/'src/config/runtime_provider.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',r/'src/handlers/selector_pipeline.cpp']+behavioral_state_sources
        hf14_run('hf14_entitlement_grammar',[r/'tests/unit/test_entitlement_grammar.cpp']+routes,['-Wno-unused-function','-Wno-misleading-indentation','-ffunction-sections','-fdata-sections','-Wl,--gc-sections'])



# Audit2 HF10 portable remediation: repair and register previously failing
# product units and bind parser/depth postconditions to executable QA.
if run_heavy_case('preloader_l_s0371_settings_provider'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_s0371_settings_provider_') as td:
        td=Path(td); exe=td/'settings_provider'
        subprocess.run(common+[
            str(r/'tests/unit/test_settings_provider.cpp'),
            str(r/'src/config/settings_provider.cpp'),
            str(r/'src/config/settings_values.cpp'),
            '-o',str(exe)
        ],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

if run_heavy_case('preloader_l_s0371_runtime_configuration_parser'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_s0371_runtime_configuration_parser_') as td:
        td=Path(td); exe=td/'runtime_configuration_parser'
        subprocess.run(common+['-Wno-unused-function',
            str(r/'tests/unit/test_runtime_configuration_parser.cpp'),
            *[str(x) for x in runtime_config_sources],
            str(r/'src/config/settings_values.cpp'),
            *[str(x) for x in behavioral_state_sources],
            '-o',str(exe)
        ],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

if run_heavy_case('preloader_l_s0446_socket_endpoint'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_s0446_socket_endpoint_') as td:
        td=Path(td); exe=td/'socket_endpoint'
        subprocess.run(common+[
            str(r/'tests/unit/test_socket_endpoint.cpp'),
            str(r/'src/net/socket_runtime.cpp'),str(r/'src/net/socket_listener.cpp'),
            str(r/'src/net/socket_endpoint.cpp'),str(r/'src/net/socket_transport.cpp'),
            '-o',str(exe)
        ],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

value_normalize=(r/'src/xml/value_codec_normalize.cpp').read_text(encoding='utf-8',errors='replace')
if 'bool equal = true;' not in value_normalize or 'continue decoding the complete source' not in value_normalize:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F323-S0381-001 full-source validity drift'); sys.exit(1)

# The TinyXML2-owned parser now guards nesting; old scanner-local budget
# statements must never be resurrected as a parallel XML engine.
engine_depth=(r/'third_party/tinyxml2/tinyxml2.h').read_text(encoding='utf-8',errors='replace')
if 'TINYXML2_MAX_ELEMENT_DEPTH = 500' not in engine_depth or 'parse_document(' not in store_document:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F323-S0689-001 active TinyXML2 depth guard missing'); sys.exit(1)
if run_heavy_case('preloader_l_s0689_store_depth'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_s0689_store_depth_') as td:
        td=Path(td); exe=td/'store_depth'
        store_sources=[
            r/'src/config/store_runtime.cpp',r/'src/config/store_parser.cpp',r/'third_party/tinyxml2/tinyxml2.cpp',
            r/'src/config/store_materialization.cpp',r/'src/config/store_values.cpp',
            r/'src/config/store_payload.cpp',
        ]
        subprocess.run(common+[str(r/'tests/unit/test_store_document_boundary.cpp')]+[str(x) for x in store_sources]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

# HF18 audit-debt closure / F1451-001: the two semantics-only implementation
# units are explicitly test support, never production inputs, and must retain
# executable coverage rather than existing as production-looking dead TUs.
if run_heavy_case('preloader_l_f1451_support'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_f1451_support_') as td:
        td=Path(td)
        def f1451_run(name, srcs):
            exe=td/name
            subprocess.run(common+[str(x) for x in srcs]+['-o',str(exe)],cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)
        exception_support=r/'tests/support/exception_policy.cpp'
        allocator=[r/'src/runtime/memory_allocator.cpp',r/'src/runtime/ptd_runtime.cpp']
        f1451_run('exception_types',[r/'tests/unit/test_exception_types.cpp',exception_support]+allocator)
        f1451_run('allocate_or_throw',[r/'tests/unit/test_allocate_or_throw.cpp',exception_support]+allocator)
        f1451_run('unwind_cleanup',[r/'tests/unit/test_unwind_cleanup.cpp',exception_support]+allocator)

# HF18 Stage03 / F1499-001 + HOTFIX14 F1571/F1573/F1579: the normalized
# request regression is executable coverage for document cardinality, duplicate
# attribute rejection and raw-source UTF-8/XML-character codec validation.
if run_heavy_case('preloader_l_f1499_direct_trailing'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_f1499_direct_trailing_') as td:
        td=Path(td); exe=td/'f1499_direct_trailing'
        subprocess.run(common+[
            str(r/'tests/unit/test_request.cpp'),
            *[str(x) for x in request_sources],
            str(r/'src/xml/lsx_framing.cpp'),
            *[str(x) for x in value_codec_sources],
            str(r/'src/xml/attribute_parser.cpp'),
            '-o',str(exe)
        ],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)


# HF18 Stage02 / F1485-001: TCP recv segmentation must not define logical
# protocol frame boundaries. Exercise split/coalesced plaintext handshakes and
# split/coalesced encrypted wire frames against the shipping socket-exchange TU.
if run_heavy_case('preloader_l_f1485_stream_reassembly'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_f1485_stream_reassembly_') as td:
        td=Path(td); exe=td/'f1485_stream_reassembly'
        # F1485 exercises transport framing, not the ShowIGO route. The current route's
        # transaction TU has a new link-time prepared-delivery dependency, so
        # provide a non-reachable test-local stub while the dedicated route
        # atomicity regression links the real handler/event-queue implementation.
        showigo_stub=td/'f1485_showigo_delivery_stub.cpp'
        showigo_stub.write_text(r'''#include "preloader_l/handlers/integration_routes.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus prepare_show_igo_delivery(const preloader_l::xml::RequestView&,char*,size_t,size_t* written,preloader_l::runtime::PreparedQueuedEvent** candidate) noexcept {
    if(written)*written=0;
    if(candidate)*candidate=nullptr;
    return HandlerStatus::wrong_command;
}
} }
''',encoding='utf-8')
        # Stream-framing-only fixture: production social paths are unreachable
        # in this scoped test; dedicated social fixture links the real owner.
        social_stub=td/'f1485_local_social_stub.cpp'
        social_stub.write_text(r'''#include "preloader_l/runtime/local_social_host.hpp"
#include "preloader_l/handlers/social_mutation_routes.hpp"
namespace preloader_l { namespace runtime {
bool bind_host_social_profile(LocalSocialSession* out) noexcept {
    if(out)*out=LocalSocialSession{};
    return false;
}
LocalSocialResult prepare_local_social_invites(const LocalSocialSession&,uint64_t,const uint64_t*,size_t,const char*,size_t,LocalSocialPending*) noexcept { return LocalSocialResult::no_principal; }
LocalSocialResult commit_local_social_mutation(const LocalSocialSession&, LocalSocialPending*) noexcept {
    return LocalSocialResult::no_principal;
}
void rollback_local_social_mutation(LocalSocialPending*) noexcept {}
} namespace handlers {
bool prepare_local_social_socket_request(const xml::RequestView&,
    const runtime::LocalSocialSession&,runtime::LocalSocialPending*) noexcept {return false;}
HandlerStatus render_local_social_success(const xml::RequestView&,char*,size_t,size_t*) noexcept {
    return HandlerStatus::unsupported;
}
} }
''',encoding='utf-8')
        f1485_sources=[
            r/'tests/unit/test_stream_reassembly.cpp',showigo_stub,social_stub,
            r/'src/protocol/socket_exchange.cpp',r/'src/protocol/socket_exchange_wire.cpp',r/'src/protocol/socket_exchange_frame_step.cpp',
            r/'src/protocol/socket_exchange_transaction.cpp',r/'src/protocol/socket_exchange_transaction_delivery.cpp',r/'src/protocol/socket_exchange_write_completion.cpp',r/'src/protocol/challenge_handshake.cpp',r/'src/net/socket_transport.cpp',
            r/'src/codec/session_crypto.cpp',r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',
        ]+runtime_config_sources+[
            r/'src/config/settings_values.cpp',
        ]+behavioral_state_sources
        subprocess.run(common+['-Wno-unused-function']+[str(x) for x in f1485_sources]+['-o',str(exe)],cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)


# HOTFIX12 / F1610-001: all 13 active-state unit programs are executable
# behavioral coverage, not syntax-only fixtures. They intentionally validate
# the current policy split: unresolved/current-production consistency-open are
# zero while historical identifiers remain source-compatible and curio /
# integrated / runtime-active masks retain their documented roles.
if run_heavy_case('preloader_l_f1610_active_state_13'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_f1610_active_state_13_') as td:
        td=Path(td)
        active_state_tests=[
            r/'tests/unit/test_service_status.cpp',
            r/'tests/unit/test_store_aux_status.cpp',
            r/'tests/unit/test_allocator_closure.cpp',
            r/'tests/unit/test_status_aliases.cpp',
            r/'tests/unit/test_integration_masks.cpp',
            r/'tests/unit/test_store_payload_status.cpp',
            r/'tests/unit/test_hook_service_status.cpp',
            r/'tests/unit/test_optional_callback_status.cpp',
            r/'tests/unit/test_optional_host_status.cpp',
            r/'tests/unit/test_optional_scanner_status.cpp',
            r/'tests/unit/test_relocation_closure.cpp',
            r/'tests/unit/test_thread_trap_closure.cpp',
        ]
        if len(active_state_tests) != 12:
            print(f'BEHAVIORAL_REGRESSIONS: FAIL - F1610 active-state test count drift ({len(active_state_tests)} != 12)'); sys.exit(1)
        active_state_support=[r/'src/debug/runtime/runtime_status.cpp',r/'src/debug/runtime/consistency_closure.cpp']
        for test_src in active_state_tests:
            exe=td/test_src.stem
            subprocess.run(common+['-Wno-error=volatile']+[str(test_src)]+[str(x) for x in active_state_support]+['-o',str(exe)],cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)


# Audit01 F319-P031-001 + F319-P078-001: the four fixtures proven stale by
# the audit are executable release-QA assets, not syntax-only inventory. Keep
# their current runtime/config/settings/session contracts pinned under -Werror.
# Fail closed if the native-only test sources or SDK binding vanish. Runtime
# assertions for these cases are NOT represented by the Linux behavioral PASS.
for native_case in ('test_language_policy.cpp','test_unicode_sibling_paths.cpp'):
    if not (r/'tests/unit'/native_case).is_file():
        print('BEHAVIORAL_REGRESSIONS: FAIL - mandatory Windows case missing:',native_case); sys.exit(1)
for win_src in ('src/config/process_configuration.cpp','src/runtime/avatar_path.cpp'):
    if '#include <windows.h>' not in (r/win_src).read_text(encoding='utf-8'):
        print('BEHAVIORAL_REGRESSIONS: FAIL - native Win32 binding missing:',win_src); sys.exit(1)
print('WINDOWS_PATH_POLICY_NATIVE: REQUIRED (not asserted by POSIX QA)')

if run_heavy_case('preloader_l_audit01_oracle_closure'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_audit01_oracle_closure_') as td:
        td=Path(td)
        def audit01_run(name, srcs, extra=None, objects=None):
            exe=td/name
            subprocess.run(common+(extra or [])+[str(x) for x in srcs]+[str(x) for x in (objects or [])]+['-o',str(exe)],cwd=r,check=True)
            subprocess.run([str(exe)],cwd=r,check=True)

        # P031 entry/runtime snapshot fixture: unresolved/current consistency
        # state is closed in the authoritative runtime ledger.
        audit01_run('audit01_entry_flow',[
            r/'tests/unit/test_entry_flow.cpp',
            r/'src/runtime/entry_flow.cpp',
            r/'src/debug/runtime/runtime_status.cpp',
            r/'src/debug/runtime/consistency_closure.cpp',
            r/'src/debug/runtime/event_trace.cpp',
            r/'src/debug/runtime/trace_format.cpp',
        ],['-DPRELOADER_L_TRACE_DIAGNOSTICS=1'])

        # P078 config/settings fixtures: exact Online true/false policy and
        # the complete current SettingKey enum must compile and execute.
        portable_stub=td/'audit01_platform_stubs.cpp'
        portable_stub.write_text(r'''#include <cstddef>
#include <cstdlib>
extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*, unsigned long, std::size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*, unsigned long, void* p){ std::free(p); return 1; }
namespace preloader_l { namespace runtime {
bool normalized_avatar_path(char*,std::size_t,std::size_t* written) noexcept { if(written)*written=0; return false; }
int query_image_size_bucket(int width) noexcept { return width/208; }
} }
''',encoding='utf-8')

        runtime_config_route_sources=[
            r/'tests/unit/test_runtime_configuration_routes.cpp',
            *runtime_config_sources,r/'src/config/runtime_provider.cpp',r/'src/config/settings_provider.cpp',r/'src/config/settings_values.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',
            r/'src/handlers/store_routes.cpp',*achievement_route_sources,r/'src/handlers/image_route.cpp',
            r/'src/handlers/game_info_routes.cpp',r/'src/handlers/profile_routes.cpp',r/'src/handlers/selector_pipeline.cpp',
            r/'src/router/route_lookup.cpp',r/'src/router/route_inventory.cpp',r/'src/hash/fnv_hash.cpp',
            *behavioral_state_sources,r/'src/runtime/utc_time.cpp',r/'src/runtime/achievement_watcher.cpp',r/'src/runtime/event_queue.cpp',
            portable_stub,
        ]
        audit01_run('audit01_runtime_configuration_routes',runtime_config_route_sources,
                    ['-Wno-unused-function','-Wno-misleading-indentation'])

        audit01_run('audit01_settings_query',[
            r/'tests/unit/test_settings_query.cpp',
            r/'src/handlers/setting_value_route.cpp',r/'src/handlers/settings_snapshot_route.cpp',r/'src/handlers/connection_state_route.cpp',
            r/'src/debug/handlers/settings_query.cpp',*request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',
            *value_codec_sources,r/'src/xml/attribute_parser.cpp',r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',
            r/'src/hash/fnv_hash.cpp',r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',
        ])

        # Audit01 F323-S0021-001: FeatureDesc and FeatureFlag retain independent
        # encounter order under every two-by-two interleaving permutation.
        audit01_run('audit01_store_feature_interleaving',[
            r/'tests/unit/test_store_feature_interleaving.cpp',
            r/'src/config/store_runtime.cpp',r/'src/config/store_materialization.cpp',
            r/'third_party/tinyxml2/tinyxml2.cpp',r/'src/config/store_values.cpp',
            r/'src/config/store_parser.cpp',
            r/'src/config/settings_values.cpp',
        ])

        # P085 store parser taxonomy: malformed surrogate numeric entities are
        # invalid XML and cannot replace the previously committed snapshot.
        audit01_run('audit01_store_surrogate_taxonomy',[
            r/'tests/unit/test_store_snapshot.cpp',
            r/'src/config/store_runtime.cpp',r/'src/config/store_materialization.cpp',
            r/'third_party/tinyxml2/tinyxml2.cpp',r/'src/config/store_values.cpp',
            r/'src/config/store_parser.cpp',
            r/'src/config/settings_values.cpp',
        ])

        # P031 session-consistency fixture needs the shipping transaction TU's
        # Windows HeapAlloc branch. Compile only that TU with a tiny test-owned
        # Win32 import shim; all other production sources remain portable.
        transaction_obj=td/'socket_exchange_transaction.o'
        subprocess.run(common+[
            '-D_WIN32','-D__declspec(x)=','-D__stdcall=',
            '-c',str(r/'src/protocol/socket_exchange_transaction.cpp'),'-o',str(transaction_obj)
        ],cwd=r,check=True)
        event_queue_obj=td/'event_queue_windows_heap.o'
        subprocess.run(common+[
            '-Wno-misleading-indentation',
            '-D_WIN32','-D__declspec(x)=','-D__stdcall=',
            '-c',str(r/'src/runtime/event_queue.cpp'),'-o',str(event_queue_obj)
        ],cwd=r,check=True)
        session_sources=[
            r/'tests/unit/test_session_continuation.cpp',
            r/'src/protocol/socket_exchange.cpp',r/'src/protocol/socket_exchange_wire.cpp',r/'src/protocol/socket_exchange_frame_step.cpp',r/'src/protocol/challenge_handshake.cpp',
            r/'src/net/socket_transport.cpp',r/'src/codec/session_crypto.cpp',r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',
            r/'src/router/route_lookup.cpp',r/'src/router/route_inventory.cpp',r/'src/core/request_dispatch.cpp',r/'src/hash/fnv_hash.cpp',
            r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',r/'src/config/runtime_provider.cpp',*runtime_config_sources,
            *behavioral_state_sources,*transient_owner_sources,r/'src/runtime/event_queue.cpp',r/'src/runtime/utc_time.cpp',
            r/'src/runtime/auth_code_service.cpp',r/'src/runtime/achievement_watcher.cpp',
            r/'src/handlers/progressive_installation_route.cpp',r/'src/handlers/permission_route.cpp',r/'src/handlers/friends_route.cpp',
            r/'src/handlers/integration_routes.cpp',r/'src/handlers/group_info_route.cpp',r/'src/handlers/utc_time_route.cpp',r/'src/handlers/query_presence_route.cpp',r/'src/handlers/social_mutation_routes.cpp',r/'src/handlers/social_mutation_socket.cpp',r/'src/handlers/local_social_list_route.cpp',r/'src/runtime/local_social_state.cpp',r/'src/runtime/local_social_host.cpp',r/'src/handlers/config_route.cpp',r/'src/handlers/setting_value_route.cpp',
            r/'src/handlers/settings_snapshot_route.cpp',r/'src/handlers/handler_registry.cpp',r/'src/handlers/connection_state_route.cpp',
            r/'src/handlers/connection_state_binding.cpp',r/'src/handlers/runtime_settings_routes.cpp',r/'src/handlers/selector_pipeline.cpp',
            r/'src/handlers/game_info_routes.cpp',r/'src/handlers/profile_routes.cpp',r/'src/handlers/auth_routes.cpp',
            r/'src/handlers/store_routes.cpp',*achievement_route_sources,r/'src/handlers/image_route.cpp',
            # Exact current split: Win32 transaction owner plus shared delivery/write completion.
            r/'src/protocol/socket_exchange_transaction_delivery.cpp',
            r/'src/protocol/socket_exchange_write_completion.cpp',
            portable_stub,
        ]
        audit01_run('hf08_local_social_live_contract',[
            r/'tests/unit/test_local_social_live_contract.cpp',
            r/'src/runtime/local_social_state.cpp',r/'src/handlers/local_social_list_route.cpp',
            r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources])
        audit01_run('hf08_local_social_store_model',[
            r/'tests/unit/test_local_social_state.cpp',r/'src/runtime/local_social_state.cpp'])
        audit01_run('hf08_local_social_read_consistency',[
            r/'tests/unit/test_local_social_snapshot.cpp',r/'src/runtime/local_social_state.cpp'])
        audit01_run('hf08_local_social_committed_read_gate',[
            r/'tests/unit/test_local_social_reader_policy.cpp',r/'src/runtime/local_social_state.cpp'])
        # W06: real shipping-source host-profile identity and socket-stage
        # validation. Config2 mock is test-local; no remote login is asserted.
        audit01_run('hf08_w06_host_profile_binding',[
            r/'tests/unit/test_local_social_host_binding.cpp',
            r/'src/runtime/local_social_host.cpp',r/'src/runtime/local_social_state.cpp'])
        audit01_run('hf08_w06_local_socket_stage',[
            r/'tests/unit/test_local_social_socket_prepare.cpp',
            r/'src/handlers/social_mutation_socket.cpp',
            r/'src/runtime/local_social_host.cpp',r/'src/runtime/local_social_state.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',
            r/'src/xml/response_builder.cpp',*value_codec_sources,
            r/'src/xml/attribute_parser.cpp'])
        audit01_run('hf08_w05_social_delivery_model_only',[
            r/'tests/unit/test_local_social_delivery_model.cpp',
            r/'tests/support/local_social_delivery_model.cpp',
            r/'src/runtime/local_social_state.cpp'],
            ['-I'+str(r/'tests/support')])
        audit01_run('hf08_social_denied_without_local_principal',[
            r/'tests/unit/test_social_denied_handlers.cpp',
            r/'src/handlers/social_mutation_routes.cpp',r/'src/xml/response_builder.cpp',
            r/'src/xml/attribute_parser.cpp',*value_codec_sources,
            *request_sources,r/'src/xml/lsx_framing.cpp',
            r/'src/router/route_inventory.cpp',r/'src/router/route_lookup.cpp',r/'src/hash/fnv_hash.cpp'])
        audit01_run('route_response_contracts',[r/'tests/unit/test_route_response_contracts.cpp']+session_sources[1:],
                    ['-Wno-unused-function','-Wno-misleading-indentation'],[transaction_obj])

        audit01_run('audit01_session_consistency',session_sources,
                    ['-Wno-unused-function','-Wno-misleading-indentation'],[transaction_obj])

        # ShowIGO event publication is a socket-delivery
        # transaction. Failure after candidate preparation must not leak DOWN,
        # and a pre-existing FIFO head must retain exact ordering on rollback.
        showigo_atomicity_sources=[r/'tests/unit/test_show_igo_socket_atomicity.cpp']+[
            x for x in session_sources[1:]
            if x not in (r/'src/runtime/event_queue.cpp', portable_stub)
        ]
        audit01_run('cervrax_showigo_socket_atomicity',showigo_atomicity_sources,
                    ['-Wno-unused-function','-Wno-misleading-indentation'],[transaction_obj,event_queue_obj])

        # Delivery-atomicity contract: event reservations release the process-wide
        # delivery gate before socket I/O, provisional candidates remain
        # invisible until commit, rollback preserves FIFO position, and a
        # monotonic idle-progress budget terminates endless would_block sends.
        audit01_run('melotrix_f02_delivery_progress',[
            r/'tests/unit/test_delivery_progress.cpp',
            r/'src/protocol/socket_exchange_wire.cpp',
            r/'src/xml/lsx_framing.cpp',
            r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp',
        ],['-Wno-misleading-indentation'],[event_queue_obj])

        # HF11/F323-S0439-001: receive idle and incomplete-frame lifetime
        # budgets terminate endless would_block/slow-trickle sessions.
        audit01_run('ranquel_receive_progress',[
            r/'tests/unit/test_receive_progress.cpp',
            r/'src/protocol/socket_exchange_wire.cpp',
            r/'src/xml/lsx_framing.cpp',
            r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp',
        ],['-Wno-misleading-indentation'])

        # Execute the full dispatcher/registry path for the compatibility
        # five explicit compatibility routes and StartDownload's route-local
        # success contract. Reuse the complete shipping dependency closure so
        # registry additions cannot pass through syntax-only coverage.
        hf02_route_sources=[r/'tests/unit/test_compatibility_routes.cpp']+session_sources[1:]
        audit01_run('hf02_compatibility_routes',hf02_route_sources,
                    ['-Wno-unused-function','-Wno-misleading-indentation'],[transaction_obj])


# AUDIT-HOTFIX16 F1427-001: true unknown command is a no-response router miss,
# not the registered generic-unimplemented ErrorSuccess class.
dispatch_src=(r/'src/core/request_dispatch.cpp').read_text(encoding='utf-8',errors='replace')
socket_src=(r/'src/protocol/socket_exchange.cpp').read_text(encoding='utf-8',errors='replace')
if 'result.status = DispatchStatus::unknown_route;' not in dispatch_src:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1427 true-unknown status missing'); sys.exit(1)
if 'Feature is not implemented' in dispatch_src:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1427 synthetic unknown ErrorSuccess remains'); sys.exit(1)
if 'transaction.dispatch_status == preloader_l::core::DispatchStatus::unknown_route' not in socket_src:
    print('BEHAVIORAL_REGRESSIONS: FAIL - F1427 persistent-session unknown continuation missing'); sys.exit(1)


print(f'BEHAVIORAL_REGRESSIONS_MH_SHARD: PASS ({_shard_index + 1}/{_AUTHORITATIVE_SHARD_COUNT}; behavior + MinHook architecture reset)')

# HF12 / Audit2 F323-S0470-001 + F323-S0718-001: recoverable debug
# capacity preserves the pending frame/counters, and trace payload publication
# is synchronized for the entire reused-slot write/copy interval.
hf12_debug_session=(r/'src/debug/session_api.cpp').read_text(encoding='utf-8',errors='replace')
hf12_socket_session=(r/'src/debug/net/socket_session.cpp').read_text(encoding='utf-8',errors='replace')
hf12_trace=(r/'src/debug/runtime/event_trace.cpp').read_text(encoding='utf-8',errors='replace')
for marker in [
    'const auto measured = preloader_l::core::dispatch_plaintext_request(',
    'measured.written >= response_capacity',
    'if (result.status == preloader_l::core::DispatchStatus::capacity)',
    'return SocketPumpStatus::capacity;',
]:
    if marker not in hf12_debug_session+'\n'+hf12_socket_session:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF12 debug capacity retry contract drift'); sys.exit(1)
if hf12_debug_session.find('measured.written >= response_capacity') > hf12_debug_session.find('consume_ingress_frame(&g_ingress'):
    print('BEHAVIORAL_REGRESSIONS: FAIL - HF12 debug frame consumed before capacity gate'); sys.exit(1)
for marker in ['uint32_t ownership;', 'lock_slot(&slot.ownership);',
               'try_lock_slot(&slot.ownership)', 'unlock_slot(&slot.ownership);']:
    if marker not in hf12_trace:
        print('BEHAVIORAL_REGRESSIONS: FAIL - HF12 trace slot ownership contract drift'); sys.exit(1)

if run_heavy_case('hf12_debug_capacity_trace'):
    with tempfile.TemporaryDirectory(prefix='preloader_l_hf12_debug_capacity_trace_') as td:
        td=Path(td)
        stub=td/'platform_stubs.cpp'
        stub.write_text(r'''#include <cstddef>
#include <cstdlib>
extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*, unsigned long, std::size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*, unsigned long, void* p){ std::free(p); return 1; }
namespace preloader_l { namespace runtime {
bool normalized_avatar_path(char*,std::size_t,std::size_t* written) noexcept { if(written)*written=0; return false; }
int query_image_size_bucket(int width) noexcept { return width/208; }
} }
''',encoding='utf-8')
        socket_exe=td/'test_socket_session'
        socket_sources=[
            r/'tests/unit/test_socket_session.cpp',
            r/'src/debug/net/session_state.cpp',r/'src/debug/net/receive_buffer.cpp',r/'src/debug/net/frame_ingress.cpp',r/'src/debug/net/socket_session.cpp',r/'src/net/socket_transport.cpp',
            r/'src/core/request_dispatch.cpp',
            *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',
            r/'src/router/route_lookup.cpp',r/'src/router/route_inventory.cpp',r/'src/hash/fnv_hash.cpp',
            *runtime_config_sources,r/'src/config/runtime_provider.cpp',r/'src/config/settings_provider.cpp',r/'src/config/settings_values.cpp',
            *behavioral_state_sources,r/'src/runtime/event_queue.cpp',r/'src/runtime/utc_time.cpp',r/'src/runtime/auth_code_service.cpp',r/'src/runtime/achievement_watcher.cpp',
            r/'src/handlers/progressive_installation_route.cpp',r/'src/handlers/permission_route.cpp',r/'src/handlers/friends_route.cpp',
            r/'src/handlers/integration_routes.cpp',r/'src/handlers/group_info_route.cpp',r/'src/handlers/utc_time_route.cpp',r/'src/handlers/query_presence_route.cpp',r/'src/handlers/social_mutation_routes.cpp',r/'src/handlers/social_mutation_socket.cpp',r/'src/handlers/local_social_list_route.cpp',r/'src/runtime/local_social_state.cpp',r/'src/runtime/local_social_host.cpp',r/'src/handlers/config_route.cpp',r/'src/handlers/setting_value_route.cpp',
            r/'src/handlers/settings_snapshot_route.cpp',r/'src/handlers/handler_registry.cpp',r/'src/handlers/connection_state_route.cpp',
            r/'src/handlers/connection_state_binding.cpp',r/'src/handlers/runtime_settings_routes.cpp',r/'src/handlers/selector_pipeline.cpp',
            r/'src/handlers/game_info_routes.cpp',r/'src/handlers/profile_routes.cpp',r/'src/handlers/auth_routes.cpp',r/'src/handlers/store_routes.cpp',
            *achievement_route_sources,r/'src/handlers/image_route.cpp',stub,
        ]
        subprocess.run(common+['-Wno-unused-function','-Wno-misleading-indentation']+[str(x) for x in socket_sources]+['-o',str(socket_exe)],cwd=r,check=True)
        subprocess.run([str(socket_exe)],cwd=r,check=True)

        trace_exe=td/'test_trace'
        subprocess.run(common+['-pthread','-DPRELOADER_L_TRACE_DIAGNOSTICS=1',
            str(r/'tests/unit/test_trace.cpp'),str(r/'src/debug/runtime/event_trace.cpp'),'-o',str(trace_exe)],cwd=r,check=True)
        subprocess.run([str(trace_exe)],cwd=r,check=True)
