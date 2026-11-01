#!/usr/bin/env python3
"""Run exact source-linked Windows Asio IOCP tests under MSVC /MT and /MTd.

Test-only, never a shipping DLL publisher or a release/game acceptance oracle.
Execute in x64 Visual Studio Developer Command Prompt with Python 3 installed.
"""
from pathlib import Path
import os
import subprocess
import sys
import shutil
r=Path(__file__).resolve().parents[1]
mode=sys.argv[1] if len(sys.argv)>1 else 'both'
if mode not in ('ingress','continuation','transactions','owner','fifo','store','dispatch','both','all'):
    print('usage: python scripts/test_asio_iocp_native.py [ingress|continuation|transactions|owner|fifo|store|dispatch|both|all]');sys.exit(2)
if os.name!='nt' or not shutil.which('cl.exe'):
    print('ERROR: x64 Visual Studio Developer Command Prompt with cl.exe required');sys.exit(2)
# A plain VS Developer Command Prompt can select x86, even if Asio IOCP tests pass.
# The shipping HF12 target is x64 only; reject explicitly selected x86.
target_arch=os.environ.get('VSCMD_ARG_TGT_ARCH','').strip().lower()
if target_arch and target_arch not in ('x64','amd64'):
    print('ERROR: HF12 requires x64 MSVC; active target is',repr(target_arch),
          '- use x64 Native Tools Command Prompt or VsDevCmd.bat -arch=amd64');sys.exit(2)
base=['src/protocol/socket_exchange_wire.cpp','src/codec/message_codec.cpp','src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp','src/xml/lsx_framing.cpp']
continuation=['src/protocol/socket_exchange_frame_step.cpp','src/protocol/challenge_handshake.cpp',
'src/codec/session_crypto.cpp','src/xml/request_parser.cpp','src/xml/request_children.cpp',
'src/xml/response_builder.cpp','src/xml/value_codec.cpp','src/xml/value_codec_utf8.cpp',
'src/xml/value_codec_normalize.cpp','src/xml/value_codec_semantic.cpp',
'src/xml/attribute_parser.cpp','third_party/tinyxml2/tinyxml2.cpp',
# SemanticValue's true shipping transient allocation owner and its PTD dependency.
# Do not substitute malloc/free or no-op native acceptance stubs here.
'src/runtime/memory_allocator.cpp','src/runtime/ptd_runtime.cpp']
include=[r/'third_party/asio/upstream/include',r/'third_party/tinyxml2',r/'third_party/pegtl/upstream/include',r/'include',r/'src/protocol']
for path in [*base,*continuation,'tests/native/test_asio_ingress.cpp','tests/native/test_asio_iocp_handshake.cpp',
             'tests/native/test_asio_iocp_delivery_completion.cpp',
             'tests/native/test_asio_iocp_transaction_driver.cpp',
             'tests/native/test_asio_iocp_real_fifo.cpp',
             'tests/native/test_asio_iocp_grant_store.cpp',
             'tests/native/test_asio_iocp_full_grant_dispatch.cpp',
             'src/protocol/socket_exchange_async_transaction.cpp',
             'src/protocol/socket_exchange_transaction_delivery.cpp',
             'src/protocol/socket_exchange_write_completion.cpp']:
    if not (r/path).is_file():
        print('ERROR: missing source',path);sys.exit(2)
