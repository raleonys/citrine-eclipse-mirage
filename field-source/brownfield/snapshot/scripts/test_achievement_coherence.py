#!/usr/bin/env python3
from pathlib import Path
import json, os, subprocess, sys, tempfile

r=Path(__file__).resolve().parents[1]
errors=[]

def read(rel):
    return (r/rel).read_text(encoding='utf-8',errors='replace')

grant=read('src/handlers/achievement_grant_route.cpp')
query=read('src/handlers/achievement_query_route.cpp')
watcher=read('src/runtime/achievement_watcher.cpp')
watcher_header=read('include/preloader_l/runtime/achievement_watcher.hpp')
header=read('include/preloader_l/runtime/service_state.hpp')
catalog=json.loads(read('metadata/RELEASE_QA_CATALOG.json'))
state=json.loads(read('RELEASE_STATE.json'))
matrix=json.loads(read('metadata/ACCEPTANCE_MATRIX.json'))

# Production contract: reuse the existing grant-delivery gate only.
measure=grant.find('if(!out&&cap==0u){')
measure_end=grant.find('\n    }',measure)
measure_body=grant[measure:measure_end] if measure>=0 and measure_end>measure else ''
checks={
    'Grant measure takes delivery gate': 'GrantAchievementDeliveryReservationScope delivery_reservation;' in measure_body,
    'Grant gate precedes behavioral state': measure_body.find('GrantAchievementDeliveryReservationScope delivery_reservation;') < measure_body.find('BehavioralServiceStateScope store_scope;'),
    'Query delivery scope exists': 'struct QueryAchievementsDeliveryScope' in query,
    'Query acquires existing delivery gate': 'preloader_l::runtime::acquire_grant_achievement_delivery();' in query,
    'Query releases existing delivery gate': 'preloader_l::runtime::release_grant_achievement_delivery();' in query,
    'Query handler takes scope': 'QueryAchievementsDeliveryScope delivery_scope;' in query,
    'Header documents coherence boundary': 'no achievement measure/render pair can straddle a post-send' in header,
    'No new handler mutex primitive': all(x not in (grant+'\n'+query) for x in ['std::mutex','SRWLOCK','CRITICAL_SECTION']),
    'Permanent query regression exists': (r/'tests/unit/test_achievement_query_coherence.cpp').is_file(),
    'Permanent grant regression exists': (r/'tests/unit/test_achievement_grant_coherence.cpp').is_file(),
    'Historical coherence gate owned by release QA': catalog.get('schema')=='universal-release-qa-catalog.v1' and any(step.get('script')=='test_achievement_coherence.py' and step.get('migration')=='GENERAL_CHECK' and step.get('retain_in_authoritative_universe') is True for step in catalog.get('steps',[])),
    'Current official release preserves coherence contract': state.get('release')=='3.2.5 SQUALERIS' and state.get('release_type')=='HOTFIX_RELEASE',
    'Historical acceptance gate retained': any(g.get('id')=='wave7_achievement_coherence' for g in matrix.get('gates',[])),
    'Audit3 S0320 callback state exported': 'achievement_watcher_callback_active()' in watcher and 'achievement_watcher_callback_active() noexcept;' in watcher_header,
    'Audit3 S0320 callback outside behavioral lock': grant.find('store_scope.release();') < grant.find('publish_achievement_watcher_snapshot(') and grant.find('store_scope.release();') >= 0,
    'Audit3 S0320 recursive grant fail-fast': 'if(preloader_l::runtime::achievement_watcher_callback_active())' in grant,
    'Audit3 S0320 query borrows outer delivery reservation': 'held(!preloader_l::runtime::achievement_watcher_callback_active())' in query,
    'Audit3 S0320 executable reentry regression retained': 'g_watcher_reenter=true' in read('tests/unit/test_grant_achievement_failure_atomicity.cpp'),
}
for name,ok in checks.items():
    if not ok: errors.append(name)
if errors:
    print('PREAUDIT_WAVE7_ACHIEVEMENT_COHERENCE: FAIL')
    for x in errors: print(' -',x)
    sys.exit(1)

cxx=os.environ.get('CXX') or 'c++'
common=[cxx,'-std=c++20','-O0','-Wall','-Wextra','-Werror','-pthread',
        '-I',str(r/'include'),'-I',str(r/'tests/support'),'-I',str(r/'src/debug'),'-I',str(r/'src/protocol'),'-I',str(r/'third_party/pegtl/upstream/include')]
request_sources=[r/'src/xml/request_parser.cpp',r/'third_party/tinyxml2/tinyxml2.cpp',r/'src/xml/request_children.cpp']
value_codec_sources=[r/'src/xml/value_codec.cpp',r/'src/xml/value_codec_utf8.cpp',r/'src/xml/value_codec_normalize.cpp',r/'src/xml/value_codec_semantic.cpp']
runtime_config_sources=[r/'src/config/runtime_configuration.cpp',r/'src/config/runtime_configuration_parser.cpp',r/'src/config/runtime_configuration_storage.cpp']
behavioral_state_sources=[r/'src/runtime/service_state.cpp',r/'src/runtime/service_state_storage.cpp',r/'src/runtime/service_state_parser.cpp',r/'src/runtime/service_state_grant.cpp']
achievement_routes=[r/'src/handlers/achievement_routes.cpp',r/'src/handlers/achievement_grant_route.cpp',r/'src/handlers/achievement_query_route.cpp']

