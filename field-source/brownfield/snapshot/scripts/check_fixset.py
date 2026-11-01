#!/usr/bin/env python3
from pathlib import Path
import sys,json
r=Path(__file__).resolve().parents[1]
e=[]
def need(v,n):
    if not v:e.append(n)
win=(r/'src/config/process_configuration.cpp').read_text(encoding='utf-8',errors='replace')
hdr=(r/'include/preloader_l/config/process_configuration.hpp').read_text(encoding='utf-8',errors='replace')
routes=(r/'src/handlers/auth_routes.cpp').read_text(encoding='utf-8',errors='replace')
auth=(r/'src/runtime/auth_code_service.cpp').read_text(encoding='utf-8',errors='replace')
auth_h=(r/'include/preloader_l/runtime/auth_code_service.hpp').read_text(encoding='utf-8',errors='replace')
avatar=(r/'src/runtime/avatar_path.cpp').read_text(encoding='utf-8',errors='replace')
env_path=r/'src/platform/environment_policy.cpp'
env=env_path.read_text(encoding='utf-8',errors='replace')
boot=(r/'src/runtime/bootstrap_thread.cpp').read_text(encoding='utf-8',errors='replace')
# DLL-adjacent surge.cfg only.
need('GetModuleFileNameW' in win and 'snapshot_state()' in win and 'state.module_base' in win and 'load_runtime_configuration_pair_wide(path,override_path)' in win and 'constexpr uint16_t config_name[]' in win and 'constexpr uint16_t override_name[]' in win and 'join_module_relative(' in win,'Unicode DLL module config + override pair policy')
need('load_module_relative_configuration()' in win and 'load_runtime_configuration_file(nullptr)' not in win,'no process-CWD surge.cfg load')
need('directory containing this DLL' in hdr,'header config path contract')
# Auth8 with presence-only ClientId request policy; ClientId value remains outside Auth8 semantics.
need('begin_session_auth_code_lease(&lease)' in routes and 'session_auth_code_lease_value(&lease)' in routes and 'finish_session_auth_code_lease(&lease,status==HandlerStatus::ok)' in routes,'transactional session Auth8 route policy')
need('find_attribute(r.command_open_tag,"ClientId",&client_id)' in routes,'ClientId attribute presence gate missing')
need('client_id_status!=preloader_l::xml::AttributeStatus::found' in routes,'ClientId missing-field gate missing')
need('(void)client_id;' in routes,'ClientId value must remain semantically unused')
need('find_direct_child_text(r.command_content,"ClientId"' not in routes,'ClientId child fallback must stay absent')
need('cfg_value_or_empty("AuthCode")' not in routes,'no Config2 AuthCode source in GetAuthCode')
need('AuthCodeValueView session_auth_code() noexcept;' not in auth_h and 'AuthCodeValueView session_auth_code() noexcept' not in auth,'compatibility unsafe Auth8 global-view API must stay retired')
need('begin_session_auth_code_lease' in auth_h and 'finish_session_auth_code_lease' in auth_h,'transactional Auth8 lease API')
need('if (commit && lease->commit_pending)' in auth and 'g_session_auth_ready = true;' in auth,'Auth8 commit-on-success boundary')
need('client_id_hash' not in auth and 'generate_session_auth_code(const char*' not in auth and 'session_auth_code(const char*' not in auth,'ClientId entropy must be absent')
need(all(x in auth for x in ('GetSystemTimeAsFileTime','QueryPerformanceCounter','GetCurrentProcessId','GetCurrentThreadId')),'Win32 Auth8 entropy source set drift')
need('reinterpret_cast<uintptr_t>(&g_session_auth_code[0])' in auth,'Auth8 address-mix entropy source drift')
need('g_session_auth_code[9]' in auth and 'g_session_auth_ready' in auth,'session Auth8 storage')
need('clear_auth_code_service()' in auth and 'reset_session_auth_code();' in auth,'Auth8 lifecycle reset')
need('"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"' in auth,'Auth8 alphanumeric alphabet')

# Active GetAuthCode policy metadata must match the shipping runtime.
release=json.loads((r/'RELEASE_STATE.json').read_text())
ga=release.get('get_auth_code_policy',{})
need(ga.get('status')=='PROJECT_ADOPTED_LOCAL_SESSION_AUTH8','GetAuthCode active policy status drift')
need(ga.get('producer')=='LOCAL_RUNTIME_SESSION_AUTH8_SERVICE','GetAuthCode producer policy drift')
need(ga.get('online_behavior')=='SESSION_STABLE_8_CHARACTER_ALPHANUMERIC_AUTHCODE','GetAuthCode Auth8 shape/lifetime metadata drift')
need(ga.get('commit_boundary')=='SUCCESSFUL_SERIALIZATION_ONLY','GetAuthCode commit-boundary metadata drift')
need(ga.get('config2_authcode_policy') is False,'Config2 AuthCode policy metadata drift')
need(ga.get('client_id_required') is True and ga.get('client_id_semantic_policy')=='PRESENCE_ONLY_EMPTY_ACCEPTED' and ga.get('client_id_entropy_policy') is False,'ClientId policy metadata drift')
need(ga.get('behavioral_active_equivalence')=='NOT_CLAIMED' and ga.get('backend_credential_validity')=='NOT_CLAIMED','GetAuthCode provenance claim drift')

# 3.2.0 bounded offline-route closure. These semantics already exist in runtime source;
# this gate prevents stale offline special cases from silently returning.
grant=(r/'src/handlers/achievement_grant_route.cpp').read_text()
query_ach=(r/'src/handlers/achievement_query_route.cpp').read_text()
offers=(r/'src/handlers/store_routes.cpp').read_text()
need('if(offline()) return error_response(r,"","-1576468080"' not in grant,'GrantAchievement offline special case must stay removed')
need('!eq(r.command,"GrantAchievement")||offline()' not in grant,'GrantAchievement event preview offline suppression must stay removed')
need('if(offline()) return error_response(r,"","-1576468080"' not in query_ach,'QueryAchievements offline special case must stay removed')
need('if(offline()) return error_response(r,"Unknown Error","-1"' not in offers,'QueryOffers offline special case must stay removed')

# Avatar fallback.
need('GetModuleFileNameW' in avatar and 'WideCharToMultiByte' in avatar and 'snapshot_state()' in avatar,'Unicode DLL-adjacent avatar policy')
need("kAsset[]={'s','u','r','g','e','.','p','n','g',0u}" in avatar and 'LOCALAPPDATA' not in avatar,'surge.png DLL sibling fallback')
# Explicit exclusions: preserve clean connectivity/bootstrap semantics while allowing
# the bounded F1420-001 lifecycle rollback correction.
need('probe_process_startup_online_configuration' not in boot and 'apply_startup_online_environment' not in boot,'Correction-2 preflight must be absent')
need('const uint32_t existing = backend->length(names[i], backend->context);' in env,'clean host-preserve environment policy retained')
need('EnvironmentStatus restore_environment(' in env,'F1420 environment rollback helper missing')
need('if ((written_mask & bit) == 0u) continue;' in env,'F1420 rollback is not written-mask bounded')
need('backend->set(names[i], nullptr, backend->context)' in env,'F1420 rollback does not delete DLL-created values')
if e:
    print('V312_MINIMAL_RUNTIME_FIXSET: FAIL')
    for x in e: print(' -',x)
    sys.exit(1)
print('V312_MINIMAL_RUNTIME_FIXSET: PASS (runtime + active GetAuthCode policy metadata aligned)')