out=r/'dist/hf12_iocp_native'
out.mkdir(parents=True,exist_ok=True)
for scenario in (('ingress','continuation','transactions','owner','fifo','store','dispatch') if mode=='all' else
                 ('ingress','continuation') if mode=='both' else (mode,)):
    if scenario=='ingress':
        sources=['tests/native/test_asio_ingress.cpp']+base
    elif scenario=='continuation':
        sources=['tests/native/test_asio_iocp_handshake.cpp']+base+continuation
    elif scenario=='transactions':
        sources=['tests/native/test_asio_iocp_delivery_completion.cpp',
                 'src/protocol/socket_exchange_write_completion.cpp']
    elif scenario=='store':
        sources=['tests/native/test_asio_iocp_grant_store.cpp',
             'tests/native/test_asio_iocp_full_grant_dispatch.cpp',
                 'src/protocol/socket_exchange_async_transaction.cpp',
                 'src/protocol/socket_exchange_transaction_delivery.cpp',
                 'src/protocol/socket_exchange_write_completion.cpp',
                 'src/runtime/event_queue.cpp',
                 'src/runtime/service_state.cpp',
                 'src/runtime/service_state_storage.cpp',
                 'src/runtime/service_state_parser.cpp',
                 'src/runtime/service_state_grant.cpp',
                 'src/config/config2_pegtl.cpp',
                 'src/runtime/heap_allocator.cpp',
                 'src/runtime/memory_allocator.cpp','src/runtime/ptd_runtime.cpp']
    elif scenario=='dispatch':
        # Real XML request dispatcher -> GrantAchievement handler + persistent
        # service state -> AES/hex response and Event -> Asio IOCP callback.
        # Source list is inherited from the Wave7 production coherence graph;
        # ONLY image/platform unrelated functions have isolated native stubs.
        sources=['tests/native/test_asio_iocp_full_grant_dispatch.cpp']
        sources += [
          'src/protocol/socket_exchange_transaction.cpp',
          'src/protocol/socket_exchange_transaction_delivery.cpp',
          'src/protocol/socket_exchange_async_transaction.cpp',
          'src/protocol/socket_exchange_write_completion.cpp',
          'src/protocol/socket_exchange.cpp','src/protocol/socket_exchange_wire.cpp',
          'src/protocol/socket_exchange_frame_step.cpp','src/protocol/challenge_handshake.cpp',
          'src/net/socket_transport.cpp','src/codec/session_crypto.cpp',
          'src/codec/message_codec.cpp','src/codec/aes128_cipher.cpp','src/codec/hex_codec.cpp',
          'src/router/route_lookup.cpp','src/router/route_inventory.cpp',
          'src/core/request_dispatch.cpp','src/hash/fnv_hash.cpp','src/runtime/event_queue.cpp',
          'src/xml/request_parser.cpp','third_party/tinyxml2/tinyxml2.cpp',
          'src/xml/request_children.cpp','src/xml/lsx_framing.cpp',
          'src/xml/response_builder.cpp','src/xml/value_codec.cpp',
          'src/xml/value_codec_utf8.cpp','src/xml/value_codec_normalize.cpp',
          'src/xml/value_codec_semantic.cpp','src/xml/attribute_parser.cpp',
          'src/config/config2_pegtl.cpp','src/config/settings_values.cpp',
          'src/config/settings_provider.cpp','src/config/runtime_provider.cpp',
          'src/config/runtime_configuration.cpp','src/config/runtime_configuration_parser.cpp',
          'src/config/runtime_configuration_storage.cpp',
          'src/runtime/service_state.cpp','src/runtime/service_state_storage.cpp',
          'src/runtime/service_state_parser.cpp','src/runtime/service_state_grant.cpp',
          'src/runtime/memory_allocator.cpp','src/runtime/ptd_runtime.cpp',
          'src/runtime/utc_time.cpp','src/runtime/auth_code_service.cpp',
          'src/runtime/achievement_watcher.cpp','src/runtime/local_social_state.cpp',
          'src/runtime/local_social_host.cpp',
          'src/handlers/progressive_installation_route.cpp','src/handlers/permission_route.cpp',
          'src/handlers/friends_route.cpp','src/handlers/integration_routes.cpp',
          'src/handlers/group_info_route.cpp','src/handlers/utc_time_route.cpp','src/handlers/query_presence_route.cpp',
          'src/handlers/social_mutation_routes.cpp','src/handlers/social_mutation_socket.cpp',
          'src/handlers/local_social_list_route.cpp','src/handlers/config_route.cpp',
          'src/handlers/setting_value_route.cpp','src/handlers/settings_snapshot_route.cpp',
          'src/handlers/handler_registry.cpp','src/handlers/connection_state_route.cpp',
          'src/handlers/connection_state_binding.cpp','src/handlers/runtime_settings_routes.cpp',
          'src/handlers/selector_pipeline.cpp','src/handlers/game_info_routes.cpp',
          'src/handlers/profile_routes.cpp','src/handlers/auth_routes.cpp',
          'src/handlers/store_routes.cpp','src/handlers/achievement_routes.cpp',
          'src/handlers/achievement_grant_route.cpp','src/handlers/achievement_query_route.cpp',
          'src/handlers/image_route.cpp']
    elif scenario=='fifo':
        sources=['tests/native/test_asio_iocp_real_fifo.cpp',
                 'src/protocol/socket_exchange_async_transaction.cpp',
                 'src/protocol/socket_exchange_transaction_delivery.cpp',
                 'src/protocol/socket_exchange_write_completion.cpp',
                 'src/runtime/event_queue.cpp',
                 'src/runtime/memory_allocator.cpp','src/runtime/ptd_runtime.cpp']
    else:
        sources=['tests/native/test_asio_iocp_transaction_driver.cpp',
                 'src/protocol/socket_exchange_async_transaction.cpp',
                 'src/protocol/socket_exchange_transaction_delivery.cpp',
                 'src/protocol/socket_exchange_write_completion.cpp',
                 'src/runtime/memory_allocator.cpp','src/runtime/ptd_runtime.cpp']
    for profile,crt in (('mt','/MT'),('mtd','/MTd')):
        target=f'{scenario}_{profile}'
        cmd=['cl.exe','/nologo','/std:c++20','/EHsc','/GR-','/W4',crt,
             '/DASIO_STANDALONE','/DASIO_NO_TYPEID','/D_WIN32_WINNT=0x0601','/Gy','/Gw',
             *[f'/I{p}' for p in include],
             *[str(r/x) for x in sources],f'/Fe:{out / (target+".exe")}',
             '/link','/OPT:REF','Ws2_32.lib','Mswsock.lib']
        print('BUILD:',target,flush=True)
        with (out/(target+'_compile.log')).open('w',encoding='utf-8',errors='replace') as log:
            cp=subprocess.run(cmd,cwd=out,stdout=log,stderr=subprocess.STDOUT)
        if cp.returncode:
            print('FAIL BUILD:',target,'see',out/(target+'_compile.log'));sys.exit(1)
        print('RUN:',target,flush=True)
        with (out/(target+'_run.log')).open('w',encoding='utf-8',errors='replace') as log:
            cp=subprocess.run([str(out/(target+'.exe'))],cwd=out,stdout=log,stderr=subprocess.STDOUT,timeout=90)
        marker=('HF12_STAGE01_REAL_WINDOWS_ASIO_IOCP PASS' if scenario=='ingress' else
                'HF12_STAGE02_REAL_WINDOWS_IOCP_HANDSHAKE PASS' if scenario=='continuation' else
                'HF12_STAGE03_REAL_WINDOWS_IOCP_DELIVERY_COMPLETION PASS' if scenario=='transactions' else
                'HF12_STAGE04B_REAL_WINDOWS_IOCP_ASYNC_OWNER PASS' if scenario=='owner' else
                'HF12_STAGE04C_REAL_FIFO_WINDOWS_IOCP PASS' if scenario=='fifo' else
                'HF12_STAGE04D_REAL_STORE_FIFO_WINDOWS_IOCP PASS' if scenario=='store' else
                'HF12_STAGE04E_FULL_GRANT_DISPATCH_WINDOWS_IOCP PASS')
        output=(out/(target+'_run.log')).read_text(encoding='utf-8',errors='replace')
        if cp.returncode or marker not in output:
            print('FAIL RUN:',target,output[-1200:]);sys.exit(1)
        print('PASS:',target,flush=True)
print('PASS: Genuine Windows x64 Asio IOCP acceptance for selected source-linked scenarios in /MT and /MTd.',flush=True)
print('SCOPE: dispatch mode attempts true XML/Grant handler/AES+hex/FIFO on IOCP with test-only socket host; no shipping Asio server, failed-close injection or game acceptance. New dispatch mode Windows result requires independent MSVC x64 execution.',flush=True)