with tempfile.TemporaryDirectory(prefix='melotrix_w7_') as td_s:
    td=Path(td_s)
    transaction_obj=td/'socket_exchange_transaction.o'
    q=subprocess.run(common+['-D_WIN32','-D__declspec(x)=','-D__stdcall=',
        '-c',str(r/'src/protocol/socket_exchange_transaction.cpp'),'-o',str(transaction_obj)],cwd=r)
    if q.returncode: raise SystemExit(q.returncode)
    event_obj=td/'event_queue.o'
    q=subprocess.run(common+['-Wno-misleading-indentation','-D_WIN32','-D__declspec(x)=','-D__stdcall=',
        '-c',str(r/'src/runtime/event_queue.cpp'),'-o',str(event_obj)],cwd=r)
    if q.returncode: raise SystemExit(q.returncode)

    shared=[
        r/'src/config/config2_pegtl.cpp',r/'src/protocol/socket_exchange_transaction_delivery.cpp',r/'src/protocol/socket_exchange_write_completion.cpp',r/'src/protocol/socket_exchange.cpp',r/'src/protocol/socket_exchange_wire.cpp',r/'src/protocol/socket_exchange_frame_step.cpp',r/'src/protocol/challenge_handshake.cpp',
        r/'src/net/socket_transport.cpp',r/'src/codec/session_crypto.cpp',r/'src/codec/message_codec.cpp',r/'src/codec/aes128_cipher.cpp',r/'src/codec/hex_codec.cpp',
        *request_sources,r/'src/xml/lsx_framing.cpp',r/'src/xml/response_builder.cpp',*value_codec_sources,r/'src/xml/attribute_parser.cpp',
        r/'src/router/route_lookup.cpp',r/'src/router/route_inventory.cpp',r/'src/core/request_dispatch.cpp',r/'src/hash/fnv_hash.cpp',
        r/'src/config/settings_values.cpp',r/'src/config/settings_provider.cpp',r/'src/config/runtime_provider.cpp',*runtime_config_sources,
        *behavioral_state_sources,r/'src/runtime/memory_allocator.cpp',r/'src/runtime/ptd_runtime.cpp',r/'src/runtime/utc_time.cpp',r/'src/runtime/auth_code_service.cpp',r/'src/runtime/achievement_watcher.cpp',
        r/'src/handlers/progressive_installation_route.cpp',r/'src/handlers/permission_route.cpp',r/'src/handlers/friends_route.cpp',
        r/'src/handlers/integration_routes.cpp',r/'src/handlers/group_info_route.cpp',r/'src/handlers/utc_time_route.cpp',r/'src/handlers/query_presence_route.cpp',r/'src/handlers/social_mutation_routes.cpp',r/'src/handlers/social_mutation_socket.cpp',r/'src/handlers/local_social_list_route.cpp',r/'src/runtime/local_social_state.cpp',r/'src/runtime/local_social_host.cpp',r/'src/handlers/config_route.cpp',r/'src/handlers/setting_value_route.cpp',
        r/'src/handlers/settings_snapshot_route.cpp',r/'src/handlers/handler_registry.cpp',r/'src/handlers/connection_state_route.cpp',
        r/'src/handlers/connection_state_binding.cpp',r/'src/handlers/runtime_settings_routes.cpp',r/'src/handlers/selector_pipeline.cpp',
        r/'src/handlers/game_info_routes.cpp',r/'src/handlers/profile_routes.cpp',r/'src/handlers/auth_routes.cpp',
        r/'src/handlers/store_routes.cpp',*achievement_routes,r/'src/handlers/image_route.cpp',
    ]
    outputs=[]
    for name,test,marker in [
        ('grant_query',r/'tests/unit/test_achievement_query_coherence.cpp','W7_GRANT_QUERY_COHERENCE=100/100'),
        ('grant_grant',r/'tests/unit/test_achievement_grant_coherence.cpp','W7_GRANT_GRANT_COHERENCE=100/100'),
    ]:
        exe=td/name
        q=subprocess.run(common+['-Wno-unused-function','-Wno-misleading-indentation',str(test)]+[str(x) for x in shared]+[str(transaction_obj),str(event_obj),'-o',str(exe)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        if q.returncode:
            print('PREAUDIT_WAVE7_ACHIEVEMENT_COHERENCE: FAIL - compile '+name)
            print(q.stdout[-3000:]); print(q.stderr[-6000:]); sys.exit(1)
        q=subprocess.run([str(exe)],cwd=r,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        text=q.stdout+q.stderr
        if q.returncode or marker not in text:
            print('PREAUDIT_WAVE7_ACHIEVEMENT_COHERENCE: FAIL - runtime '+name)
            print(text[-6000:]); sys.exit(1)
        outputs.append(marker)

print(f'PREAUDIT_WAVE7_ACHIEVEMENT_COHERENCE: PASS {len(checks)}/{len(checks)}')
for x in outputs: print(' '+x)
